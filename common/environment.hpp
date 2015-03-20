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

#ifndef STREAM_LANG_ENVIRONMENT_INCLUDED
#define STREAM_LANG_ENVIRONMENT_INCLUDED

#include "ast.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cassert>

namespace stream {
namespace semantic {

using std::string;
using std::vector;
using std::unordered_map;

struct symbol
{
    enum symbol_type
    {
        expression,
        function
    };

    //symbol() {}

    symbol(symbol_type t, string name, const ast::node_ptr & src = ast::node_ptr()):
        type(t),
        name(name),
        source(src)
    {}

    symbol_type type;
    string name;
    vector<string> parameter_names;
    ast::node_ptr source;
    ast::node_ptr source_expression() const
    { return source->as_list()->elements[2]; }
};

struct environment : public unordered_map<string, symbol>
{
    environment();
};

std::ostream & operator<<(std::ostream & s, const environment & env);

}
}

#endif // STREAM_LANG_ENVIRONMENT_INCLUDED
