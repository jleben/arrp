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

#include "../common/primitives.hpp"
#include "array_func_printer.hpp"

#include <iomanip>
#include <vector>

namespace stream {
namespace polyhedral {

using namespace std;


void array_func_printer::print(const array_index_expr & expr, ostream & s)
{
    int count = expr.size();
    for(const auto & term : expr)
    {
        if (term.var)
        {
            if (term.scalar != 1)
                s << term.scalar;
            s << name(term.var);
        }
        else
        {
            s << term.scalar;
        }
        if (--count)
            s << "+";
    }
}

void array_func_printer::print(const vector<array_index_expr> & exprs, ostream & s)
{
    int count = exprs.size();
    for (auto & expr : exprs)
    {
        print(expr, s);
        if (--count)
            s << ",";
    }
}

array_func_printer::array_func_printer(): level(0) {}

void array_func_printer::print(const primitive_type t, ostream &s)
{
    switch(t)
    {
    case primitive_type::integer:
        s << "integer";
        break;
    case primitive_type::real:
        s << "real";
        break;
    case primitive_type::boolean:
        s << "boolean";
        break;
    }
}

void array_func_printer::print(const expression *expr, ostream &s)
{
    if (auto const_int = dynamic_cast<const constant<int>*>(expr))
    {
        s << const_int->value;
    }
    else if (auto const_double = dynamic_cast<const constant<double>*>(expr))
    {
        s << const_double->value;
    }
    else if (auto primitive = dynamic_cast<const primitive_expr*>(expr))
    {
        s << primitive->op;

        s << " ( ";
        for(auto & operand : primitive->operands)
        {
            print(operand.get(), s);
            if (operand != primitive->operands.back())
                s << ", ";
        }
        s << " )";
    }
    else if (auto it = dynamic_cast<const iterator_access*>(expr))
    {
        print(it->expr, s);
    }
    else if (auto access = dynamic_cast<const array_access*>(expr))
    {
        s << access->target->name << "[";
        print(access->index, s);
        s << "]";
    }
    else if (auto access = dynamic_cast<const input_access*>(expr))
    {
        s << "_in_" << access->index << "";
        if (!access->array_index.empty())
        {
            s << "[";
            print(access->array_index, s);
            s << "]";
        }
    }
    else if (auto func = dynamic_cast<const array_function*>(expr))
    {
        s << "\\";
        {
            int count = func->vars.size();
            for (auto var : func->vars)
            {
                s << name(var);
                if (--count)
                    s << ",";
            }
        }
        s << " -> { ";
        print(func->expr.get(), s);
        s << " }";
    }
    else
    {
        s << "?";
    }
}

void array_func_printer::print(statement_ptr stmt, ostream & s)
{
    s << stmt->name << ": " << stmt->array->name;
    s << "["; print(stmt->write_index, s); s << "] <= ";
    print(stmt->expr.get(), s);
}

}
}
