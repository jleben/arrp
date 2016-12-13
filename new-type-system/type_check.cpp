#include "type_check.hpp"

#include <iostream>

using namespace stream::functional;
using namespace std;

namespace arrp {

type_checker::type_checker(built_in_types * builtin):
    m_builtin(builtin)
{}

void type_checker::process(const unordered_set<id_ptr> & ids)
{
    context_type::scope_holder scope(m_context);

    for (auto & id : ids)
    {
        m_context.bind(id, shared(new type_var));
    }

    for (auto & id : ids)
    {
        auto id_type = m_context.find(id).value();
        auto expr_type = visit(id->expr);
        unify(id_type, expr_type);
    }

    for (auto & id : ids)
    {
        auto id_type = m_context.find(id).value();
        cout << id->name << " : " << id_type << endl;
    }
}

type_ptr type_checker::visit_int(const shared_ptr<int_const> & i)
{
    return m_builtin->integer32();
}

type_ptr type_checker::visit_real(const shared_ptr<real_const> &)
{
    return m_builtin->real64();
}

type_ptr type_checker::visit_func(const shared_ptr<stream::functional::function> & func)
{
    context_type::scope_holder func_scope(m_context);

    vector<type_ptr> param_types;

    for (auto & var : func->vars)
    {
        auto param_type = shared(new type_var);
        m_context.bind(var, param_type);
        param_types.push_back(param_type);
    }

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

    return func_type;
}

type_ptr type_checker::visit_func_app(const shared_ptr<func_app> &app)
{
    auto result_t = visit(app->object);

    for (auto & arg : app->args)
    {
        auto arg_t = visit(arg);

        auto expected_func_t = m_builtin->function(arg_t, shared(new type_var));

        unify(result_t, expected_func_t);

        result_t = expected_func_t->arguments[1];
    }

    return result_t;
}

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    auto binding = m_context.find(ref->var);
    if (!binding)
        throw stream::error("Unexpected: reference has no bound type.");

    auto type = binding.value();

    cout << "Getting instance of: " << ref->var->name << " : " << type << endl;

    return instance(type);
}

type_ptr type_checker::instance(type_ptr t)
{
    if (auto var = dynamic_pointer_cast<type_var>(t))
    {
        if (is_free(var))
        {
            cout << "Var " << var << " is free." << endl;
            auto new_var = new type_var;
            new_var->constraints = var->constraints;
            return shared(new_var);
        }
        else
        {
            cout << "Var " << var << " is not free." << endl;
            return var;
        }
    }
    else if (auto cons = dynamic_pointer_cast<type_cons>(t))
    {
        auto new_cons = new type_cons(cons->kind);
        for (auto & arg : cons->arguments)
        {
            new_cons->arguments.push_back(instance(arg));
        }
        return shared(new_cons);
    }
    else
    {
        throw stream::error("Unexpected kind of type.");
    }
}

bool type_checker::is_free(type_var_ptr var)
{
    auto scope = m_context.current_scope();

    while(scope)
    {
        for (auto & binding : *scope)
        {
            auto value = follow(binding.second);
            if (value == var)
                return false;
        }

        scope = scope.parent();
    }

    return true;
}

}
