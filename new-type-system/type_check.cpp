#include "type_check.hpp"
#include "../utility/containers.hpp"

#include <iostream>

using namespace stream::functional;
using namespace std;

namespace arrp {

type_checker::type_checker(built_in_types * builtin):
    m_builtin(builtin)
{}

void type_checker::process(const arrp::scope & scope)
{
    context_type::scope_holder type_scope(m_context);

    for (auto group : scope.groups)
    {
        auto active_group_token = stack_scoped(group, m_active_scope_groups);

        for (auto & id : group->ids)
        {
            m_context.bind(id, shared(new type_var));
        }

        for (auto & id : group->ids)
        {
            cout << "id:" << id->name << endl;

            auto expr_type = visit(id->expr);

            auto binding = m_context.find(id);
            auto id_type = binding.value();
            unify(id_type, expr_type);
        }

        for (auto & id : group->ids)
        {
            auto id_type = m_context.find(id).value();

            make_universal(id_type);

            cout << id->name << " : " << id_type << endl;
        }
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

type_ptr type_checker::visit_primitive(const shared_ptr<primitive> & prim)
{
    return type_ptr();
}

type_ptr type_checker::visit_func(const shared_ptr<stream::functional::function> & func)
{
    cout << "Function: ";
    m_printer.print(func, cout);
    cout << endl;

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
    cout << "Application: ";
    m_printer.print(app, cout);
    cout << endl;

    auto result_t = visit(app->object);

    cout << "Applying: " << *result_t << endl;

    for (auto & arg : app->args)
    {
        auto arg_t = visit(arg);

        auto expected_func_t = m_builtin->function(arg_t, shared(new type_var));

        unify(result_t, expected_func_t);

        result_t = expected_func_t->arguments[1];
    }

    cout << "Result: " << *result_t;

    return result_t;
}

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    type_ptr type;

    {
        auto binding = m_context.find(ref->var);
        if (!binding.value())
            throw stream::error("Unexpected: reference has no bound type.");
        type = binding.value();
    }

    cout << "Instantiating: " << ref->var->name << " : " << type << endl;

    return instance(type);
}

type_ptr type_checker::unify(const type_ptr & a_raw, const type_ptr & b_raw)
{
    cout << "Unifying: " << a_raw << " & " << b_raw << endl;

    auto a = follow(a_raw);
    auto b = follow(b_raw);

    if (auto a_var = dynamic_pointer_cast<type_var>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            auto result_var = shared(new type_var);
            result_var->constraints.insert(a_var->constraints.begin(), a_var->constraints.end());
            result_var->constraints.insert(b_var->constraints.begin(), b_var->constraints.end());
            a_var->constraints.clear();
            b_var->constraints.clear();

            a_var->value = b_var->value = result_var;

            return result_var;
        }
        else if (is_contained(a_var, b))
        {
            ostringstream msg;
            msg << "Variable " << *a << " is contained in type " << *b << ".";
            throw type_error(msg.str());
        }
        else
        {
            a_var->constraints.clear();
            a_var->value = b;
            return b;
        }
    }
    else if (auto a_cons = dynamic_pointer_cast<type_cons>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            // Already handled above
            return unify(b, a);
        }
        else if (auto b_cons = dynamic_pointer_cast<type_cons>(b))
        {
            if (a_cons->kind != b_cons->kind ||
                    a_cons->arguments.size() != b_cons->arguments.size())
            {
                ostringstream msg;
                msg << "Type mismatch: " << *a << " != " << *b;
                throw type_error(msg.str());
            }

            // FIXME: Triggers on simple legal function application.
#if 0
            if (a_cons->kind == m_builtin->function_cons())
            {
                throw type_error("Recursive use of functions.");
            }
#endif

            for (int i = 0; i < (int)a_cons->arguments.size(); ++i)
            {
                auto unified_arg = unify(a_cons->arguments[i], b_cons->arguments[i]);
                a_cons->arguments[i] = unified_arg;
                b_cons->arguments[i] = unified_arg;
            }

            return a_cons;
        }
    }

    throw stream::error("Unexpected kind of type.");
}

type_ptr type_checker::instance(type_ptr type)
{
    auto type_instance = subterm_instance(type);
    m_var_instances.clear();
    return type_instance;
}

type_ptr type_checker::subterm_instance(type_ptr type)
{
    type = follow(type);


    if (auto var = dynamic_pointer_cast<type_var>(type))
    {
        {
            auto instance_pos = m_var_instances.find(var);
            if (instance_pos != m_var_instances.end())
            {
                cout << "Reusing already copied var " << var << endl;
                return instance_pos->second;
            }
        }

        if (var->is_universal)
        {
            cout << "Copying universal var " << var << endl;

            auto instance = shared(new type_var);
            instance->constraints = var->constraints;

            m_var_instances.emplace(var, instance);

            return instance;
        }
        else
        {
            cout << "Reusing non-universal var " << var << endl;
            return var;
        }
    }
    else if (auto cons = dynamic_pointer_cast<type_cons>(type))
    {
        auto new_cons = shared(new type_cons(cons->kind));
        for (auto & arg : cons->arguments)
        {
            new_cons->arguments.push_back(subterm_instance(arg));
        }
        return new_cons;
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

void type_checker::make_universal(type_ptr type)
{
    type = follow(type);

    if (auto var = dynamic_pointer_cast<type_var>(type))
    {
        var->is_universal = true;
    }
    else if (auto cons = dynamic_pointer_cast<type_cons>(type))
    {
        for (auto & arg : cons->arguments)
        {
            make_universal(arg);
        }
    }
    else
    {
        throw stream::error("Unexpected kind of type.");
    }
}

}
