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
#include "error.hpp"

using namespace std;

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

type_structure structure(const type_ptr & t)
{
    type_structure structure;

    switch(t->get_tag())
    {
    case type::boolean:
        structure.type = primitive_type::boolean;
        break;
    case type::integer_num:
        structure.type = primitive_type::integer;
        break;
    case type::real_num:
        structure.type = primitive_type::real;
        break;
    case type::stream:
    {
        const stream & st = t->as<stream>();
        structure.type = st.element_type;
        structure.size = st.size;
        break;
    }
    default:
        throw undefined();
    }

    return structure;
}

primitive_type operator+ (primitive_type a, primitive_type b)
{
    if (a == primitive_type::boolean || b == primitive_type::boolean)
    {
        if (a == b)
            return primitive_type::boolean;
        else
            throw undefined();
    }
    else if (a == primitive_type::integer && b == primitive_type::integer)
        return primitive_type::integer;
    else
        return primitive_type::real;
}

type_ptr operator+ (const type_ptr & a, const type_ptr & b)
{
    type_structure a_struct = structure(a);
    type_structure b_struct = structure(b);

    if (a_struct.size != b_struct.size)
        throw undefined();

    vector<int> & common_size = a_struct.size;
    primitive_type common_type = a_struct.type + b_struct.type;

    if (common_size.size())
    {
        return make_shared<stream>(common_size, common_type);
    }
    else
    {
        return type_for(common_type);
    }
}

type_ptr type_for(primitive_type pt)
{
    switch(pt)
    {
    case primitive_type::integer:
        return make_shared<integer_num>();
    case primitive_type::real:
        return make_shared<real_num>();
    case primitive_type::boolean:
        return make_shared<boolean>();
    default:
        throw undefined();
    }
}

primitive_type primitive_type_for(type::tag tag)
{
    switch(tag)
    {
    case type::boolean:
        return primitive_type::boolean;
    case type::integer_num:
        return primitive_type::integer;
    case type::real_num:
        return primitive_type::real;
    default:
        throw undefined();
    }
}

primitive_type primitive_type_for(const type_ptr &t)
{
    switch(t->get_tag())
    {
    case type::boolean:
        return primitive_type::boolean;
    case type::integer_num:
        return primitive_type::integer;
    case type::real_num:
        return primitive_type::real;
    case type::stream:
        return t->as<stream>().element_type;
    default:
        throw undefined();
    }
}

}
}
