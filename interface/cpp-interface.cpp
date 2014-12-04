#include "cpp-interface.hpp"
#include <json++/json.hh>

namespace stream {
namespace interface {

channel parse_channel(const JSON::Value & data)
{
    using read_error = descriptor::read_error;
    //using descriptor::read_error;

    channel ch;

    if (data.type() != JSON::OBJECT)
        throw read_error("Channel not an object.");

    JSON::Value init_count = data["init"];
    if (init_count.type() != JSON::INT)
        throw read_error("Channel init count not an integer.");

    JSON::Value period_count = data["period"];
    if (period_count.type() != JSON::INT)
        throw read_error("Channel period count not an integer.");

    JSON::Value size = data["size"];
    if (size.type() != JSON::ARRAY)
        throw read_error("Channel size not an array.");

    ch.init_count = init_count.as_int();
    ch.period_count = period_count.as_int();

    JSON::Array size_array = size;
    for(int dim = 0; dim < size_array.size(); ++dim)
    {
        JSON::Value dim_size = size_array[dim];
        if (dim_size.type() != JSON::INT)
            throw read_error("Channel dimension size not an integer.");

        ch.size.push_back(dim_size.as_int());
    }

    return ch;
}

descriptor descriptor::from_file(const string &filename)
{
    descriptor d;

    JSON::Value doc;
    try
    {
        doc = parse_file(filename.c_str());
    }
    catch(...)
    {
        throw read_error("Parsing failed.");
    }

    if (doc.type() != JSON::OBJECT)
        throw read_error("Doc not an object.");

    // Inputs

    JSON::Value inputs = doc["inputs"];

    if (inputs.type() != JSON::ARRAY)
        throw read_error("Inputs not an array.");

    JSON::Array inputs_array = inputs;
    for(int i = 0; i < inputs_array.size(); ++i)
    {
        d.inputs.push_back( parse_channel(inputs_array[i]) );
    }

    // Output

    JSON::Value output = doc["output"];

    d.output = parse_channel(output);

    // Buffers

    JSON::Value buffers = doc["buffers"];
    if (buffers.type() != JSON::ARRAY)
        throw read_error("Buffers not an array.");
    JSON::Array buffers_array = buffers;
    for(int i = 0; i < buffers_array.size(); ++i)
    {
        JSON::Value buffer = buffers_array[i];
        if (buffer.type() != JSON::INT)
            throw read_error("Buffer not an integer.");

        d.buffers.push_back(buffer.as_int());
    }

    return d;
}

} // namespace interface
} // namespace stream
