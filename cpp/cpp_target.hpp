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

void generate(const string & name,
              const polyhedral::model & model,
              const polyhedral::ast_isl & ast,
              ostream & cpp_file,
              ostream & hpp_file);

}
}

#endif // STREAM_LANG_CPP_TARGET_INCLUDED
