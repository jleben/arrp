#include "ph_model_gen.hpp"
#include "error.hpp"

#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/matrix.hpp>
#include <isl-cpp/utility.hpp>
#include <isl-cpp/printer.hpp>

#include <iostream>

using namespace std;

namespace stream {
namespace functional {

namespace ph = stream::polyhedral;

polyhedral_gen::polyhedral_gen():
    m_isl_printer(m_isl_ctx)
{

}

void polyhedral_gen::process(const unordered_set<id_ptr> & input, ph::model & output)
{
    for (const auto & id : input)
        process(id, output);
}

void polyhedral_gen::process(id_ptr id, ph::model & output)
{
    auto ph_array = make_array(id);

    // TODO: Is sub-domain statement infinite?

    if (auto arr = dynamic_pointer_cast<functional::array>(id->expr))
    {
        if (auto case_expr = dynamic_pointer_cast<functional::case_expr>(arr->expr))
        {
            for(int c = 0; c < (int)case_expr->cases.size(); ++c)
            {
                auto & a_case = case_expr->cases[c];

                string name;
                {
                    ostringstream text;
                    text << id->name << '.' << c;
                    name = text.str();
                }

                make_stmt(ph_array, arr->vars, name, a_case.first);
            }
        }
        else
        {
            make_stmt(ph_array, arr->vars, id->name, nullptr);
        }
    }
    else
    {
        make_stmt(ph_array, {}, id->name, nullptr);
    }
}


ph::array_ptr polyhedral_gen::make_array(id_ptr id)
{
    ostringstream text;
    text << '@' << id->name;
    string name = text.str();

    vector<array_var_ptr> vars;
    if (auto arr = dynamic_pointer_cast<functional::array>(id->expr))
        vars = arr->vars;

    auto tuple = isl::set_tuple( isl::identifier(name, nullptr),
                                 std::max((int)vars.size(), 1) );

    auto space = isl::space( m_isl_ctx, tuple );
    auto domain = isl::set::universe(space);
    auto constraint_space = isl::local_space(space);
    bool is_infinite = false;

    if (!vars.empty())
    {
        for (int dim = 0; dim < (int)vars.size(); ++dim)
        {
            auto var = vars[dim];
            int extent = -1;
            if (var->range)
            {
                auto c = dynamic_pointer_cast<constant<int>>(var->range);
                if (!c)
                    throw source_error("Array bound not constant.",
                                       var->range->location);
                extent = c->value;
            }
            else
            {
                if (dim != 0)
                {
                    ostringstream text;
                    text << "Dimension " << dim << " of array " << name
                         << " is infinite." << endl;
                    throw source_error(text.str(), id->location);
                }
            }

            auto dim_var =
                    isl::expression::variable(constraint_space, isl::space::variable, dim);

            auto lower_bound = dim_var >= 0;
            domain.add_constraint(lower_bound);

            if (extent >= 0)
            {
                auto upper_bound = dim_var < extent;
                domain.add_constraint(upper_bound);
            }
            else
            {
                is_infinite = true;
            }
        }
    }
    else
    {
        auto dim_var =
                isl::expression::variable(constraint_space, isl::space::variable, 0);
        domain.add_constraint(dim_var == 0);
    }

    auto arr = make_shared<ph::array>(name, domain);
    arr->is_infinite = is_infinite;

    auto result = m_arrays.emplace(id, arr);
    assert(result.second);

    cout << "Array domain:" << endl;
    m_isl_printer.print(domain); cout << endl;

    return arr;
}

polyhedral::stmt_ptr polyhedral_gen::make_stmt
(polyhedral::array_ptr array,
 const vector<array_var_ptr> & vars,
 const string & name,
 expr_ptr subdomain_expr)
{
    auto domain = array->domain;
    domain.set_name(name);

    if (subdomain_expr)
    {
        auto space = domain.get_space();
        auto local_space = isl::local_space(space);
        space_map sm(space, local_space, vars);

        auto subdomain = to_affine_set(subdomain_expr, sm);
        domain = domain & subdomain;
    }

    cout << "Statement domain:" << endl;
    m_isl_printer.print(domain); cout << endl;

    // TODO: Array write relation

    auto stmt = make_shared<ph::statement>(name, domain);
    return stmt;
}

isl::set polyhedral_gen::to_affine_set(expr_ptr e, const space_map & s)
{
    if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        switch(op->type)
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
            switch(op->type)
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

isl::expression polyhedral_gen::to_affine_expr(expr_ptr e, const space_map & s)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        return isl::expression::value(s.local_space, c->value);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto avar = dynamic_pointer_cast<array_var>(ref->var);
        if (!avar)
            throw source_error("Invalid type of variable in affine expression.",
                               ref->location);
        int dim = s.index_of(avar);
        if (dim < 0)
            throw source_error("Free variable.", ref->location);

        return isl::expression::variable(s.local_space, isl::space::variable, dim);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        switch(op->type)
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
        default:;
        }
    }

    throw source_error("Not an integer affine expression.", e->location);
}

}
}
