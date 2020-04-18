#include "generate.h"
#include "../../common/error.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;
using nlohmann::json;

namespace arrp {
namespace puredata_io {

void generate_io_function(const json & channel, int index, bool is_input, ostream & out)
{
    string name = channel["name"];
    string direction = is_input ? "input" : "output";

    out << "void " << direction << "_" << name;
    out << "(float & value) { ";
    out << direction << "(" << index << ", value);";
    out << " }" << endl;
}

void generate(const options & opt, const nlohmann::json & report)
{
    string kernel_file_name = report["cpp"]["filename"];
    string kernel_namespace = report["cpp"]["namespace"];

    int commonPeriodFrames = -1;

    auto & inputs = report["inputs"];
    auto & outputs = report["outputs"];

    for (auto & channel : inputs)
    {
        if (not channel["is_stream"])
            throw stream::error("Input is not a stream: " + string(channel["name"]));
        if (string(channel["type"]) != "real32")
            throw stream::error("Input does not have type real32: " + string(channel["name"]));
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
        if (string(channel["type"]) != "real32")
            throw stream::error("Output does not have type real32: " + string(channel["name"]));
        if (channel.count("dimensions"))
            throw stream::error("Output is multidimensional: " + string(channel["name"]));

        int periodFrames = channel["period_count"];
        if (commonPeriodFrames >= 0 and periodFrames != commonPeriodFrames)
            throw stream::error("Input/output channels have different rates.");
        commonPeriodFrames = periodFrames;
    }

    ostringstream io_text;

    io_text << "#include \"" << kernel_file_name << "\"" << endl;
    io_text << "#include <arrp/puredata_io/interface.h>" << endl;
    io_text << "#include <memory>" << endl;

    io_text << "namespace arrp { namespace puredata_io {" << endl;

    io_text << "class IO : public Abstract_IO {" << endl;

    io_text << "using Kernel = " << kernel_namespace << "::program<IO>;" << endl;
    io_text << "std::unique_ptr<Kernel> kernel;" << endl;

    io_text << "public:" << endl;
    io_text << "IO(): "
            << "Abstract_IO(" << inputs.size() << ", " << outputs.size() << ")"
            << "{}" << endl;

    io_text << "void prologue() override {" << endl;
    io_text << "kernel = std::make_unique<Kernel>();" << endl;
    io_text << "kernel->io = this;" << endl;
    io_text << "kernel->prelude();" << endl;
    io_text << "}" << endl;

    io_text << "void period() override {" << endl;
    io_text << "kernel->period();" << endl;
    io_text << "}" << endl;

    for (int i = 0; i < inputs.size(); ++i)
    {
        generate_io_function(inputs[i], i, true, io_text);
    }

    for (int i = 0; i < outputs.size(); ++i)
    {
        generate_io_function(outputs[i], i, false, io_text);
    }

    io_text << "};" << endl; // class

    io_text << "Abstract_IO * create_kernel() { return new IO; }" << endl;

    io_text << "void library_setup(const char * name, bool has_signal_inputs);" << endl;

    io_text << "}}" << endl; // namespace

    io_text << "extern \"C\" {" << endl;

    io_text << "void " << opt.pd_object_name << "_tilde_setup()"
            << " { "
            << "arrp::puredata_io::library_setup("
            << "\"" << opt.pd_object_name << "~\""
            << ", " << (inputs.size() ? "true" : "false")
            << ");"
            << " }"
            << endl;

    io_text << "}" << endl; // extern C

    {
        string filename = opt.base_file_name + "-pd-interface.cpp";
        cerr << "Writing to " << filename << endl;
        ofstream io_file(filename);
        io_file << io_text.str();
    }
}

}
}

