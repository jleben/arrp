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

polyhedral_gen::polyhedral_gen(const polyhedral_gen::options &options):
    m_options(options),
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
        auto a0 = access(s0, ar, {make_shared<int_const>(0)}, false, true);
        s0->expr = make_shared<ph::assignment>(a0, make_shared<int_const>(0));

        m_time_stmts.push_back(s0);
    }
    {
        auto s1_space = isl::space(m_isl_ctx, isl::set_tuple(isl::identifier(".time.s1"), 1));
        auto s1_dom = isl::set::universe(s1_space);
        s1_dom.add_constraint(s1_space.var(0) > 0);

        auto s1 = make_shared<ph::statement>(s1_dom);

        {
            auto t = make_shared<ph::iterator_read>(0);
            auto t_1 = make_shared<primitive>(primitive_op::subtract, t, make_shared<int_const>(1));
            t_1->type = make_int_type();
            auto at_1 = access(s1, ar, {t_1}, true, false);

            auto v = make_shared<primitive>(primitive_op::add, at_1, make_shared<int_const>(1));
            v->type = make_int_type();

            auto at = access(s1, ar, {make_shared<ph::iterator_read>(0)}, false, true);

            s1->expr = make_shared<ph::assignment>(at, v);
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
        if (auto input = dynamic_pointer_cast<functional::external>(id->expr.expr))
        {
            make_input(id, model, m_options.atomic_io, m_options.ordered_io);
        }
        else
        {
            auto a = make_array(id);
            model.arrays.push_back(a);
        }
    }

    for (const auto & id : ids)
    {
        if (dynamic_pointer_cast<functional::external>(id->expr.expr))
            continue;

        // FIXME: Hack for recursive arrays
        m_current_id = id;
        make_statements(id, model);
    }

    if (m_time_array_needed)
        add_time_array(model);

    return model;
}

void polyhedral_gen::make_input(id_ptr id, polyhedral::model & model, bool atomic, bool ordered)
{
    auto input = dynamic_pointer_cast<functional::external>(id->expr.expr);
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

    string call_name = "input_" + id->name;

    ph::stmt_ptr stmt;

    {
        // Make statement domain.

        // If the array is not infinite, the statement has
        // a single iteration that writes the entire array,
        // unless atomic input is requested.

        string stmt_name = id->name + ".in";

        isl::set domain(nullptr);

        if (atomic)
        {
            domain = ar->domain;
        }
        else
        {
            auto tuple = isl::set_tuple( isl::identifier(stmt_name), 1 );
            auto space = isl::space(model.context, tuple);
            domain = isl::set::universe(space);

            auto i = space.var(0);
            if (size.empty() || !ar->is_infinite)
            {
                domain.add_constraint(i == 0);
            }
            else
            {
                domain.add_constraint( i >= 0 );
            }
        }

        stmt = make_shared<polyhedral::statement>(domain);
        stmt->is_input_or_output = true;
        stmt->is_infinite = ar->is_infinite;

        // functional call expression

        auto call = make_shared<polyhedral::external_call>();
        call->name = call_name;

        vector<expr_ptr> ar_index;
        if (atomic)
        {
            for (int i = 0; i < domain.dimensions(); ++i)
                ar_index.push_back(make_shared<ph::iterator_read>(i));
        }
        else if (ar->is_infinite)
        {
            // Iterate first dimension
            ar_index.push_back(make_shared<ph::iterator_read>(0));
        }

        auto a = access(stmt, ar, ar_index, false, true);
        call->args.push_back(a);

        stmt->expr = call;

        // Add self-relation to impose order
        if (ordered && (atomic || ar->is_infinite))
        {
            auto m = isl::order_less_than(stmt->domain.get_space());

            stmt->self_relations = m;

            if (verbose<polyhedral_gen>::enabled())
            {
                cout << "Input self relation:" << endl;
                m_isl_printer.print(m); cout << endl;
            }
        }

        model.statements.push_back(stmt);
    }

    ph::io_channel ch;
    ch.name = id->name;
    ch.type = type;
    ch.array = ar;
    ch.statement = stmt;

    model.inputs.push_back(ch);
}

void polyhedral_gen::add_output(polyhedral::model & model,
                                id_ptr id,
                                bool atomic, bool ordered)
{
    auto array = m_arrays.at(id);

    string stmt_name = id->name + ".out";

    isl::set domain(nullptr);

    if (atomic)
    {
        domain = array->domain;
        domain.set_name(stmt_name);
    }
    else
    {
        auto tuple = isl::set_tuple( isl::identifier(stmt_name, nullptr), 1 );
        auto space = isl::space(model.context, tuple);
        domain = isl::set::universe(space);

        auto i = space.var(0);
        if (array->size.empty() || !array->is_infinite)
        {
            domain.add_constraint(i == 0);
        }
        else
        {
            domain.add_constraint( i >= 0 );
        }
    }

    auto stmt = make_shared<polyhedral::statement>(domain);
    stmt->is_input_or_output = true;
    stmt->is_infinite = array->is_infinite;

    // functional call expression

    auto call = make_shared<polyhedral::external_call>();
    call->name = "output_" + id->name;

    vector<expr_ptr> ar_index;
    if (atomic)
    {
        for (int i = 0; i < array->domain.dimensions(); ++i)
            ar_index.push_back(make_shared<ph::iterator_read>(i));
    }
    else if (array->is_infinite)
    {
        // Iterate first dimension
        ar_index.push_back(make_shared<ph::iterator_read>(0));
    }

    auto a = access(stmt, array, ar_index, true, false);
    call->args.push_back(a);

    stmt->expr = call;

    if (verbose<polyhedral_gen>::enabled())
    {
        cout << "Output read relation:" << endl;
        m_isl_printer.print(a->map); cout << endl;
    }

    // Add self-relation to impose order
    if (ordered && (atomic || array->is_infinite))
    {
        auto m = isl::order_less_than(stmt->domain.get_space());
        stmt->self_relations = m;

        if (verbose<polyhedral_gen>::enabled())
        {
            cout << "Output self relation:" << endl;
            m_isl_printer.print(m); cout << endl;
        }
    }

    model.statements.push_back(stmt);

    ph::io_channel ch;
    ch.name = id->name;
    ch.type = id->expr->type;
    ch.array = array;
    ch.statement = stmt;

    model.outputs.push_back(ch);
}

ph::array_ptr polyhedral_gen::make_array(id_ptr id)
{
    string array_name = id->name;

    array_size_vec size;

    if (id->expr->type->is_array())
    {
        size = id->expr->type->array()->size;
    }

    auto arr = make_shared<ph::array>();

    auto tuple = isl::set_tuple( isl::identifier(array_name, arr.get()),
                                 std::max((int)size.size(), 1) );

    auto space = isl::space( m_isl_ctx, tuple );
    auto domain = isl::set::universe(space);
    auto constraint_space = isl::local_space(space);
    bool is_infinite = false;

    if (!size.empty())
    {
        for (int dim = 0; dim < (int)size.size(); ++dim)
        {
            int s = size[dim];
            if (s < 0 && dim != 0)
            {
                ostringstream text;
                text << "Dimension " << (dim+1) << " of array " << id->name
                     << " is infinite.";
                throw source_error(text.str(), id->location);
            }

            auto dim_var =
                    isl::expression::variable(constraint_space, isl::space::variable, dim);

            auto lower_bound = dim_var >= 0;
            domain.add_constraint(lower_bound);

            if (s >= 0)
            {
                auto upper_bound = dim_var < s;
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

            auto combined_write_domain = isl::set(array_domain.get_space());
            for (auto & stmt : case_stmts)
            {
                auto write_domain = stmt->domain;
                write_domain.clear_id();
                if (write_domain.dimensions() < array_domain.dimensions())
                {
                    auto extra_dim_count = array_domain.dimensions() - write_domain.dimensions();
                    write_domain.add_dimensions(isl::space::variable, extra_dim_count);
                }

                assert_or_throw(write_domain.dimensions() == array_domain.dimensions());

                if (!write_domain.is_disjoint(combined_write_domain))
                {
                    throw source_error("'" + id->name + "': " +
                                       "Array subdomains are not disjoint.",
                                       case_expr->location);
                }

                combined_write_domain = combined_write_domain | write_domain;
            }

            if (false)
            {
                combined_write_domain.coalesce();
                if (my_verbose_out::enabled())
                {
                    cout << "Combined domains:" << endl;
                    m_isl_printer.print(combined_write_domain); cout << endl;
                }
            }

            if (!(combined_write_domain == array_domain))
            {
                throw source_error("'" + id->name + "': " +
                                   "Array subdomains do not cover entire array.",
                                   case_expr->location);
            }
        }
        else
        {
            auto s = make_stmt(arr->vars, stmt_name, expr_slot(nullptr), arr->expr);
            output.statements.push_back(s);
        }
    }
    else
    {
        auto s = make_stmt({}, stmt_name, expr_slot(nullptr), id->expr);
        output.statements.push_back(s);
    }
}

polyhedral::stmt_ptr polyhedral_gen::make_stmt
(const vector<array_var_ptr> & vars,
 const string & name,
 const expr_slot & subdomain_expr, const expr_slot & expr)
{
    auto array = m_arrays.at(m_current_id);

    ph::stmt_ptr stmt;

    {
        // Create domain

        auto domain = array->domain;

        int n_dim = domain.dimensions();
        int n_var = vars.size();

        // Statement has only as many dimensions as actual array vars

        if (n_var > 0 && n_var < n_dim)
        {
            domain.project_out_dimensions(isl::space::variable, n_var, n_dim - n_var);
        }
        else if (n_var == 0)
        {
            // But make the domain {0} if there is no vars
            auto space = isl::space(m_isl_ctx, isl::set_tuple(1));
            domain = isl::set::universe(space);
            domain.add_constraint(space.var(0) == 0);
        }

        domain.set_name(name);

        stmt = make_shared<ph::statement>(domain);
    }

    auto space = stmt->domain.get_space();
    space_map sm(space, vars);

    m_space_map = &sm;

    if (subdomain_expr)
    {
        auto subdomain = to_affine_set(subdomain_expr, sm);
        if (subdomain.is_empty())
        {
            print(source_error::non_critical,
                  source_error("Effective domain of array part is empty.",
                               expr.location));
        }
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

    m_current_stmt = stmt;
    stmt->expr = visit(expr);

    if (stmt->expr->type)
    {
        assert_or_throw(stmt->expr->type->is_scalar());

        vector<expr_ptr> index;
        if (array->size.size()) // If statement not singular
        {
            for (int dim = 0; dim < stmt->domain.dimensions(); ++dim)
            {
                auto i = make_shared<ph::iterator_read>(dim);
                index.push_back(i);
            }
        }
        auto dest = access(stmt, array, index, false, true);
        stmt->expr = make_shared<ph::assignment>(dest, stmt->expr);
    }

    if (my_verbose_out::enabled())
    {
        cout << "Statement domain "
             << (stmt->is_infinite ? "(unbounded)" : "(bounded)")
             << ":" << endl;
        m_isl_printer.print(stmt->domain); cout << endl;

        for (auto & rel : stmt->array_accesses)
        {
            cout << "Relation (";
            if (rel->reading)
                cout << " read";
            if (rel->writing)
                cout << " write";
            cout << " ):" << endl;
            m_isl_printer.print(rel->map);
            cout << endl;
        }
    }

    return stmt;
}

expr_ptr polyhedral_gen::visit(const expr_ptr &expr)
{
    if (m_nonaffine_array_args.count(expr))
    {
        revertable<bool> guard(m_in_affine_array_arg, false);
        return rewriter_base::visit(expr);
    }
    else
    {
        return rewriter_base::visit(expr);
    }
}

expr_ptr polyhedral_gen::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto av = dynamic_pointer_cast<array_var>(ref->var))
    {
        int dim = m_space_map->index_of(av);
        auto iter_value = make_shared<ph::iterator_read>(dim, ref->location);

        assert(av->range);

        if (!m_in_affine_array_arg && dynamic_pointer_cast<infinity>(av->range.expr))
        {
            if (my_verbose_out::enabled())
            {
                cout << "Statement " << m_current_stmt->name << " uses time as data"
                     << " (variable " << av->name << ")" << endl;
            }

            m_time_array_needed = true;

            auto time_value = access(m_current_stmt, m_time_array, {iter_value}, true, false);
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
        return access(m_current_stmt, arr, {}, true, false);
    }
    else
    {
        throw error("Unexpected reference type.");
    }
}

expr_ptr polyhedral_gen::visit_array_app
(const shared_ptr<array_app> & app)
{
    auto read_expr = make_shared<ph::array_access>();

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
    read_expr->reading = true;
    read_expr->writing = false;

    assert(app->args.size() <= arr->size.size());

    functional::array_size_vec result_size
            (arr->size.begin() + app->args.size(),
             arr->size.end());
    read_expr->type = type_for(result_size, arr->type);

    {
        // In case of non-affine expressions, we over-estimate the set of accessed array indices.
        // This is achieved by representing unknown values as parameters.
        // The values may be bounded using "min" and "max" expressions,
        // and therefore array arguments are piecewise expression.

        // Build ISL expressions for array arguments

        piecewise_affine_expr_builder builder(m_space_map->space, m_space_map->vars);

        vector<isl::piecewise_expression> args;
        for (int a = 0; a < app->args.size(); ++a)
        {
            args.push_back(builder.build(app->args[a]));
            //cout << "Arg expression: ";
            //isl_printer_print_pw_aff(m_isl_printer.get(), args.back().get());
            //cout << endl;
        }

        m_nonaffine_array_args.insert(builder.nnonaffine().begin(), builder.nnonaffine().end());

        for (auto & a : app->args)
        {
            revertable<bool> guard(m_in_affine_array_arg, true);
            read_expr->indexes.push_back(visit(a));
        }

        for (auto & a : args)
            a = isl_pw_aff_align_params(a.copy(), builder.space().copy());

        // Combine the arguments into a multi-expression

        auto range_space = isl::space(m_isl_ctx, isl::set_tuple(args.size()));
        range_space = isl_space_align_params(range_space.copy(), builder.space().copy());

        auto relation_space = isl::space::from(builder.space(), range_space);

        auto * range_expr = isl_multi_pw_aff_zero(relation_space.copy());
        for (int i = 0; i < args.size(); ++i)
            range_expr = isl_multi_pw_aff_set_pw_aff(range_expr, i, args[i].copy());

        // Turn the multi-expression into a map

        isl::map m = isl_map_from_multi_pw_aff(range_expr);

        //cout << "Access map with params: ";
        //m_isl_printer.print(m);
        //cout << endl;

        // Project out parameters

        int nparam = m.get_space().dimension(isl::space::parameter);
        m.project_out_dimensions(isl::space::parameter, 0, nparam);
        m.coalesce();

        // Add remaining array dimensions without arguments

        int n_extra_dim = arr->domain.dimensions() - args.size();
        if (n_extra_dim > 0)
            m.add_dimensions(isl::space::output, n_extra_dim);

        // Set proper array id

        m.set_id(isl::space::output, arr->domain.id());

        //cout << "Access map without params: ";
        //m_isl_printer.print(m);
        //cout << endl;

        read_expr->map = m;
    }

    m_current_stmt->array_accesses.push_back(read_expr);

    return read_expr;
}

expr_ptr polyhedral_gen::visit_func_app(const shared_ptr<func_app> &app)
{
    auto ext = dynamic_pointer_cast<external>(app->object.expr);
    if (!ext)
        throw error("Unexpected object of function application.");

    for (auto & arg : app->args)
        arg = visit(arg);

    auto call = make_shared<ph::external_call>();
    call->name = ext->name;

    for (auto & arg : app->args)
        call->args.push_back(arg);

    if (app->type->is_array())
    {
        // Add pointer to write destination as arg

        // FIXME: Prevent aliasing between input and output by
        // adding conflicts to polyhedral::model::parallel_accesses.

        auto ar = m_arrays.at(m_current_id);
        assert(ar);

        // Statement might be singular, so we only use
        // non-singular dimensions.
        int write_dim_count = app->type->array()->size.size();
        assert(write_dim_count <= ar->size.size());
        int max_stmt_dim = std::min((int) m_current_stmt->domain.dimensions(),
                                    (int) ar->size.size() - write_dim_count);

        vector<expr_ptr> index;
        for (int dim = 0; dim < max_stmt_dim; ++dim)
            index.push_back(make_shared<ph::iterator_read>(dim));

        auto dest = access(m_current_stmt, ar, index, false, true);

        call->args.push_back(dest);

        // No result value.
        call->type = nullptr;
    }
    else
    {
        assert(app->type->is_scalar());
        call->type = app->type;
    }

    return call;
}

isl::set polyhedral_gen::to_affine_set(expr_ptr e, const space_map & s)
{
    if (auto b = dynamic_pointer_cast<bool_const>(e))
    {
        if (b->value)
        {
            return isl::set::universe(s.space);
        }
        else
        {
            return isl::set(s.space);
        }
    }
    else if (auto op = dynamic_pointer_cast<primitive>(e))
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

isl::expression polyhedral_gen::to_affine_expr(expr_ptr e, const space_map & sm)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        return sm.space.val(c->value);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto avar = dynamic_pointer_cast<array_var>(ref->var);
        if (!avar)
            throw source_error("Invalid type of variable in affine expression.",
                               ref->location);
        int dim = sm.index_of(avar);
        if (dim < 0)
            throw error("Free variable in affine expression.");

        return sm.space.var(dim);
    }
    else if (auto iter = dynamic_pointer_cast<ph::iterator_read>(e))
    {
        int dim = iter->index;
        return sm.space.var(dim);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        vector<isl::expression> args;
        args.reserve(op->operands.size());
        for (auto & arg : op->operands)
            args.push_back(to_affine_expr(arg, sm));

        switch(op->kind)
        {
        case primitive_op::add:
            return (args[0] + args[1]);
        case primitive_op::subtract:
            return (args[0] - args[1]);
        case primitive_op::negate:
            return (-args[0]);
        case primitive_op::multiply:
        {
            auto lhs = args[0];
            auto rhs = args[1];
            if (lhs.is_constant())
                return lhs.constant() * rhs;
            else if (rhs.is_constant())
                return lhs * rhs.constant();
            break;
        }
        case primitive_op::divide_integer:
        {
            auto lhs = args[0];
            auto rhs = args[1];
            if (rhs.is_constant())
                return isl::floor(lhs / rhs.constant());
            break;
        }
        case primitive_op::modulo:
        {
            auto lhs = args[0];
            auto rhs = args[1];
            if (rhs.is_constant())
                return lhs % rhs.constant();
            break;
        }
        default:
            break;
        }
    }

    throw error("Not an integer affine expression.");
}

isl::piecewise_expression piecewise_affine_expr_builder::build(expr_ptr e)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        auto e = m_space.val(c->value);
        return isl_pw_aff_from_aff(e.copy());
    }
    else if (auto ref = dynamic_pointer_cast<reference>(e))
    {
        auto avar = dynamic_pointer_cast<array_var>(ref->var);
        if (!avar)
            throw source_error("Invalid type of variable in affine expression.",
                               ref->location);
        int dim = index_of(avar);
        if (dim < 0)
            throw error("Free variable in affine expression.");

        auto e = m_space.var(dim);
        return isl_pw_aff_from_aff(e.copy());
    }
    else if (auto iter = dynamic_pointer_cast<ph::iterator_read>(e))
    {
        int dim = iter->index;
        auto e = m_space.var(dim);
        return isl_pw_aff_from_aff(e.copy());
    }
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        vector<isl::piecewise_expression> args;

        auto build_args = [&]()
        {
            args.reserve(op->operands.size());
            for (auto & arg : op->operands)
                args.push_back(build(arg));

            if (args.size() > 1)
            {
                // Expand common space to contain params of all args
                for (auto & arg : args)
                    m_space = isl::align_params(m_space, isl_pw_aff_get_space(arg.get()));

                // Expand params in each arg to match the common space
                for (auto & arg : args)
                    arg = isl_pw_aff_align_params(arg.copy(), m_space.copy());
            }
        };

        switch(op->kind)
        {
        case primitive_op::add:
        {
            build_args();
            return isl_pw_aff_add(args[0].copy(), args[1].copy());
        }
        case primitive_op::subtract:
        {
            build_args();
            return isl_pw_aff_sub(args[0].copy(), args[1].copy());
        }
        case primitive_op::negate:
        {
            build_args();
            return isl_pw_aff_neg(args[0].copy());
        }
        case primitive_op::multiply:
        {
            build_args();
            auto & lhs = args[0];
            auto & rhs = args[1];
            if (isl_pw_aff_is_cst(lhs.get()) || isl_pw_aff_is_cst(rhs.get()))
                return isl_pw_aff_mul(lhs.copy(), rhs.copy());
            break;
        }
        case primitive_op::divide_integer:
        {
            build_args();
            auto & lhs = args[0];
            auto & rhs = args[1];
            if (isl_pw_aff_is_cst(rhs.get()))
                return isl_pw_aff_floor(isl_pw_aff_div(lhs.copy(), rhs.copy()));
            break;
        }
        case primitive_op::modulo:
        {
            build_args();
            auto lhs = args[0];
            auto rhs = args[1].plain_continuous();
            if (rhs.is_valid() && rhs.is_constant())
                return isl_pw_aff_mod_val(lhs.copy(), rhs.constant().copy());
            break;
        }
        case primitive_op::max:
        {
            build_args();
            return isl_pw_aff_max(args[0].copy(), args[1].copy());
        }
        case primitive_op::min:
        {
            build_args();
            return isl_pw_aff_min(args[0].copy(), args[1].copy());
        }
        default:
            break;
        }
    }

    // This expression is not affine. Represent it with a new parameter.

    m_nonaffine.insert(e);

    //cout << "Creating a new parameter. Expression type = " << typeid(*e).name() << endl;

    int i = m_space.dimension(isl::space::parameter);
    m_space.add_dimensions(isl::space::parameter, 1);
    m_space.set_name(isl::space::parameter, i, string("p") + to_string(i));
    auto p = m_space.param(i);
    return isl_pw_aff_from_aff(p.copy());
}

// NOTE: This only works for custom-made access,
// with indexes already in polyhedral model form.
shared_ptr<polyhedral::array_access> polyhedral_gen::access
(const polyhedral::stmt_ptr & stmt,
 const polyhedral::array_ptr & array,
 const vector<expr_ptr> indexes,
 bool reading, bool writing)
{
    assert(indexes.size() <= array->domain.dimensions());

    auto access = make_shared<polyhedral::array_access>();
    access->array = array;
    access->indexes = indexes;
    access->reading = reading;
    access->writing = writing;

    int used_index_count = min(indexes.size(), array->size.size());
    functional::array_size_vec result_size
            (array->size.begin() + used_index_count,
             array->size.end());
    access->type = type_for(result_size, array->type);

    auto space = isl::space::from(stmt->domain.get_space(), array->domain.get_space()).wrapped();
    auto m = isl::basic_set::universe(space);

    {
        int num_in = stmt->domain.dimensions();
        space_map sm(space, {});
        for (int i = 0; i < indexes.size(); ++i)
        {
            auto e = to_affine_expr(indexes[i], sm);
            auto c = space.var(num_in + i) == e;
            m.add_constraint(c);
        }
    }

    access->map = m.unwrapped();

    stmt->array_accesses.push_back(access);

    return access;
}


}
}
