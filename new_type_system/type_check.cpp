#include "type_check.hpp"

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
        unify(id_type, expr_type);
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
    else
    {
        throw stream::error("To do.");
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
    auto actual_func = instance(m_builtin->primitive_op(prim->kind));

    unify(required_func, actual_func);

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

    auto result_t = visit(app->object);

    for (auto & arg : app->args)
    {
        auto arg_t = visit(arg);

        cout << "Applying: " << *result_t << "(" << *arg_t << ")" << endl;

        auto expected_func_t = m_builtin->function(arg_t, type_ptr(new type_var));

        cout << "Expected: " << *expected_func_t << endl;

        unify(result_t, expected_func_t);

        result_t = expected_func_t->arguments[1];
    }

    cout << "Result: " << *result_t << endl;

    return result_t;
}

type_ptr type_checker::recursive_instance(type_ptr type, type_var_map & map, bool universal = false)
{
    type = follow(type);

    if (auto cons = dynamic_pointer_cast<type_cons>(type))
    {
        bool children_universal = universal | (cons->kind == m_builtin->function_cons());

        auto new_cons = shared(new type_cons(cons->kind));
        for (auto & arg : cons->arguments)
        {
            auto instance = recursive_instance(arg, map, children_universal);
            new_cons->arguments.push_back(instance);
        }

        return new_cons;
    }
    else if (auto var = dynamic_pointer_cast<type_var>(type))
    {
        {
            auto instance_pos = map.find(var);
            if (instance_pos != map.end())
            {
                cout << "Reusing already copied var " << var << endl;
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
            cout << "Copying universal var " << var << endl;

            // FIXME: Variables in constraints should be changed to the new variables
            auto instance = shared(new type_var);

            instance->constraints = var->constraints;
            map.emplace(var, instance);

            var = instance;
        }
        else
        {
            cout << "Reusing non-universal var " << var << endl;
        }

        for (auto & constraint : var->constraints)
        {
            for (auto & arg : constraint.second)
            {
                arg = recursive_instance(arg, map, universal);
            }
        }

        return var;
    }
    else
    {
        throw stream::error("Unexpected kind of type.");
    }
}

type_ptr type_checker::instance(const type_ptr & type)
{
    type_var_map map;
    return recursive_instance(type, map, false);
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
