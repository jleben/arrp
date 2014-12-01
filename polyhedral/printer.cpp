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
    table[intrinsic::raise] = "^";
    table[intrinsic::exp] = "exp";
    table[intrinsic::exp2] = "exp2";
    table[intrinsic::log] = "log";
    table[intrinsic::log2] = "log2";
    table[intrinsic::log10] = "log10";
    table[intrinsic::pow] = "pow";
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

void printer::print(expression *expr, ostream &s)
{
    if (auto const_int = dynamic_cast<constant<int>*>(expr))
    {
        s << const_int->value;
    }
    else if (auto const_double = dynamic_cast<constant<double>*>(expr))
    {
        s << const_double->value;
    }
    else if (auto intr = dynamic_cast<intrinsic*>(expr))
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
    else if (auto it = dynamic_cast<iterator_access*>(expr))
    {
        s << "iterator: " << it->offset << " + " << it->ratio << " * "
          << '[' << it->dimension << ']';
    }
    else if (auto access = dynamic_cast<stream_access*>(expr))
    {
        s << "access: " << access->target << " [" << endl;
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
    else if (auto access = dynamic_cast<reduction_access*>(expr))
    {
        s << "reduction access: "
          << access->initializer
          << " / "
          << access->reductor;
    }
    else if (auto access = dynamic_cast<input_access*>(expr))
    {
        s << "input access: "
          << access->index;
    }
    else
    {
        s << "unexpected expression";
    }
}

void printer::print(statement *stmt, ostream &s )
{
    s << stmt << " [";
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
