/*
Compiler for language for stream processing

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED
#define STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED

#include "../common/ph_model.hpp"
#include "../utility/debug.hpp"

namespace stream {
namespace polyhedral {

struct ast_gen {}; // for verbose output

ast_isl make_isl_ast(schedule &, bool separate_loops );

}
}


#endif // STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED

