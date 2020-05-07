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

#ifndef STREAM_LANG_CPP_TARGET_INCLUDED
#define STREAM_LANG_CPP_TARGET_INCLUDED

#include "../common/ph_model.hpp"
#include "../utility/cpp-gen.hpp"
#include "../compiler/options.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>

namespace stream {
namespace cpp_gen {

using std::vector;
using std::string;
using std::ostream;

struct buffer
{
    string name;
    primitive_type type;
    int period_offset = 0;

    bool has_phase = false;

    struct
    {
        int source = 0;
        int size = 0;
        int period_count = 0;
    }
    data_shift;

    bool on_stack;
    vector<int> dimension_size;
    vector<bool> dimension_needs_wrapping;

    int64_t size;
};

// For verbose output
struct cpp_target {};

inline
string type_name_for(primitive_type pt)
{
    switch(pt)
    {
    case primitive_type::boolean:
        return ("bool");
    case primitive_type::int8:
        return ("int8_t");
    case primitive_type::uint8:
        return ("uint8_t");
    case primitive_type::int16:
        return ("int16_t");
    case primitive_type::uint16:
        return ("uint16_t");
    case primitive_type::int32:
        return ("int32_t");
    case primitive_type::uint32:
        return ("uint32_t");
    case primitive_type::int64:
        return ("int64_t");
    case primitive_type::uint64:
        return ("uint64_t");
    case primitive_type::real32:
        return ("float");
    case primitive_type::real64:
        return ("double");
    case primitive_type::complex32:
        return ("complex<float>");
    case primitive_type::complex64:
        return ("complex<double>");
    default:
        throw error("Unexpected primitive type.");
    }
}

inline basic_type_ptr type_for(primitive_type pt)
{
    return std::make_shared<basic_type>(type_name_for(pt));
}

inline int size_for(primitive_type pt)
{
    switch(pt)
    {
    case primitive_type::boolean:
        return 4;
    case primitive_type::int8:
    case primitive_type::uint8:
        return 1;
    case primitive_type::int16:
    case primitive_type::uint16:
        return 2;
    case primitive_type::int32:
    case primitive_type::uint32:
        return 4;
    case primitive_type::int64:
    case primitive_type::uint64:
        return 8;
    case primitive_type::real32:
        return 32;
    case primitive_type::real64:
        return 64;
    case primitive_type::complex32:
        return 2 * 32;
    case primitive_type::complex64:
        return 2 * 64;
    default:
        throw error("Unexpected primitive type.");
    }
}

void generate(const string & name,
              const polyhedral::model & model,
              const polyhedral::ast_isl & ast,
              ostream & cpp_file,
              const compiler::options &);

}
}

#endif // STREAM_LANG_CPP_TARGET_INCLUDED
