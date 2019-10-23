#include "generator.h"
#include "../../common/error.hpp"
#include "../../extra/json/json.hpp"
#include "../../extra/arguments/arguments.hpp"
#include "../../utility/subprocess.hpp"
#include "../../utility/debug.hpp"

#include <iostream>
#include <fstream>
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

void generate(const generic_io::options & options, const nlohmann::json & report)
{
    bool raw_mode = false;

    if (options.mode == "text")
        raw_mode = false;
    else if (options.mode == "raw")
        raw_mode = true;
    else
        throw stream::error("Invalid IO mode: " + options.mode);

    string kernel_file_name = report["cpp"]["filename"];
    string kernel_namespace = report["cpp"]["namespace"];

    // Generate C++ code

    string io_text = arrp_raw_io_template;

    replace(io_text, "INPUT_COUNT", to_string(report["inputs"].size()));
    replace(io_text, "OUTPUT_COUNT", to_string(report["outputs"].size()));

    string input_names = channel_names(report["inputs"]);
    string output_names = channel_names(report["outputs"]);

    replace(io_text, "INPUT_NAMES", input_names);
    replace(io_text, "OUTPUT_NAMES", output_names);

    string io_functions;

    {
        int input_index = 0;
        for (auto & channel : report["inputs"])
        {
            io_functions += io_function(channel, input_index, true, raw_mode);
            ++input_index;
        }
    }

    {
        int output_index = 0;
        for (auto & channel : report["outputs"])
        {
            io_functions += io_function(channel, output_index, false, raw_mode);
            ++output_index;
        }
    }

    replace(io_text, "IO_FUNCTIONS", io_functions);

    string kernel_declaration;
    kernel_declaration = "#include \"" + kernel_file_name + "\"";
    replace(io_text, "DECLARE_KERNEL", kernel_declaration);

    string kernel_instance;
    kernel_instance += kernel_namespace;
    kernel_instance += "::program<arrp::Raw_IO> kernel;\n";
    kernel_instance += "kernel.io = &io;\n";
    replace(io_text, "INSTANTIATE_KERNEL", kernel_instance);

    string tmp_dir = "/tmp/arrp/generic-io";
    subprocess::run("mkdir -p " + tmp_dir);

    string main_cpp_file_name = tmp_dir + "/program.cpp";

    {
        ofstream file(main_cpp_file_name);
        file << io_text << endl;
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
        include_dirs += " -I.";

        auto * ARRP_HOME = getenv("ARRP_HOME");
        if (ARRP_HOME)
            include_dirs += " -I" + string(ARRP_HOME) + "/include";
    }

    {
        string cmd = cpp_compiler
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
