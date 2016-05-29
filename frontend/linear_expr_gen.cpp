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
        switch(op->kind)
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
            if (auto c = dynamic_pointer_cast<constant<int>>(avar->range.expr))
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

linear_set::constraint to_linear_constraint(expr_ptr expr)
{
    if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        linear_set::constraint c;

        switch(op->kind)
        {
        case primitive_op::compare_eq:
            c.type = linear_set::equal; break;
        case primitive_op::compare_neq:
            c.type = linear_set::not_equal; break;
        case primitive_op::compare_l:
            c.type = linear_set::lesser; break;
        case primitive_op::compare_g:
            c.type = linear_set::greater; break;
        case primitive_op::compare_leq:
            c.type = linear_set::lesser_or_equal; break;
        case primitive_op::compare_geq:
            c.type = linear_set::greater_or_equal; break;
        default:
            throw source_error("Not a linear constraint.", expr->location);
        }

        assert(op->operands.size() == 2);

        c.lhs = to_linear_expr(op->operands[0]);
        c.rhs = to_linear_expr(op->operands[1]);

        return c;
    }
    else
    {
        throw source_error("Not a linear constraint.", expr->location);
    }
}

expr_ptr to_linear_set(expr_ptr expr)
{
    if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        switch(op->kind)
        {
        case primitive_op::compare_eq:
        case primitive_op::compare_neq:
        case primitive_op::compare_l:
        case primitive_op::compare_g:
        case primitive_op::compare_leq:
        case primitive_op::compare_geq:
        {
            assert(op->operands.size() == 2);
            auto & lhs = op->operands[0];
            auto & rhs = op->operands[1];
            if (!lhs->type->is_affine())
                throw source_error("Not an affine expression.", lhs.location);
            if (!rhs->type->is_affine())
                throw source_error("Not an affine expression.", rhs.location);
            return op;
        }
        case primitive_op::negate:
        {
            assert(op->operands.size() == 1);
            to_linear_set(op->operands[0]);
            return op;
        }
        case primitive_op::logic_and:
        case primitive_op::logic_or:
        {
            assert(op->operands.size() == 2);
            to_linear_set(op->operands[0]);
            to_linear_set(op->operands[1]);
            return op;
        }
        default:
            throw source_error("Not a linear constraint.", expr->location);
        }
    }
    else
    {
        throw source_error("Not a linear constraint.", expr->location);
    }
}

}
}
