#include "generator.h"
#include "../../common/error.hpp"
#include "../../extra/json/json.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;
using json = nlohmann::json;

namespace arrp {
namespace jack_io {

void generate_io_function(const json & channel, int index, bool is_input, ostream & out)
{
    string name = channel["name"];
    string direction = is_input ? "input" : "output";
    string type = string(channel["type"]) == "real64" ? "double" : "float";

    out << "void " << direction << "_" << name;
    out << "(" << type << "& value) { ";
    if (is_input)
        out << "value = input(" << index << ");";
    else
        out << "output(" << index << ", value);";
    out << " }" << endl;
}

void generate(const options & opt, const nlohmann::json & report)
{
    // TODO:
    // - Pass sample rate to kernel
    // - Support other finite inputs (parameters)
    // - Support runtime variable parameters?
    // - Support MIDI/OSC streams?

    string kernel_file_name = report["cpp"]["filename"];
    string kernel_namespace = report["cpp"]["namespace"];

    int commonPeriodFrames = -1;

    auto & inputs = report["inputs"];
    auto & outputs = report["outputs"];

    for (auto & channel : inputs)
    {
        if (not channel["is_stream"])
            throw stream::error("Input is not a stream: " + string(channel["name"]));
        if (string(channel["type"]) != "real32" and string(channel["type"]) != "real64")
            throw stream::error("Input does not have type real32 or real64: " + string(channel["name"]));
        if (channel.count("dimensions"))
            throw stream::error("Input is multidimensional: " + string(channel["name"]));

        int periodFrames = channel["period_count"];
        if (commonPeriodFrames >= 0 and periodFrames != commonPeriodFrames)
            throw stream::error("Input/output channels have different rates.");
        commonPeriodFrames = periodFrames;
    }

    for (auto & channel : outputs)
    {
        if (not channel["is_stream"])
            throw stream::error("Output is not a stream: " + string(channel["name"]));
        if (string(channel["type"]) != "real32" and string(channel["type"]) != "real64")
            throw stream::error("Output does not have type real32 or real64: " + string(channel["name"]));
        if (channel.count("dimensions"))
            throw stream::error("Output is multidimensional: " + string(channel["name"]));

        int periodFrames = channel["period_count"];
        if (commonPeriodFrames >= 0 and periodFrames != commonPeriodFrames)
            throw stream::error("Input/output channels have different rates.");
        commonPeriodFrames = periodFrames;
    }

    ostringstream io_text;
    io_text << "#include <arrp/jack_io/jack_client.cpp>" << endl;
    io_text << "#include \"" << kernel_file_name << "\"" << endl;
    io_text << "#include <string>" << endl;
    io_text << "#include <memory>" << endl;
    // FIXME: Use standard C++ to sleep forever instead of POSIX sleep from unistd.h
    io_text << "#include <unistd.h>" << endl;

    io_text << "namespace arrp { namespace jack_io {" << endl;

    io_text << "class Program : public Jack_Client {" << endl;

    io_text << "using Kernel = " << kernel_namespace << "::program<Program>;" << endl;

    io_text << "public:" << endl;

    io_text << "std::unique_ptr<Kernel> d_kernel;" << endl;

    io_text << "Program(): "
            << "Jack_Client(\"" << opt.client_name << "\", "
            << inputs.size() << ", " << outputs.size() << ")" << endl
            << "{" << endl
            << " d_kernel = std::make_unique<Kernel>();" << endl
            << " d_kernel->io = this;" << endl
            << "}" << endl;

    for (int i = 0; i < inputs.size(); ++i)
    {
        generate_io_function(inputs[i], i, true, io_text);
    }

    for (int i = 0; i < outputs.size(); ++i)
    {
        generate_io_function(outputs[i], i, false, io_text);
    }

    io_text << "void process() override {" << endl
            << "d_kernel->prelude();" << endl
            << "for(;;) { d_kernel->period(); }" << endl
            << "}" << endl;

    io_text << "};" << endl; // class


    io_text << "}}" << endl; // namespace

    io_text << "int main () {" << endl
            << "arrp::jack_io::Program program;" << endl
            << "sleep(-1);" << endl
            << "}" << endl;

    {
        string filename = opt.base_file_name + "-jack-client.cpp";
        cerr << "Writing to " << filename << endl;
        ofstream io_file(filename);
        io_file << io_text.str();
    }
}

}
}
