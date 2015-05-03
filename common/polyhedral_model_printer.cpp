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
#include "polyhedral_model_printer.hpp"

#include <iomanip>
#include <vector>

namespace stream {
namespace polyhedral {

using namespace std;
#if 0
string name_of_primitive( primitive_op op )
{
    switch(op)
    {
    case primitive_op::negate:
        return "!";
    case primitive_op::add:
        return "+";
    case primitive_op::subtract:
        return "-";
    case primitive_op::multiply:
        return "*";
    case primitive_op::divide:
        return "/";
    case primitive_op::divide_integer:
        return ":";
    case primitive_op::raise:
        return "^";
    case primitive_op::exp:
        return "exp";
    case primitive_op::exp2:
        return "exp2";
    case primitive_op::log:
        return "log";
    case primitive_op::log2:
        return "log2";
    case primitive_op::log10:
        return "log10";
    case primitive_op::sqrt:
        return "sqrt";
    case primitive_op::sin:
        return "sin";
    case primitive_op::cos:
        return "cos";
    case primitive_op::tan:
        return "tan";
    case primitive_op::asin:
        return "asin";
    case primitive_op::acos:
        return "acos";
    case primitive_op::atan:
        return "atan";
    case primitive_op::ceil:
        return "ceil";
    case primitive_op::floor:
        return "floor";
    case primitive_op::abs:
        return "abs";
    case primitive_op::min:
        return "min";
    case primitive_op::max:
        return "max";
    case primitive_op::conditional:
        return "if";
    default:
        return "<unknown primitive op>";
    }
}
#endif

printer::printer(): level(0) {}

void printer::print(const primitive_type t, ostream &s)
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
    else if (auto primitive = dynamic_cast<const primitive_expr*>(expr))
    {
        s << primitive->op;

        s << " (" << endl;
        indent();
        for(auto & operand : primitive->operands)
        {
            s << indentation();
            print(operand.get(), s);
            if (operand != primitive->operands.back())
                s << ",";
            s << endl;
        }
        unindent();
        s << indentation() << ")";
    }
    else if (auto it = dynamic_cast<const iterator_access*>(expr))
    {
        s << "iterator: ";
        print(it->relation, s);
    }
    else if (auto access = dynamic_cast<const array_access*>(expr))
    {
        s << "read: " << access->target->name << " ";
        print(access->pattern, s);
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
    s << " ]:" << endl;
    if (stmt->array)
    {
        s << stmt->array->name
          << " [" << endl
          << stmt->write_relation
          << "]";
    }
    else
    {
        s << " <no array>";
    }
    s << " = ";
    if (stmt->expr)
    {
        s << endl;
        indent();

        s << indentation();
        print(stmt->expr.get(), s);

        unindent();
    }
    else
        s << "...";
    s << endl;
}

void printer::print(const mapping & m, ostream & s)
{
    s << "[" << endl;
    indent();
    for (int row = 0; row < m.output_dimension(); ++row)
    {
        s << indentation();
        for (int col = 0; col < m.input_dimension(); ++col)
        {
            s << std::setw(4) << m.coefficients(row,col);
        }
        s << " | " << m.constants[row];
        s << endl;
    }
    unindent();
    s << indentation() << "]";
}

}
}
