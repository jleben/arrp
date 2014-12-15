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

#include "types.hpp"
#include "ast.hpp"

namespace stream {
namespace semantic {

ast::node_ptr function::expression() const
{
    return statement->as_list()->elements[2];
}

type_ptr function::result_type() const
{
    return expression()->semantic_type;
}


void function::print_on( ostream & s ) const
{
    s << name;
    s << " (";
    int p = 0;
    for (const string & param : parameters)
    {
        ++p;
        s << param;
        if (p < parameters.size())
            s << ", ";
    }
    s << ") -> ";
    type_ptr t = result_type();
    if (t)
        s << *t;
    else
        s << "?";
}

}
}
