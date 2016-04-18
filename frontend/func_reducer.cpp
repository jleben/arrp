#include "func_reducer.hpp"
#include "error.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace functional {

string wrong_arg_count_msg(int required, int actual)
{
    ostringstream text;
    text << " Wrong number of arguments ("
         << "expected: " << required << ", "
         << "actual: " << actual
         << ")."
            ;
    return text.str();
}

func_reducer::func_reducer(name_provider & nmp):
    m_name_provider(nmp),
    m_copier(m_ids, nmp)
{}

id_ptr func_reducer::reduce(id_ptr id, const vector<expr_ptr> & args)
{
    if (m_ids.find(id) == m_ids.end())
    {
        if (verbose<functional::model>::enabled())
        {
            cout << "Reducing expression of id " << id->name << endl;
        }

        id->expr = reduce(id->expr);
        m_ids.insert(id);
    }

    if (!args.empty())
    {
        if (verbose<functional::model>::enabled())
        {
            cout << "Applying args to id " << id->name << endl;
        }

        auto new_name = m_name_provider.new_name(id->name);
        auto new_expr = m_copier.copy(id->expr);
        auto new_id = make_shared<identifier>(new_name, new_expr, id->location);

        new_id->expr = apply(new_id->expr, args, location_type());

        return new_id;
    }
    else
    {
        return id;
    }
}

expr_ptr func_reducer::apply(expr_ptr expr, const vector<expr_ptr> & args,
                             const location_type & loc)
{
    printer p;

    if (verbose<functional::model>::enabled())
    {
        cout << "Function application at: " << loc << endl;
    }

    m_trace.push(loc);

    expr_ptr reduced_expr;

    auto func = dynamic_pointer_cast<function>(expr);

    if (func)
    {
        if(func->vars.size() < args.size())
            throw reduction_error
                (wrong_arg_count_msg(func->vars.size(), args.size()), loc);

        reduce_context_type::scope_holder scope(m_beta_reduce_context);

        for (int i = 0; i < args.size(); ++i)
        {
            auto & var = func->vars[i];
            auto arg = args[i];

            if (var->ref_count > 1)
            {
                auto arg_id_name = m_name_provider.new_name(var->name);
                auto arg_id = make_shared<identifier>(arg_id_name, arg, location_type());
                arg = make_shared<reference>(arg_id, arg->location);

                m_ids.insert(arg_id);
                if (m_scope_stack.size())
                {
                    m_scope_stack.top()->ids.push_back(arg_id);
                    if (verbose<functional::model>::enabled())
                    {
                        cout << "Stored id for multi-ref argument " << var->name
                             << " into enclosing function scope."
                             << " (" << m_scope_stack.top() << ")"
                             << endl;
                    }
                }
                else
                {
                    if (verbose<functional::model>::enabled())
                    {
                        cout << "No enclosing function scope for id for multi-ref argument "
                             << var->name
                             << endl;
                    }
                }
            }

            if (verbose<functional::model>::enabled())
            {
                cout << "+ bound var: " << var << endl;
            }
            m_beta_reduce_context.bind(var, arg);
        }

        vector<func_var_ptr> remaining_vars;
        if (func->vars.size() > args.size())
        {
            remaining_vars.insert(remaining_vars.end(),
                        func->vars.begin() + args.size(),
                        func->vars.end());
        }

        if (verbose<functional::model>::enabled())
        {
            cout << "Pushing scope of applied function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.push(&func->scope);

        reduce(func->scope);

        reduced_expr = reduce(func->expr);

        if (verbose<functional::model>::enabled())
        {
            cout << "Popping scope of applied function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.pop();

        if (!remaining_vars.empty())
        {
            auto new_func = make_shared<function>(remaining_vars, reduced_expr, loc);
            new_func->scope.ids = func->scope.ids;
            reduced_expr = new_func;
        }
        else
        {
            // add ids to enclosing scope, if any
            if (!m_scope_stack.empty())
            {
                auto & parent_ids = m_scope_stack.top()->ids;
                auto & ids = func->scope.ids;
                parent_ids.insert(parent_ids.end(), ids.begin(), ids.end());
            }
        }
    }
    else
    {
        if (args.size())
            throw reduction_error
                (wrong_arg_count_msg(0, args.size()), loc);

        reduced_expr = reduce(expr);
    };

    m_trace.pop();

    return reduced_expr;
}

expr_ptr func_reducer::reduce(expr_ptr expr)
{
    if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        // FIXME: Can we operate on functions?
        for (auto & operand : op->operands)
            operand = no_function(reduce(operand), operand->location);
        return op;
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        for (auto & a_case : c->cases)
        {
            auto & d = a_case.first;
            auto & e = a_case.second;
            d = no_function(reduce(d), d->location);
            e = no_function(reduce(e), e->location);
        }
        return c;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            if (m_ids.find(id) == m_ids.end())
            {
                if (verbose<functional::model>::enabled())
                {
                    cout << "Reducing id " << id->name
                         << " referenced at " << ref->location << endl;
                }

                m_trace.push(ref->location);

                id->expr = reduce(id->expr);
                m_ids.insert(id);

                m_trace.pop();
            }

            // TODO: remember location of reference
            if (auto func = dynamic_pointer_cast<function>(id->expr))
            {
                return m_copier.copy(func);
            }
            else
                return ref;
        }
        else if (auto f_var = dynamic_pointer_cast<func_var>(ref->var))
        {
            // TODO: remember location of reference
            auto binding = m_beta_reduce_context.find(f_var);
            if (binding)
            {
                if (verbose<functional::model>::enabled())
                {
                    cout << "Substituting bound var: " << f_var << endl;
                }
                // Reduce the substituted value,
                // to count bound variables.
                return reduce(binding.value());
            }
            else
            {
                if (verbose<functional::model>::enabled())
                {
                    cout << "No substitution for bound var: " << f_var << endl;
                }
                return ref;
            }
        }
        else
        {
            return ref;
        }
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        for (auto & var : arr->vars)
        {
            if (var->range)
                var->range = no_function(reduce(var->range), var->range->location);
        }

        m_scope_stack.push(&arr->scope);

        reduce(arr->scope);

        arr->expr = no_function(reduce(arr->expr), arr->expr->location);

        m_scope_stack.pop();

        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = no_function(reduce(app->object), app->object->location);
        for(auto & arg : app->args)
            arg = no_function(reduce(arg), app->object->location);
        return app;
    }
    else if (auto as = dynamic_pointer_cast<array_size>(expr))
    {
        as->object = no_function(reduce(as->object), as->object->location);
        auto & dim = as->dimension;
        if (dim)
            dim = no_function(reduce(dim), dim->location);
        return as;
    }
    else if (auto func = dynamic_pointer_cast<function>(expr))
    {
        printer p;

        if (verbose<functional::model>::enabled())
        {
            cout << "Pushing scope of reduced function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.push(&func->scope);

        reduce(func->scope);

        auto reduced_expr = reduce(func->expr);

        m_scope_stack.pop();

        if (auto func2 = dynamic_pointer_cast<function>(reduced_expr))
        {
            func->vars.insert(func->vars.end(), func2->vars.begin(), func2->vars.end());
            func->scope.ids.insert(func->scope.ids.end(),
                                   func2->scope.ids.begin(), func2->scope.ids.end());
            reduced_expr = func2->expr;
        }

        func->expr = reduced_expr;

        if (verbose<functional::model>::enabled())
        {
            cout << "Popping scope of reduced function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        return func;
    }
    else if (auto app = dynamic_pointer_cast<func_app>(expr))
    {
        auto func = reduce(app->object);

        if (dynamic_pointer_cast<reference>(func))
        {
            if (verbose<functional::model>::enabled())
            {
                cout << "Aborting function application due to unresolved function"
                     << " at: " << app->location << endl;
            }

            return app;
        }

        vector<expr_ptr> reduced_args;
        for (auto & arg : app->args)
            reduced_args.push_back(reduce(arg));

        auto reduced_func = apply(func, reduced_args, app->location);
        return reduced_func;
    }
    else
    {
        return expr;
    }
}

void func_reducer::reduce(scope & s)
{
    // Create a copy, because the set of ids
    // might be modified along the way
    auto ids = s.ids;

    for (auto id : ids)
    {
        if (verbose<functional::model>::enabled())
            cout << "Reducing scope-local id \"" << id->name << "\"" << endl;

        id->expr = reduce(id->expr);
    }
}

expr_ptr func_reducer::no_function(expr_ptr expr)
{
    return no_function(expr, expr->location);
}

expr_ptr func_reducer::no_function(expr_ptr expr, const location_type & loc)
{
    // FIXME: Perform this check for entire tree after all reduction?
#if 0
    if (auto f = dynamic_pointer_cast<function>(expr))
        throw reduction_error("An abstraction is not allowed here.", loc);
#endif
    return expr;
}

}
}
