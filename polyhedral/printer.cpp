#include "printer.hpp"

#include <iomanip>

namespace stream {
namespace polyhedral {

using namespace std;

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
        switch(intr->kind)
        {
        case intrinsic::add:
            s << "+"; break;
        case intrinsic::subtract:
            s << "-"; break;
        case intrinsic::multiply:
            s << "*"; break;
        case intrinsic::divide:
            s << "/"; break;
        case intrinsic::raise:
            s << "^"; break;
        case intrinsic::negate:
            s << "!"; break;
        case intrinsic::exp:
            s << "exp"; break;
        case intrinsic::log:
            s << "log"; break;
        default:
            s << "unknown-intrinsic";
        }

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
