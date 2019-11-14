#include "generator.h"
#include "../../common/error.hpp"
#include "../../extra/json/json.hpp"
#include "../../extra/arguments/arguments.hpp"
#include "../../utility/subprocess.hpp"
#include "../../utility/debug.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

using namespace std;
using json = nlohmann::json;

extern string arrp_raw_io_template;

namespace arrp {
namespace generic_io {

void replace(string & text, const string & pattern, const string & replacement)
{
    auto pos = text.find(pattern);
    if (pos == string::npos)
        throw runtime_error("Could not find pattern to replace.");

    text.replace(pos, pattern.size(), replacement);
}

string channel_names(const json& channels)
{
    string text;
    for (auto & elem : channels)
    {
        string name = elem["name"];

        if (!text.empty())
            text += ", ";
        text += '"';
        text += name;
        text += '"';
    }
    return "{ " + text + " }";
}

string io_function(const json & channel, int index, bool is_input, bool raw)
{
    string name = channel["name"];
    int size = channel["size"];

    ostringstream function;

    if (raw)
    {
        function << "template <typename T> void " << name;
        if (size > 1)
            function << "(T * value)";
        else
            function << "(T & value)";
        function << "{" << endl;

        function << "  " << (is_input ? "raw_input" : "raw_output") << "(";

        function << (size > 1 ? "value" : "&value");

        function << ", " << size << ", ";

        function << (is_input ? "*inputs" : "*outputs");

        function << "[" << index << "]);" << endl;

        function << "}" << endl;
    }
    else
    {
        if (size > 1)
        {
            function << "template <typename T> void " << name << "(T * value) {" << endl;
            if (is_input)
                function << "  text_input(value, " << size << ", *inputs[" << index << "]);" << endl;
            else
                function << "  text_output(value, " << size << ", *outputs[" << index << "]);" << endl;
            function << "}" << endl;
        }
        else
        {
            function << "template <typename T> void " << name << "(T & value) {" << endl;
            if (is_input)
                function << "  text_input(value, *inputs[" << index << "]);" << endl;
            else
                function << "  text_output(value, *outputs[" << index << "]);" << endl;
            function << "}" << endl;
        }
    }

    return function.str();
}

string cpp_type_for_arrp_type(const string & type)
{
    if (type == "bool")
        return "bool";
    if (type == "int")
        return "int";
    if (type == "real32")
        return "float";
    if (type == "real64")
        return "double";
    if (type == "complex32")
        return "complex<float>";
    if (type == "complex64")
        return "complex<double>";
    return string();
}

void write_channel_func(ostream & text, const nlohmann::json & channel, bool is_input)
{
    string name = channel["name"];

    string func_name;
    if (is_input)
        func_name = "input_" + name;
    else
        func_name = "output_" + name;

    int size = channel["size"];
    string type = cpp_type_for_arrp_type(channel["type"]);

    text << "shared_ptr<AbstractChannel<" << type << ">> sp_" << name << ";" << endl;

    if (size > 1)
    {
        text << "template <typename A>" << endl;
        text << "void " << func_name << "("
                << "A &data"
                << ") {" << endl;
        text << "  sp_" << name << "->transfer(reinterpret_cast<" << type << "*>(data), " << size << ");" << endl;
        text << "}" << endl;
    }
    else
    {
        {
            text << "void " << func_name << "("
                    << type << "& data"
                    << ") {" << endl;
            text << "  sp_" << name << "->transfer(&data, 1);" << endl;
            text << "}" << endl;
        }
    }
}

void write_channel_manager(ostream & text, const nlohmann::json & channel, bool is_input)
{
    string name = channel["name"];
    string type = cpp_type_for_arrp_type(channel["type"]);
    bool is_stream = channel["is_stream"];
    int size = channel["size"];
    string manager_type = "ChannelManager<" + type + ">";
    text << "  { "
            << "\"" << name << "\", "
            << " std::make_shared<" << manager_type << ">"
            << "(sp_" << name << ", "
            << manager_type << "::Properties { " << is_input << ", " << is_stream << ", " << size << " }) "
            << "},"
            << endl;
}

void generate
(const generic_io::options & options, const nlohmann::json & report,
 filesystem::temporary_dir & temp_dir)
{
    string kernel_file_name = report["cpp"]["tmp-filename"];
    string kernel_namespace = report["cpp"]["namespace"];

    bool has_period = false;

    for (auto & out : report["outputs"])
    {
        bool is_stream = out["is_stream"];
        if (is_stream)
        {
            has_period = true;
            break;
        }
    }

    ostringstream io_text;
    io_text << "namespace arrp { namespace generic_io {" << endl;
    io_text << "struct Generated_IO {" << endl;

    io_text << "static const bool has_period = " << (has_period ? "true" : "false") << ";" << endl;

    // IO functions

    for (auto & channel : report["inputs"])
    {
        write_channel_func(io_text, channel, true);
    }

    for (auto & channel : report["outputs"])
    {
        write_channel_func(io_text, channel, false);
    }

    // Channel managers

    io_text << "ChannelManagerMap input_managers = {" << endl;
    for (auto & channel : report["inputs"])
    {
        write_channel_manager(io_text, channel, true);
    }
    io_text << "};" << endl;

    io_text << "ChannelManagerMap output_managers = {" << endl;
    for (auto & channel : report["outputs"])
    {
        write_channel_manager(io_text, channel, false);
    }
    io_text << "};" << endl;

    // End class
    io_text << "};" << endl;

    // End namespace
    io_text << "}}" << endl;

    string main_cpp_file_name = temp_dir.name() + "/main.cpp";
    string io_cpp_file_name = temp_dir.name() + "/generated_interface.h";

    {
        ofstream file(io_cpp_file_name);
        file << io_text.str() << endl;
    }
    {
        ofstream file(main_cpp_file_name);
        file << "#include <arrp/generic_io/interface.h>" << endl;
        file << "#include \"generated_interface.h\"" << endl;
        file << "#include \"" << kernel_file_name << "\"" << endl;
        file << "using Generated_Kernel = " << kernel_namespace
             << "::program<arrp::generic_io::Generated_IO>;" << endl;
        file << "#include <arrp/generic_io/main.cpp>" << endl;
    }

    // Compile C++

    string cpp_compiler;

    {
        auto * CXX = getenv("CXX");
        if (CXX)
            cpp_compiler = CXX;
        if (cpp_compiler.empty())
        {
            if (system("c++ --version 2>&1 > /dev/null") == 0)
                cpp_compiler = "c++";
        }
        if (cpp_compiler.empty())
        {
            if (system("g++ --version 2>&1 > /dev/null") == 0)
                cpp_compiler = "g++";
        }
        if (cpp_compiler.empty())
        {
            throw stream::error("Failed to find C++ compiler.");
        }
    }

    if (stream::verbose<log>::enabled())
        cerr << "Using C++ compiler: " << cpp_compiler << endl;

    string include_dirs;

    {
        auto * ARRP_HOME = getenv("ARRP_HOME");
        if (ARRP_HOME)
            include_dirs += " -I" + string(ARRP_HOME) + "/include";
    }

    {
        string cmd = cpp_compiler
                + " -std=c++17"
                + " -O1 "
                + include_dirs
                + " " + main_cpp_file_name
                + " -o " + options.output_file;

        if (stream::verbose<log>::enabled())
            cerr << "Executing: " << cmd << endl;

        subprocess::run(cmd);
    }
}

}
}
