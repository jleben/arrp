#include "linear_expr_gen.hpp"
#include "error.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

linexpr to_linear_expr(expr_ptr e)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        return linexpr(c->value);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto avar = dynamic_pointer_cast<array_var>(ref->var);
        if (!avar)
            throw source_error("Invalid type of variable in linear expression.",
                               ref->location);
        return linexpr(ref->var);
    }
    // TODO: Function variables: get bound value, not type
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        switch(op->type)
        {
        case primitive_op::add:
            return (to_linear_expr(op->operands[0]) + to_linear_expr(op->operands[1]));
        case primitive_op::subtract:
            return (to_linear_expr(op->operands[0]) + -to_linear_expr(op->operands[1]));
        case primitive_op::negate:
            return (-to_linear_expr(op->operands[0]));
        case primitive_op::multiply:
        {
            auto lhs = to_linear_expr(op->operands[0]);
            auto rhs = to_linear_expr(op->operands[1]);
            if (!lhs.is_constant() && !rhs.is_constant())
                throw source_error("Not an integer linear expression.", e->location);
            if (lhs.is_constant())
                return rhs * lhs.constant();
            else
                return lhs * rhs.constant();
        }
        default:
            throw source_error("Not an integer linear expression.", e->location);
        }
    }
    else
        throw source_error("Not an integer linear expression.", e->location);
}

linexpr maximum(const linexpr & expr)
{
    linexpr max_expr;
    for (const auto & term : expr)
    {
        int value = term.second;
        if (term.first)
        {
            auto avar = dynamic_pointer_cast<array_var>(term.first);
            assert(avar);
            if (auto c = dynamic_pointer_cast<constant<int>>(avar->range))
                value *= std::max(0, c->value - 1);
            else // unconstrained
            {
                max_expr = max_expr + term;
                continue;
            }
        }
        max_expr = max_expr + value;
    }
    return max_expr;
}

}
}
