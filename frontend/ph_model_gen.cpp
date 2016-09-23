#include "ph_model_gen.hpp"
#include "error.hpp"
#include "../utility/stacker.hpp"

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

    make_time_array();
}

void polyhedral_gen::make_time_array()
{
    string array_name(".time");

    auto array_space = isl::space(m_isl_ctx, isl::set_tuple(isl::identifier(array_name), 1));
    auto array_domain = isl::set::universe(array_space);
    array_domain.add_constraint(array_space.var(0) >= 0);

    auto ar = make_shared<ph::array>(array_name,  array_domain, primitive_type::integer);
    ar->is_infinite = true;
    ar->size = { ph::infinite };

    m_time_array = ar;

    // s0(0): a[0] = 0
    // s1(t): a[t] = a[t-1] + 1
    {
        auto s0_space = isl::space(m_isl_ctx, isl::set_tuple(isl::identifier(".time.s0"), 1));
        auto s0_dom = isl::set::universe(s0_space);
        s0_dom.add_constraint(s0_space.var(0) == 0);

        auto s0 = make_shared<ph::statement>(s0_dom);
        s0->expr = make_shared<int_const>(0);

        auto wrel_space = isl::space::from(s0->domain.get_space(), ar->domain.get_space());
        auto wrel_map = isl::basic_map::universe(wrel_space);
        wrel_map.add_constraint(wrel_space.out(0) == 0);

        s0->write_relation.array = ar;
        s0->write_relation.map = wrel_map;

        m_time_stmts.push_back(s0);
    }
    {
        auto s1_space = isl::space(m_isl_ctx, isl::set_tuple(isl::identifier(".time.s1"), 1));
        auto s1_dom = isl::set::universe(s1_space);
        s1_dom.add_constraint(s1_space.var(0) > 0);

        auto s1 = make_shared<ph::statement>(s1_dom);

        {
            auto i = make_shared<ph::iterator_read>(0);
            auto j = make_shared<primitive>(primitive_op::subtract, i, make_shared<int_const>(1));
            j->type = make_int_type();
            auto t0 = make_shared<ph::array_read>(ar, vector<expr_ptr>({j}));
            auto t1 = make_shared<primitive>(primitive_op::add, t0, make_shared<int_const>(1));
            t1->type = make_int_type();

            s1->expr = t1;

            {
                auto s = isl::space::from(s1->domain.get_space(), ar->domain.get_space());
                auto m = isl:: basic_map::universe(s);
                m.add_constraint(s.out(0) == s.in(0) - 1);

                s1->read_relations.emplace_back(ar, m);
                t0->relation = &s1->read_relations.back();
            }
        }
        {
            auto s = isl::space::from(s1->domain.get_space(), ar->domain.get_space());
            auto m = isl::basic_map::universe(s);
            m.add_constraint(s.out(0) == s.in(0));

            s1->write_relation = {ar, m};
        }
        s1->is_infinite = true;

        m_time_stmts.push_back(s1);
    }
}

void polyhedral_gen::add_time_array(polyhedral::model & output)
{
    output.arrays.push_back(m_time_array);
    for (auto & s : m_time_stmts)
        output.statements.push_back(s);
}

polyhedral::model
polyhedral_gen::process(const unordered_set<id_ptr> & ids)
{
    polyhedral::model model;
    model.context = m_isl_ctx;

    m_time_array_needed = false;

    for (const auto & id : ids)
    {
        if (auto input = dynamic_pointer_cast<functional::input>(id->expr.expr))
        {
            make_input(id, model);
        }
        else
        {
            auto a = make_array(id);
            model.arrays.push_back(a);
        }
    }

    for (const auto & id : ids)
    {
        if (dynamic_pointer_cast<functional::input>(id->expr.expr))
            continue;

        // FIXME: Hack for recursive arrays
        m_current_id = id;
        make_statements(id, model);
    }

    if (m_time_array_needed)
        add_time_array(model);

    return model;
}

void polyhedral_gen::make_input(id_ptr id, polyhedral::model & model)
{
    auto input = dynamic_pointer_cast<functional::input>(id->expr.expr);
    auto type = input->type;

    primitive_type pt;
    if (type->is_array())
        pt = type->array()->element;
    else if (type->is_scalar())
        pt = type->scalar()->primitive;
    else
        throw error("Unexpected type.");

    string array_name = id->name;
    array_size_vec size;
    if (type->is_array())
        size = type->array()->size;

    ph::array_ptr ar = nullptr;

    {
        auto tuple = isl::set_tuple( isl::identifier(array_name),
                                     std::max((int)size.size(), 1) );
        auto space = isl::space(model.context, tuple);
        auto domain = isl::set::universe(space);

        bool is_infinite = false;

        if (size.empty())
        {
            domain.add_constraint(space.var(0) == 0);
        }
        else
        {
            for (int dim = 0; dim < size.size(); ++dim)
            {
                int ub = size[dim];
                auto v = space.var(dim);

                domain.add_constraint(v >= 0);
                if (ub >= 0)
                    domain.add_constraint(v < ub);
                else if (dim == 0)
                    is_infinite = true;
                else
                    throw error("A dimension other dimension than 0 is infinite.");
            }
        }

        ar = make_shared<ph::array>(array_name, domain, pt);
        ar->is_infinite = is_infinite;
        ar->size = size;

        model.arrays.push_back(ar);

        auto result = m_arrays.emplace(id, ar);
        assert(result.second);
    }

    {
        // Make statement domain.

        // If the array is not infinite, the statement has
        // a single iteration that writes the entire array.

        string stmt_name = id->name + ".s";

        auto tuple = isl::set_tuple( isl::identifier(stmt_name), 1 );
        auto space = isl::space(model.context, tuple);
        auto domain = isl::set::universe(space);

        auto i = space.var(0);
        if (size.empty() || !ar->is_infinite)
        {
            domain.add_constraint(i == 0);
        }
        else
        {
            domain.add_constraint( i >= 0 );
        }

        auto stmt = make_shared<polyhedral::statement>(domain);

        stmt->is_infinite = ar->is_infinite;

        // functional call expression

        auto call = make_shared<polyhedral::external_call>();
        call->name = "input_" + id->name;

        vector<expr_ptr> ar_index;
        if (ar->is_infinite)
            // Iterate first dimension
            ar_index.push_back(make_shared<ph::iterator_read>(0));

        auto read = make_shared<ph::array_read>(ar, ar_index);

        call->args.push_back(read);

        // isl relations

        {
            auto s = isl::space::from(stmt->domain.get_space(),
                                      ar->domain.get_space());

            auto m = isl::basic_map::universe(s);
            if (ar->is_infinite)
                m.add_constraint(s.out(0) == s.in(0));

            stmt->write_relation = { ar, m };
            stmt->read_relations.emplace_back(ar, m);

            read->relation = &stmt->read_relations.back();

            if (verbose<polyhedral_gen>::enabled())
            {
                cout << "Input read and write relation:" << endl;
                m_isl_printer.print(m); cout << endl;
            }
        }

        stmt->expr = call;

        model.statements.push_back(stmt);
    }
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

    auto read = make_shared<ph::array_read>(array, ar_index);

    call->args.push_back(read);

    // isl index expression

    {
        auto s = isl::space::from(stmt->domain.get_space(),
                                  array->domain.get_space());

        auto m = isl::basic_map::universe(s);
        m.add_constraint(s.out(0) == s.in(0));

        stmt->read_relations.emplace_back(array, m);

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

    auto arr = make_shared<ph::array>();

    auto tuple = isl::set_tuple( isl::identifier(array_name, arr.get()),
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
            if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
            {
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

    arr->name = array_name;
    arr->domain = domain;
    arr->type = element_type;
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
                    throw source_error("'" + id->name + "': " +
                                       "Array subdomains are not disjoint.",
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
                throw source_error("'" + id->name + "': " +
                                   "Array subdomains do not cover entire array.",
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
        auto m = isl::basic_map::identity
                (stmt->domain.get_space(), array->domain.get_space());
        stmt->write_relation.map = m;
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
        m_isl_printer.print(stmt->write_relation.map); cout << endl;

        for (const auto & rel : stmt->read_relations)
        {
            cout << "Read relation:" << rel.array->name << endl;
            m_isl_printer.print(rel.map); cout << endl;
        }
    }

    return stmt;
}

expr_ptr polyhedral_gen::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto av = dynamic_pointer_cast<array_var>(ref->var))
    {
        int dim = m_space_map->index_of(av);
        auto iter_value = make_shared<ph::iterator_read>(dim, ref->location);

        if (!m_in_array_application && dynamic_pointer_cast<infinity>(av->range.expr))
        {
            m_time_array_needed = true;

            auto time_value = make_shared<ph::array_read>
                    (m_time_array, vector<expr_ptr>({iter_value}));

            auto rel_space = isl::space::from
                    (m_current_stmt->domain.get_space(),
                     m_time_array->domain.get_space());
            auto rel_map = isl::basic_map::universe(rel_space);
            // FIXME: is the input dimension right?
            rel_map.add_constraint(rel_space.out(0) == rel_space.in(dim));
            ph::array_relation rel(m_time_array, rel_map);

            m_current_stmt->read_relations.push_back(rel);
            time_value->relation = &m_current_stmt->read_relations.back();

            return time_value;
        }
        else
        {
            return iter_value;
        }
    }
    else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        auto arr = m_arrays.at(id);
        assert(arr->size.empty());

        vector<expr_ptr> index = { make_shared<int_const>(0) };

        auto read_expr = make_shared<ph::array_read>(arr, index, ref->location);

        {
            auto space = isl::space::from(m_space_map->space, arr->domain.get_space());
            auto map = isl::basic_map::universe(space);
            map.add_constraint(space.out(0) == 0);
            ph::array_relation rel(arr, map);
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

        auto m = isl::basic_map::universe(space);

        assert(app->args.size() == space.dimension(isl::space::output));
        for (int a = 0; a < app->args.size(); ++a)
        {
            isl::expression e { nullptr };
            try {
                e = to_affine_expr(app->args[a], sm_rel);
            } catch (...) {
                continue;
            }

            m.add_constraint(space.out(a) == e);
        }

        m_current_stmt->read_relations.emplace_back(arr, m);

        read_expr->relation = &m_current_stmt->read_relations.back();
    }

    revertable<bool> guard(m_in_array_application, true);

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




}
}
