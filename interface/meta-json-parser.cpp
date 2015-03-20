/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "meta-json-parser.hpp"
#include <json++/json.hh>

namespace stream {
namespace meta_json {

numerical_type parse_type(const JSON::Value & data)
{
    using read_error = descriptor::read_error;

    if (data.type() != JSON::STRING)
        throw read_error("Numerical type not a string.");
    string type_str = data.as_string();
    if (type_str == "integer")
        return integer;
    else if (type_str == "real")
        return real;
    else
        throw read_error("Invalid numerical type: " + type_str);
}

descriptor::channel parse_channel(const JSON::Value & data)
{
    using read_error = descriptor::read_error;
    using channel = descriptor::channel;

    channel ch;

    if (data.type() != JSON::OBJECT)
        throw read_error("Channel not an object.");

    ch.type = parse_type(data["type"]);

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
        if (buffer.type() != JSON::OBJECT)
            throw read_error("Buffer not an object.");

        JSON::Value type = buffer["type"];
        JSON::Value size = buffer["size"];

        if (size.type() != JSON::INT)
            throw read_error("Buffer size not an integer.");

        descriptor::buffer b;
        b.type = parse_type(type);
        b.size = size.as_int();

        d.buffers.push_back(b);
    }

    return d;
}

} // namespace interface
} // namespace stream
