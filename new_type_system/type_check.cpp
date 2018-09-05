#include "type_check.hpp"
#include "../utility/printing.hpp"
#include "../frontend/linear_expr_gen.hpp"

#include <iostream>

using namespace std;

namespace arrp {

/*
Type Checking

We require IDs to be topologically sorted so that identifiers with polymorphic function types
are only instantiated after all they are fully defined.
If we allowed recursive functions, then we would need to prevent instantiating function types
in all identifiers involved in a recursion, to be unambiguous.
However, we don't support recursive functions anyway, so we assume no identifier involved in
a recursion has a function type.
*/

type_checker::type_checker(built_in_types * builtin):
    m_builtin(builtin)
{
}

void type_checker::process(const arrp::scope & scope)
{
    context_type::scope_holder type_scope(m_context);
    do_process_scope(scope);
}

void type_checker::do_process_scope(const arrp::scope & scope)
{
    for (auto & id : scope.ids)
    {
        m_context.bind(id, shared(new type_var));
    }

    for (auto & id : scope.ids)
    {
        cout << "ID: " << id->name << endl;

        auto expr_type = visit(id->expr);

        auto binding = m_context.find(id);
        auto id_type = binding.value();
        unify_and_satisfy_constraints(id_type, expr_type);
    }
}

type_ptr type_checker::visit(const expr_ptr & expr)
{
    auto type = visitor::visit(expr);
    expr->new_type = type;
    return type;
}

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto fvar = dynamic_pointer_cast<func_var>(ref->var))
    {
        cout << "Referenced function var: " << fvar->qualified_name << endl;

        auto binding = m_context.find(fvar);
        if (!binding)
            throw stream::error("Unexpected: referenced function variable has no bound type.");
        return binding.value();
    }
    else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        cout << "Referenced ID: " << id->name << endl;

        auto binding = m_context.find(id);
        if (!binding)
            throw stream::error("Unexpected: referenced id has no bound type.");
        return instance(binding.value());
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        auto binding = m_context.find(avar);
        if (!binding)
            throw stream::error("Unexpected: referenced array variable has no bound type.");
        return binding.value();
    }
    else
    {
        throw stream::error("Unexpected reference type.");
    }
}

type_ptr type_checker::visit_bool(const shared_ptr<bool_const> &)
{
    return m_builtin->boolean();
}

type_ptr type_checker::visit_int(const shared_ptr<int_const> & i)
{
    return m_builtin->integer32();
}

type_ptr type_checker::visit_real(const shared_ptr<real_const> &)
{
    return m_builtin->real64();
}

type_ptr type_checker::visit_complex(const shared_ptr<complex_const> &)
{
    return m_builtin->complex64();
}

type_ptr type_checker::visit_infinity(const shared_ptr<infinity> &)
{
    return m_builtin->infinity();
}

type_ptr type_checker::visit_primitive(const shared_ptr<primitive> &prim)
{
    using stream::primitive_op;

    vector<type_ptr> operand_types;
    for (const auto & op : prim->operands)
    {
        auto t = visit(op);
        operand_types.push_back(t);
    }

    auto result = shared(new type_var);
    auto required_func = m_builtin->function(operand_types, result);

    cout << "Instantiating primitive op: " << prim->kind << endl;
    auto actual_func = instance(m_builtin->primitive_op(prim->kind));

    cout << "Unifying primitive op: " << prim->kind << endl;
    unify_and_satisfy_constraints(required_func, actual_func);

    return collapse(result);

#if 0
    type_ptr u(new type_var);
    for (auto & op : prim->operands)
    {
        auto op_type = visit(op);
        u = unify(u, op_type);
    }
    return u;
#endif
}

type_ptr type_checker::visit_func(const shared_ptr<stream::functional::function> & func)
{
    vector<type_ptr> param_types;

    stream::stacker<type_ptr, bound_type_stack> bound_types(m_bound_types);
    context_type::scope_holder func_scope(m_context);

    for (auto & var : func->vars)
    {
        type_ptr type(new type_var);
        param_types.push_back(type);
        bound_types.push(type);
        m_context.bind(var, type);
    }

    do_process_scope(func->scope);

    auto result_type = visit(func->expr);

    type_ptr func_type;

    type_cons_ptr nested_func;

    for (auto & param_type : param_types)
    {
        auto new_func = m_builtin->function(param_type, nullptr);
        if (!func_type)
            func_type = new_func;
        if (nested_func)
            nested_func->arguments[1] = new_func;
        nested_func = new_func;
    }

    assert(func_type);
    assert(nested_func);

    nested_func->arguments[1] = result_type;

    return collapse(func_type);
}

type_ptr type_checker::visit_func_app(const shared_ptr<func_app> &app)
{
    cout << "Application: ";
    m_printer.print(app, cout);
    cout << endl;

    vector<type_ptr> args;
    for (auto & arg : app->args)
    {
        args.push_back(visit(arg));
    }

    auto f = visit(app->object);

    cout << "Applying: " << f << "(" << printable(args, ", ") << ")" << endl;

    auto result = shared(new type_var);
    auto f_expected = m_builtin->function(args, result);

    unify_and_satisfy_constraints(f_expected, f);

    cout << "Application result: " << result << endl;

    return result;
}

type_ptr type_checker::visit_array(const shared_ptr<stream::functional::array> & arr)
{
    context_type::scope_holder type_scope(m_context);

    vector<type_ptr> sizes;

    for (auto & var : arr->vars)
    {
        type_ptr size;
        if (var->range)
        {
            // FIXME: Should be a constant, so it should be reduced immediately.
            size = visit(var->range);
            if (size != m_builtin->integer32() && size != m_builtin->infinity())
                throw type_error("Array size is not an integer or infinity.");
        }
        else
        {
            size = m_builtin->infinity();
        }

        sizes.push_back(size);

        m_context.bind(var, size);
    }

    type_ptr value = visit(arr->expr);
    auto value_constraint = add_constraint(m_builtin->indexable(), { value, type_ptr(new type_var) });

    auto t = m_builtin->array(sizes, value);
    cout << "Array: " << *t << endl;

    satisfy({ value_constraint });

    return t;
}

type_ptr type_checker::visit_array_patterns(const shared_ptr<array_patterns> & ap)
{
    type_ptr t = shared(new type_var);

    for(const auto & pattern : ap->patterns)
    {
        if (pattern.domains)
        {
            auto d = visit(pattern.domains);
            t = unify_and_satisfy_constraints(t, d);
        }
        auto e = visit(pattern.expr);
        t = unify_and_satisfy_constraints(t, e);
    }

    return t;
}

type_ptr type_checker::visit_cases(const shared_ptr<case_expr> & cexpr)
{
    type_ptr t = shared(new type_var);

    for(auto & c : cexpr->cases)
    {
        auto & domain = c.first;
        auto & expr = c.second;

        visit(domain);
        stream::functional::ensure_affine_integer_constraint(domain);

        auto e = visit(expr);
        t = unify_and_satisfy_constraints(t, e);
    }

    return t;
}

type_ptr type_checker::visit_array_app(const shared_ptr<stream::functional::array_app> & app)
{
    //return shared(new type_var);

    vector<type_ptr> args;
    for (auto & arg : app->args)
    {
        args.push_back(visit(arg));
    }

    auto a = visit(app->object);
    auto e = shared(new type_var);

    cout << "Array application: " << a << "[" << printable(args, ", ") << "]" << endl;

    auto c = add_constraint(m_builtin->indexable(), { a, e });

    satisfy({ c });

    cout << "Array application result: " << with_constraints(e) << endl;

    return e;
}

type_ptr type_checker::recursive_instance
(type_ptr type, type_var_map & vmap, type_constr_map & cmap, bool universal = false)
{
    type = follow(type);

    if (auto cons = dynamic_pointer_cast<type_cons>(type))
    {
        bool children_universal = universal | (cons->kind == m_builtin->function_cons());

        auto new_cons = shared(new type_cons(cons->kind));
        for (auto & arg : cons->arguments)
        {
            auto instance = recursive_instance(arg, vmap, cmap, children_universal);
            new_cons->arguments.push_back(instance);
        }

        return new_cons;
    }
    else if (auto var = dynamic_pointer_cast<type_var>(type))
    {
        {
            auto instance_pos = vmap.find(var);
            if (instance_pos != vmap.end())
            {
                cout << "Reusing already copied var " << instance_pos->second << endl;
                return instance_pos->second;
            }
        }

        bool can_copy = universal;

        for (auto & type : m_bound_types)
        {
            type = collapse(type);

            if (is_contained(var, type))
                can_copy = false;
        }

        if (can_copy)
        {
            auto instance = shared(new type_var);
            vmap.emplace(var, instance);

            cout << "Copied universal var " << var << " to " << instance << endl;

            for (const auto & c : var->constraints)
            {
                type_constraint_ptr c2;
                if (cmap.find(c) != cmap.end())
                {
                    cout << "Reusing copied constraint: " << *c << endl;
                    c2 = cmap[c];
                }
                else
                {
                    cout << "Copying constraint: " << *c << "..." << endl;
                    c2 = shared(new type_constraint { *c });
                    cmap.emplace(c, c2);
                }

                instance->constraints.insert(c2);

                // Substitute variables in new constraint
                for (auto & param : c2->params)
                {
                    param = recursive_instance(param, vmap, cmap, universal);
                }
            }

            cout << "New var: " << with_constraints(instance) << endl;

            return instance;
        }
        else
        {
            cout << "Reusing non-universal var " << var << endl;
            return var;
        }
    }
    else
    {
        throw stream::error("Unexpected kind of type.");
    }
}

type_ptr type_checker::instance(const type_ptr & type)
{
    type_var_map vmap;
    type_constr_map cmap;
    return recursive_instance(type, vmap, cmap, false);
}

void type_printer::print(scope & s)
{
    for (const auto & id : s.ids)
    {
        cout << indent() << id->name << " : " << with_constraints(id->expr->new_type) << endl;
        ++m_indent;
        visit(id->expr);
        --m_indent;
    }
}

void type_printer::visit_func(const shared_ptr<stream::functional::function> & func)
{
    print(func->scope);
    visit(func->expr);
}
void type_printer::visit_array(const shared_ptr<stream::functional::array> & arr)
{
    print(arr->scope);
    visit(arr->expr);
}

void type_printer::print(const type_ptr & t)
{
    if (auto var = dynamic_pointer_cast<type_var>(t))
    {
        int & i = m_var_names[var];
        if (i == 0)
            i = m_var_names.size();
        cout << "v" << i << endl;
    }
}

}
