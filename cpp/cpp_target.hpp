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

namespace stream {
namespace cpp_gen {

using std::vector;
using std::string;
using std::ostream;

struct buffer
{
    bool has_phase;
    bool on_stack;
    int size;
};

struct renaming {}; // For verbose output


inline basic_type_ptr type_for(primitive_type pt)
{
    switch(pt)
    {
    case primitive_type::boolean:
        return std::make_shared<basic_type>("bool");
    case primitive_type::integer:
        return std::make_shared<basic_type>("int");
    case primitive_type::real32:
        return std::make_shared<basic_type>("float");
    case primitive_type::real64:
        return std::make_shared<basic_type>("double");
    case primitive_type::complex32:
        return std::make_shared<basic_type>("complex<float>");
    case primitive_type::complex64:
        return std::make_shared<basic_type>("complex<double>");
    default:
        throw error("Unexpected primitive type.");
    }
}

inline int size_for(primitive_type pt)
{
    switch(pt)
    {
    case primitive_type::boolean:
        return sizeof(bool);
    case primitive_type::integer:
        return sizeof(int);
    case primitive_type::real32:
        return sizeof(float);
    case primitive_type::real64:
        return sizeof(double);
    case primitive_type::complex32:
        return sizeof(std::complex<float>);
    case primitive_type::complex64:
        return sizeof(std::complex<double>);
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
