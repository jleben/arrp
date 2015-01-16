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

#include "printer.hpp"

#include <iomanip>
#include <vector>

namespace stream {
namespace polyhedral {

using namespace std;

vector<string> make_intrinsic_name_table()
{
    vector<string> table(intrinsic::count);
    table[intrinsic::negate] = "!";
    table[intrinsic::add] = "+";
    table[intrinsic::subtract] = "-";
    table[intrinsic::multiply] = "*";
    table[intrinsic::divide] = "/";
    table[intrinsic::divide_integer] = ":";
    table[intrinsic::raise] = "^";
    table[intrinsic::exp] = "exp";
    table[intrinsic::exp2] = "exp2";
    table[intrinsic::log] = "log";
    table[intrinsic::log2] = "log2";
    table[intrinsic::log10] = "log10";
    table[intrinsic::sqrt] = "sqrt";
    table[intrinsic::sin] = "sin";
    table[intrinsic::cos] = "cos";
    table[intrinsic::tan] = "tan";
    table[intrinsic::asin] = "asin";
    table[intrinsic::acos] = "acos";
    table[intrinsic::atan] = "atan";
    table[intrinsic::ceil] = "ceil";
    table[intrinsic::floor] = "floor";
    table[intrinsic::abs] = "abs";
    table[intrinsic::min] = "min";
    table[intrinsic::max] = "max";
    table[intrinsic::conditional] = "if";
    return table;
}

string name_of_intrinsic( intrinsic::of_kind type )
{
    static vector<string> intrinsic_names = make_intrinsic_name_table();

    if (type >= intrinsic_names.size())
        return "<unknown intrinsic>";
    else
        return intrinsic_names[type];
}

printer::printer(): level(0) {}

void printer::print(const numerical_type t, ostream &s)
{
    switch(t)
    {
    case integer:
        s << "integer";
        break;
    case real:
        s << "real";
        break;
    case boolean:
        s << "boolean";
        break;
    }
}

void printer::print(const expression *expr, ostream &s)
{
    if (auto const_int = dynamic_cast<const constant<int>*>(expr))
    {
        s << const_int->value;
    }
    else if (auto const_double = dynamic_cast<const constant<double>*>(expr))
    {
        s << const_double->value;
    }
    else if (auto intr = dynamic_cast<const intrinsic*>(expr))
    {
        s << name_of_intrinsic(intr->kind);

        s << " (" << endl;
        indent();
        for(expression *operand : intr->operands)
        {
            s << indentation();
            print(operand, s);
            if (operand != intr->operands.back())
                s << ",";
            s << endl;
        }
        unindent();
        s << indentation() << ")";
    }
    else if (auto it = dynamic_cast<const iterator_access*>(expr))
    {
        s << "iterator: " << it->offset << " + " << it->ratio << " * "
          << '[' << it->dimension << ']';
    }
    else if (auto access = dynamic_cast<const stmt_access*>(expr))
    {
        s << "access: " << access->target->name << " [" << endl;
        indent();
        for (int row = 0; row < access->pattern.output_dimension(); ++row)
        {
            s << indentation();
            for (int col = 0; col < access->pattern.input_dimension(); ++col)
            {
                s << std::setw(4) << access->pattern.coefficients(row,col);
            }
            s << " | " << access->pattern.constants[row];
            s << endl;
        }
        unindent();
        s << indentation() << "]";
#if 0
        indent();
        for(const auto & pattern : access->pattern)
        {
            s << indentation();
            s << pattern.source_dimension;
            s << " -> " << pattern.target_dimension;
            s << " @ " << pattern.offset;
            s << " % " << pattern.stride;
            s << endl;
        }
        unindent();
        s << indentation() << "]";
#endif
    }
    else if (auto access = dynamic_cast<const reduction_access*>(expr))
    {
        s << "reduction access: "
          << access->initializer->name
          << " / "
          << access->reductor->name;
    }
    else if (auto access = dynamic_cast<const input_access*>(expr))
    {
        s << "input access: "
          << access->index;
    }
    else
    {
        s << "unexpected expression";
    }
}

void printer::print(const statement *stmt, ostream &s )
{
    s << stmt->name << " [";
    for(int size : stmt->domain)
        s << " " << size;
    s << " ] = ";
    if (stmt->expr)
    {
        s << endl;
        indent();
        s << indentation();
        print(stmt->expr, s);
        unindent();
    }
    else
        s << "...";
    s << endl;
}

}
}
