#pragma once

#include <arrp/arguments/arguments.hpp>

using namespace std;
using namespace arrp::generic_io;

struct Options
{
    string default_channel_format = "raw";
    unordered_map<string, string> channel_options;
};

struct Config
{
    Config(const Options & options, Generated_IO & io)
    {
        single_input_stream = find_singular_stream(io.input_managers);
        single_output_stream = find_singular_stream(io.output_managers);

        for(auto & entry : io.input_managers)
        {
            auto & name = entry.first;
            auto manager = entry.second;
            setup_manager(name, manager, options);
        }

        for(auto & entry : io.output_managers)
        {
            auto & name = entry.first;
            auto manager = entry.second;
            setup_manager(name, manager, options);
        }

        if (unconfigured_channels.size())
        {
            for (auto & name : unconfigured_channels)
            {
                cerr << "Unconfigured channel: " << name << endl;
            }
            throw std::runtime_error("Some channels are not configured.");
        }
    }

private:
    string find_singular_stream(const ChannelManagerMap & managers)
    {
        // If there's only one channel, use it even if it is not a stream:
        if (managers.size() == 1)
            return managers.begin()->first;

        string name;
        for(const auto & entry : managers)
        {
            const auto & manager = entry.second;
            if (manager->is_stream())
            {
                if (name.empty())
                {
                    name = entry.first;
                }
                else
                {
                    return string();
                }
            }
        }
        return name;
    }

    bool is_singular_stream(const string & name)
    {
        return name.size() and (name == single_input_stream or name == single_output_stream);
    }

    void setup_manager(const string & name, shared_ptr<AbstractChannelManager> manager, const Options & options)
    {
        ChannelConfig config;

        string channel_options;
        if (options.channel_options.count(name))
            channel_options = options.channel_options.at(name);

        if (!channel_options.empty())
        {
            try
            {
                config = parse_channel_options(channel_options, options);
            } catch (std::exception & e)
            {
                cerr << "Invalid options for channel '" << name << "': " << e.what() << endl;
                unconfigured_channels.push_back(name);
                return;
            }
        }
        else if (is_singular_stream(name))
        {
            config.type = "pipe";
            config.value = "pipe";
            config.format = options.default_channel_format;
        }
        else
        {
            unconfigured_channels.push_back(name);
            return;
        }

        manager->setup(config);

        if (manager->is_input())
            manager->stream()->exceptions(std::ifstream::failbit | std::ifstream::badbit);
        else
            manager->stream()->exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
    }

    ChannelConfig parse_channel_options(const string & text, const Options & options)
    {
        ChannelConfig config;

        auto colon_pos = text.find(':');
        if (colon_pos == string::npos)
        {
            config.value = text;
        }
        else
        {
            config.value = text.substr(0, colon_pos);
            config.format = text.substr(colon_pos+1);
        }

        config.type = infer_channel_type(config.value);

        if (config.type != "value" and config.format.empty())
        {
            config.format = options.default_channel_format;
        }

        return config;
    }

    string single_input_stream;
    string single_output_stream;
    vector<string> unconfigured_channels;
};

static
bool report_stream_error(ios& stream, const string & name)
{
    if (stream.bad() or (stream.fail() and !stream.eof()))
    {
        cerr << "Error: Transferring stream " << name << "." << endl;
        return false;
    }

    return true;
}

static
void report_channel_config(ostream & s, const ChannelConfig & config)
{
    s << "type: " << config.type << endl;
    s << "value: " << config.value << endl;
    s << "format: " << config.format << endl;
}

template <typename List>
static void print_list(ostream & s, const string & separator, const List & list)
{
    auto i = list.begin();
    if (i == list.end())
        return;
    s << *i;
    for(++i; i != list.end(); ++i)
        s << separator << *i;
}

static
void print_help(Generated_IO & io)
{
    vector<string> input_names;
    vector<string> output_names;

    for(auto & entry : io.input_managers)
        input_names.push_back(entry.first);
    for(auto & entry : io.output_managers)
        output_names.push_back(entry.first);

    std::sort(input_names.begin(), input_names.end());
    std::sort(output_names.begin(), output_names.end());

    cerr << "Options: " << endl;
    cerr << "  -h or --help" << endl;
    cerr << "    ... Print this help." << endl;
    cerr << "  -f=<format> or --format=<format>" << endl;
    cerr << "    ... Use this format for inputs outputs without explicit format." << endl;
    cerr << "  <input>=<value>" << endl;
    cerr << "    ... Define input value." << endl;
    cerr << "  <input>=<source>[:<format>]" << endl;
    cerr << "    ... Read input from source with given format." << endl;
    cerr << "  <output>=<destination>[:<format>]" << endl;
    cerr << "    ... Write output to destination with given format." << endl;;

    cerr << "Sources/Destinations:" << endl;
    cerr << "  pipe: Read from stdin or write to stdout." << endl;
    cerr << "  <filename>: Use file as source/destination." << endl;
    cerr << "Formats: " << endl;
    cerr << "  raw: Binary output as stored in memory." << endl;
    cerr << "  text: Print or parse values using C++ formatting (array elements separated by spaces)." << endl;

    cerr << "Note: If there is a single input (output) or a single stream input (output) "
            "it will use pipe source (destination) with raw format, unless specified otherwise." << endl;

    cerr << "Inputs:" << endl;
    cerr << "  ";
    print_list(cerr, ", ", input_names);
    cerr << endl;

    cerr << "Outputs:" << endl;
    cerr << "  ";
    print_list(cerr, ", ", output_names);
    cerr << endl;


}

int main(int argc, char *argv[])
{
    Generated_IO io;

    Options options;
    bool help_requested = false;

    Arguments::Parser parser;

    for(auto & entry : io.input_managers)
    {
        auto & name = entry.first;
        parser.add_option(name, options.channel_options[name]);
    }

    for(auto & entry : io.output_managers)
    {
        auto & name = entry.first;
        parser.add_option(name, options.channel_options[name]);
    }

    parser.add_option("-f", options.default_channel_format);
    parser.add_option("--format", options.default_channel_format);
    parser.add_switch("-h", help_requested);
    parser.add_switch("--help", help_requested);

    try { parser.parse(argc, argv); }
    catch (Arguments::Parser::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    if (help_requested)
    {
        print_help(io);
        return 0;
    }

    try { Config config(options, io); }
    catch (std::exception & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    Generated_Kernel kernel;
    kernel.io = &io;

    try
    {
        kernel.prelude();

        if (io.has_period)
        {
            while(true)
            {
                kernel.period();
            }
        }
    }
    catch(std::ios_base::failure &)
    {
        bool ok = true;
        for(auto & entry : io.input_managers)
            ok &= report_stream_error(*entry.second->stream(), entry.first);
        for(auto & entry : io.output_managers)
            ok &= report_stream_error(*entry.second->stream(), entry.first);
        if (!ok)
            return 1;
    }
}
