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

tuple<expr_ptr, affine_integer_expression_check::affine_expr_type>
affine_integer_expression_check::ensure_expression_get_type(expr_ptr expr)
{
    if (auto cint = dynamic_pointer_cast<int_const>(expr))
    {
        return { expr, affine_constant };
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
        {
            return { expr, affine_variable };
        }
        else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            if (!id->is_recursive)
                return ensure_expression_get_type(m_copier.copy(id->expr));
        }
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        vector<affine_expr_type> types;
        types.reserve(op->operands.size());

        for (auto & operand : op->operands)
        {
            affine_expr_type type;
            std::tie(operand, type) = ensure_expression_get_type(operand);
            types.push_back(type);
        }

        switch(op->kind)
        {
        case primitive_op::add:
        case primitive_op::subtract:
        {
            auto lhs = types[0];
            auto rhs = types[1];
            if (lhs == affine_constant && rhs == affine_constant)
                return { expr, affine_constant };
            return { expr, affine_variable };
            break;
        }
        case primitive_op::multiply:
        {
            auto lhs = types[0];
            auto rhs = types[1];
            if (lhs == affine_constant && rhs == affine_constant)
                return { expr, affine_constant };
            if (lhs == affine_constant || rhs == affine_constant)
                return { expr, affine_variable };
            break;
        }
        case primitive_op::divide_integer:
        case primitive_op::modulo:
        {
            auto lhs = types[0];
            auto rhs = types[1];
            if (lhs == affine_constant && rhs == affine_constant)
                return { expr, affine_constant };
            if (rhs == affine_constant)
                return { expr, affine_variable };
            break;
        }
        default:
            break;
        }
    }

    throw source_error("Not an affine integer expression.", expr->location);
}

expr_ptr affine_integer_expression_check::ensure_expression(expr_ptr expr)
{
    auto [e, t] = ensure_expression_get_type(expr);
    std::ignore = t;
    return e;
}

expr_ptr affine_integer_expression_check::ensure_contraint(expr_ptr expr)
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
            op->operands[0] = ensure_expression(op->operands[0]);
            op->operands[1] = ensure_expression(op->operands[1]);
            return expr;
        }
        case primitive_op::negate:
        {
            assert(op->operands.size() == 1);
            op->operands[0] = ensure_contraint(op->operands[0]);
            return expr;
        }
        case primitive_op::logic_and:
        case primitive_op::logic_or:
        {
            assert(op->operands.size() == 2);
            op->operands[0] = ensure_contraint(op->operands[0]);
            op->operands[1] = ensure_contraint(op->operands[1]);
            return expr;
        }
        default:
            break;
        }
    }

    throw source_error("Not an affine integer constraint.", expr->location);
}

}
}
