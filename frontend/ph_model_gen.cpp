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

using my_verbose_out = verbose<polyhedral_gen>;

polyhedral_gen::polyhedral_gen():
    m_isl_printer(m_isl_ctx)
{
    m_isl_ctx.set_error_action(isl::context::abort_on_error);
}

polyhedral::model
polyhedral_gen::process(const unordered_set<id_ptr> & input)
{
    polyhedral::model output;
    output.context = m_isl_ctx;

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

    return output;
}

void polyhedral_gen::add_output(polyhedral::model & model,
                                const string & name, id_ptr id)
{
    auto array = m_arrays.at(id);

    string stmt_name = id->name + "." + name;

    auto tuple = isl::set_tuple( isl::identifier(stmt_name, nullptr), 1 );
    auto space = isl::space(model.context, tuple);
    auto domain = isl::set::universe(space);
    auto lspace = isl::local_space(space);
    {
        auto index = lspace(isl::space::variable, 0);
        if (array->size.empty())
        {
            domain.add_constraint(index == 0);
        }
        else
        {
            domain.add_constraint( index >= 0 );
            if (!array->is_infinite)
                domain.add_constraint( index < array->size[0] );
        }
    }

    auto stmt = make_shared<polyhedral::statement>(domain);
    stmt->is_infinite = array->is_infinite;

    // functional call expression

    auto call = make_shared<polyhedral::external_call>();
    call->name = name;

    vector<expr_ptr> ar_index;
    ar_index.push_back(make_shared<ph::iterator_read>(0));
    for (int d = 1; d < array->domain.dimensions(); ++d)
        ar_index.push_back(make_shared<int_const>(0));

    auto read = make_shared<ph::array_read>(array, ar_index);

    call->args.push_back(read);

    // isl index expression

    {
        auto s = isl::space::from(stmt->domain.get_space(),
                                  array->domain.get_space());

        auto m = isl::multi_expression::zero(s, array->domain.dimensions());
        m.set(0, s.in(0));
        for (int d = 1; d < array->domain.dimensions(); ++d)
        {
            auto e = s.val(0);
            m.set(d, e);
        }

        vector<int> size;
        if (array->size.empty())
        {
            size = {1};
        }
        else
        {
            size = array->size;
            size[0] = 1;
        }

        stmt->read_relations.emplace_back(array, m, size);

        read->relation = &stmt->read_relations.back();

        if (verbose<polyhedral_gen>::enabled())
        {
            cout << "Output read relation:" << endl;
            m_isl_printer.print(m); cout << endl;
        }
    }

    stmt->expr = call;

    model.statements.push_back(stmt);
}

ph::array_ptr polyhedral_gen::make_array(id_ptr id)
{
    string array_name = id->name;

    vector<array_var_ptr> vars;
    if (auto arr = dynamic_pointer_cast<functional::array>(id->expr.expr))
        vars = arr->vars;

    auto tuple = isl::set_tuple( isl::identifier(array_name, nullptr),
                                 std::max((int)vars.size(), 1) );

    auto space = isl::space( m_isl_ctx, tuple );
    auto domain = isl::set::universe(space);
    auto constraint_space = isl::local_space(space);
    bool is_infinite = false;
    vector<int> size;

    if (!vars.empty())
    {
        for (int dim = 0; dim < (int)vars.size(); ++dim)
        {
            auto var = vars[dim];
            int extent = -1;
            if (var->range)
            {
                auto c = dynamic_pointer_cast<constant<int>>(var->range.expr);
                assert_or_throw(bool(c));
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

            size.push_back(extent);
        }
    }
    else
    {
        auto dim_var =
                isl::expression::variable(constraint_space, isl::space::variable, 0);
        domain.add_constraint(dim_var == 0);
    }

    primitive_type element_type;
    if (auto scalar = dynamic_pointer_cast<scalar_type>(id->expr->type))
        element_type = scalar->primitive;
    else if (auto ar = dynamic_pointer_cast<array_type>(id->expr->type))
        element_type = ar->element;
    else
        element_type = primitive_type::undefined;

    assert_or_throw(element_type != primitive_type::undefined);

    auto arr = make_shared<ph::array>(array_name, domain, element_type);
    arr->is_infinite = is_infinite;
    arr->size = size;

    auto result = m_arrays.emplace(id, arr);
    assert(result.second);

    if (my_verbose_out::enabled())
    {
        cout << "Array domain for " << id << ":" << endl;
        cout << "    "; m_isl_printer.print(domain); cout << endl;
    }

    return arr;
}

void polyhedral_gen::make_statements(id_ptr id, ph::model & output)
{
    // TODO: Is sub-domain statement infinite?

    string stmt_name = id->name + ".s";

    if (auto arr = dynamic_pointer_cast<functional::array>(id->expr.expr))
    {
        if (auto case_expr = dynamic_pointer_cast<functional::case_expr>(arr->expr.expr))
        {
            vector<ph::stmt_ptr> case_stmts;
            for(int c = 0; c < (int)case_expr->cases.size(); ++c)
            {
                auto & a_case = case_expr->cases[c];
                auto s = make_stmt(arr->vars, stmt_name + to_string(c),
                                   a_case.first, a_case.second);
                case_stmts.push_back(s);
                output.statements.push_back(s);
            }

            auto ph_arr = m_arrays.at(id);

            auto array_domain = ph_arr->domain;
            array_domain.clear_id();

            auto combined_domain = isl::set(array_domain.get_space());
            for (auto & stmt : case_stmts)
            {
                auto domain = stmt->domain;
                domain.clear_id();
                if (!domain.is_disjoint(combined_domain))
                {
                    throw source_error("Cases are not disjoint.",
                                       case_expr->location);
                }
                combined_domain = combined_domain | domain;
            }

            if (false)
            {
                combined_domain.coalesce();
                if (my_verbose_out::enabled())
                {
                    cout << "Combined domains:" << endl;
                    m_isl_printer.print(combined_domain); cout << endl;
                }
            }

            if (!(combined_domain == array_domain))
            {
                throw source_error("Cases do not cover entire array.",
                                   case_expr->location);
            }
        }
        else
        {
            auto s = make_stmt(arr->vars, stmt_name, nullptr, arr->expr);
            output.statements.push_back(s);
        }
    }
    else
    {
        auto s = make_stmt({}, stmt_name, nullptr, id->expr);
        output.statements.push_back(s);
    }
}

polyhedral::stmt_ptr polyhedral_gen::make_stmt
(const vector<array_var_ptr> & vars,
 const string & name,
 expr_ptr subdomain_expr, expr_ptr expr)
{
    auto array = m_arrays.at(m_current_id);

    ph::stmt_ptr stmt;

    {
        auto domain = array->domain;
        domain.set_name(name);

        stmt = make_shared<ph::statement>(domain);
    }

    auto space = stmt->domain.get_space();
    auto local_space = isl::local_space(space);
    space_map sm(space, local_space, vars);

    m_space_map = &sm;

    if (subdomain_expr)
    {
        auto subdomain = to_affine_set(subdomain_expr, sm);
        stmt->domain = stmt->domain & subdomain;
    }

    {
        // Is the domain bounded?
        auto time_var = stmt->domain.get_space()(isl::space::variable, 0);
        try {
            auto result = stmt->domain.maximum(time_var);
            stmt->is_infinite = result.is_infinity();
        } catch (isl::error &) {
            throw error("Could not check whether statement domain is infinite.");
        }
    }

    {
        int n_dim = array->domain.dimensions();
        stmt->write_relation.array = array;
        assert(stmt->domain.dimensions() == n_dim);
        auto s = isl::space::from(stmt->domain.get_space(),
                                  array->domain.get_space());
        auto m = isl::multi_expression::zero(s, n_dim);
        for (int d = 0; d < n_dim; ++d)
            m.set(d, s.in(d));
        stmt->write_relation.expr = m;
    }

    m_current_stmt = stmt;
    stmt->expr = visit(expr);

    if (my_verbose_out::enabled())
    {
        cout << "Statement domain "
             << (stmt->is_infinite ? "(unbounded)" : "(bounded)")
             << ":" << endl;
        m_isl_printer.print(stmt->domain); cout << endl;

        cout << "Write relation: " << stmt->write_relation.array->name << endl;
        m_isl_printer.print(stmt->write_relation.expr); cout << endl;

        for (const auto & rel : stmt->read_relations)
        {
            cout << "Read relation:" << rel.array->name << endl;
            m_isl_printer.print(rel.expr); cout << endl;
        }
    }

    return stmt;
}

expr_ptr polyhedral_gen::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto av = dynamic_pointer_cast<array_var>(ref->var))
    {
        int i = m_space_map->index_of(av);
        return make_shared<ph::iterator_read>(i, ref->location);
    }
    else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        auto arr = m_arrays.at(id);
        assert(arr->size.empty());

        vector<expr_ptr> index = { make_shared<int_const>(0) };

        auto read_expr = make_shared<ph::array_read>(arr, index, ref->location);

        {
            auto ls = isl::local_space
                    (isl::space::from(m_space_map->space, arr->domain.get_space()));
            ph::array_relation rel(arr, isl::expression::value(ls, 0));
            m_current_stmt->read_relations.push_back(rel);
            read_expr->relation = &m_current_stmt->read_relations.back();
        }

        return read_expr;
    }
    else
    {
        throw error("Unexpected reference type.");
    }
}

expr_ptr polyhedral_gen::visit_array_app
(const shared_ptr<array_app> & app)
{
    auto read_expr = make_shared<ph::array_read>();

    ph::array_ptr arr;

    if (auto ref = dynamic_pointer_cast<reference>(app->object.expr))
    {
        auto id = dynamic_pointer_cast<identifier>(ref->var);
        assert(id);
        arr = m_arrays.at(id);
    }
    else if (auto sref = dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        arr = m_arrays.at(m_current_id);

    }
    else
    {
        throw error("Unexpected expression.");
    }

    read_expr->array = arr;
    read_expr->type = make_shared<scalar_type>(arr->type);

    {
        auto space = isl::space::from(m_space_map->space, arr->domain.get_space());
        auto local_space = isl::local_space(space);
        space_map sm_rel(space, local_space, m_space_map->vars);

        auto m = isl::multi_expression::zero(space, app->args.size());

        assert(app->args.size() == space.dimension(isl::space::output));
        for (int a = 0; a < app->args.size(); ++a)
        {
            auto e = to_affine_expr(app->args[a], sm_rel);
            m.set(a, e);
        }

        m_current_stmt->read_relations.emplace_back(arr, m);

        read_expr->relation = &m_current_stmt->read_relations.back();
    }

    for (auto & arg : app->args)
    {
        read_expr->indexes.push_back(visit(arg));
    }

    return read_expr;
}

isl::set polyhedral_gen::to_affine_set(expr_ptr e, const space_map & s)
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
            throw error("Free variable in affine expression.");

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
        default:;
        }
    }

    throw error("Not an integer affine expression.");
}




}
}
