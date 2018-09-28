#include "array_to_isl.hpp"
#include "error.hpp"

#include <isl-cpp/constraint.hpp>

using namespace std;

namespace stream {
namespace functional {

isl::set to_affine_set(expr_ptr e, const space_map & s)
{
    if (auto op = dynamic_pointer_cast<primitive>(e))
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
            auto lhs = to_affine_expr(op->operands[0],s);
            auto rhs = to_affine_expr(op->operands[1],s);
            auto set = isl::set::universe(s.space);
            switch(op->kind)
            {
            case primitive_op::compare_eq:
                set.add_constraint( lhs == rhs ); break;
            case primitive_op::compare_neq:
                set.add_constraint( lhs == rhs );
                set = !set;
                break;
            case primitive_op::compare_l:
                set.add_constraint( lhs < rhs ); break;
            case primitive_op::compare_g:
                set.add_constraint( lhs > rhs ); break;
            case primitive_op::compare_leq:
                set.add_constraint( lhs <= rhs ); break;
            case primitive_op::compare_geq:
                set.add_constraint( lhs >= rhs ); break;
            default:;
            }
            return set;
        }
        case primitive_op::negate:
        {
            assert(op->operands.size() == 1);
            auto op_set = to_affine_set(op->operands[0], s);
            return !op_set;
        }
        case primitive_op::logic_and:
        {
            assert(op->operands.size() == 2);
            auto lhs = to_affine_set(op->operands[0], s);
            auto rhs = to_affine_set(op->operands[1], s);
            return lhs & rhs;
        }
        case primitive_op::logic_or:
        {
            assert(op->operands.size() == 2);
            auto lhs = to_affine_set(op->operands[0], s);
            auto rhs = to_affine_set(op->operands[1], s);
            return lhs | rhs;
        }
        default:;
        }
    }

    throw source_error("Not a linear constraint.", e->location);
}

isl::expression to_affine_expr(expr_ptr e, const space_map & s)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        return s.space.val(c->value);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto avar = dynamic_pointer_cast<array_var>(ref->var);
        if (!avar)
            throw source_error("Invalid type of variable in affine expression.",
                               ref->location);
        int dim = s.index_of(avar);
        if (dim < 0)
            throw error("Free variable in affine expression.");

        if (s.space.is_set())
        {
            return s.space.var(dim);
        }
        else
        {
            return s.space.in(dim);
        }
    }
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        switch(op->kind)
        {
        case primitive_op::add:
            return (to_affine_expr(op->operands[0],s) + to_affine_expr(op->operands[1],s));
        case primitive_op::subtract:
            return (to_affine_expr(op->operands[0],s) - to_affine_expr(op->operands[1],s));
        case primitive_op::negate:
            return (-to_affine_expr(op->operands[0],s));
        case primitive_op::multiply:
        {
            auto lhs = to_affine_expr(op->operands[0],s);
            auto rhs = to_affine_expr(op->operands[1],s);
            if (lhs.is_constant())
                return lhs.constant() * rhs;
            else if (rhs.is_constant())
                return lhs * rhs.constant();
        }
        case primitive_op::divide_integer:
        {
            auto lhs = to_affine_expr(op->operands[0],s);
            auto rhs = to_affine_expr(op->operands[1],s);
            if (rhs.is_constant())
                return isl::floor(lhs / rhs.constant());
        }
        case primitive_op::modulo:
        {
            auto lhs = to_affine_expr(op->operands[0],s);
            auto rhs = to_affine_expr(op->operands[1],s);
            if (rhs.is_constant())
                return lhs % rhs.constant();
        }
        default:;
        }
    }

    throw error("Not an integer affine expression.");
}

isl_model_visitor::isl_model_visitor()
{
    m_space_map.space = isl::space(m_ctx, isl::set_tuple());
}



}
}
