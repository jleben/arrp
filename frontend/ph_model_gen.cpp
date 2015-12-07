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
    {
        auto a = make_array(id);
        output.arrays.push_back(a);
    }

    for (const auto & id : input)
    {
        // FIXME: Hack for recursive arrays
        m_current_id = id;
        make_statements(id, output);
    }
    m_current_id = nullptr;
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
                    text << "Dimension " << (dim+1) << " of array " << id->name
                         << " is infinite.";
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

void polyhedral_gen::make_statements(id_ptr id, ph::model & output)
{
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

                auto s = make_stmt(arr->vars, name, a_case.first, a_case.second);
                output.statements.push_back(s);
            }
        }
        else
        {
            auto s = make_stmt(arr->vars, id->name, nullptr, arr->expr);
            output.statements.push_back(s);
        }
    }
    else
    {
        auto s = make_stmt({}, id->name, nullptr, id->expr);
        output.statements.push_back(s);
    }
}

polyhedral::stmt_ptr polyhedral_gen::make_stmt
(const vector<array_var_ptr> & vars,
 const string & name,
 expr_ptr subdomain_expr, expr_ptr expr)
{
    auto array = m_arrays.at(m_current_id);
    auto domain = array->domain;
    domain.set_name(name);

    auto space = domain.get_space();
    auto local_space = isl::local_space(space);
    space_map sm(space, local_space, vars);

    if (subdomain_expr)
    {
        auto subdomain = to_affine_set(subdomain_expr, sm);
        domain = domain & subdomain;
    }

    cout << "Statement domain:" << endl;
    m_isl_printer.print(domain); cout << endl;

    auto write_rel = isl::basic_map::identity(domain.get_space(),
                                              array->domain.get_space());

    cout << "Write relation: " << endl;
    m_isl_printer.print(write_rel); cout << endl;

    auto stmt = make_shared<ph::statement>(name, domain, write_rel);

    stmt->expr = make_affine_array_reads(stmt, expr, sm);

    cout << "All read relations:" << endl;
    m_isl_printer.print(stmt->read_relations); cout << endl;

    return stmt;
}

expr_ptr polyhedral_gen::make_affine_array_reads
(polyhedral::stmt_ptr stmt, expr_ptr e, const space_map & sm)
{
    if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        for (auto & operand : op->operands)
        {
            operand = make_affine_array_reads(stmt, operand, sm);
        }
        return op;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto av = dynamic_pointer_cast<array_var>(ref->var);
        assert(av);
        int i = sm.index_of(av);
        return make_shared<ph::iterator_read>(i, ref->location);
    }
    else if (auto app = dynamic_pointer_cast<array_app>(e))
    {
        ph::array_ptr arr;

        if (auto ref = dynamic_pointer_cast<reference>(app->object))
        {
            auto id = dynamic_pointer_cast<identifier>(ref->var);
            assert(id);
            arr = m_arrays.at(id);
        }
        else if (auto sref = dynamic_pointer_cast<array_self_ref>(e))
        {
            arr = m_arrays.at(m_current_id);
        }

        auto space = isl::space::from(sm.space, arr->domain.get_space());
        auto local_space = isl::local_space(space);
        space_map sm_rel(space, local_space, sm.vars);

        auto rel = isl::basic_map::universe(sm_rel.space);

        // TODO: assert space sizes vs. args and vars...
        for (int d = 0; d < app->args.size(); ++d)
        {
            auto e = to_affine_expr(app->args[d], sm_rel);
            auto v = isl::expression::variable(sm_rel.local_space,
                                               isl::space::output, d);
            rel.add_constraint(v == e);
        }

        stmt->read_relations = stmt->read_relations | rel;

        cout << "Read relation:" << endl;
        m_isl_printer.print(rel); cout << endl;

        auto read_expr = make_shared<ph::array_read>(arr, rel, app->location);
        return read_expr;
    }
    else
    {
        return e;
    }
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

        if (s.local_space.is_set_space())
        {
            return isl::expression::variable(s.local_space, isl::space::variable, dim);
        }
        else
        {
            return isl::expression::variable(s.local_space, isl::space::input, dim);
        }
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
