#include "func_reducer.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace functional {

static bool is_constant(expr_ptr expr)
{
    if (dynamic_cast<constant<int>*>(expr.get()))
        return true;
    if (dynamic_cast<constant<double>*>(expr.get()))
        return true;
    if (dynamic_cast<constant<bool>*>(expr.get()))
        return true;
    return false;
}

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

        m_type_checker.process(id->expr);

        m_ids.insert(id);
    }

    if (!args.empty())
    {
        if (verbose<functional::model>::enabled())
        {
            cout << "Applying args to id " << id->name << endl;
        }

        auto func = dynamic_pointer_cast<function>(id->expr.expr);
        if (!func)
        {
            ostringstream msg;
            msg << "Id " << id->name << " is not a function. "
                << " It can not be applied to arguments.";
            throw source_error(msg.str(), id->location);
        }

        auto new_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_name, id->location);
        new_id->expr.location = id->expr.location;

        auto func_copy =
                dynamic_pointer_cast<function>(m_copier.copy(id->expr).expr);

        new_id->expr = apply(func_copy, args, location_type());

        return new_id;
    }
    else
    {
        return id;
    }
}

expr_ptr func_reducer::apply(shared_ptr<function> func,
                             const vector<expr_ptr> & args,
                             const location_type & loc)
{
    printer p;

    if (verbose<functional::model>::enabled())
    {
        cout << "Function application at: " << loc << endl;
    }

    m_trace.push(loc);

    expr_ptr reduced_expr;

    if(func->vars.size() < args.size())
        throw reduction_error
            (wrong_arg_count_msg(func->vars.size(), args.size()), loc);

    reduce_context_type::scope_holder scope(m_beta_reduce_context);

    // Prepare arguments

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

    // Reduce

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


    if (func->vars.size() > args.size())
    {
        // Generate partially applied function

        vector<func_var_ptr> remaining_vars
                (func->vars.begin() + args.size(),
                 func->vars.end());

        auto new_func = make_shared<function>(remaining_vars, reduced_expr, loc);
        new_func->scope.ids = func->scope.ids;
        reduced_expr = new_func;
    }
    else
    {
        m_type_checker.process(reduced_expr);

        // Add ids to enclosing scope, if any.

        if (!m_scope_stack.empty())
        {
            auto & parent_ids = m_scope_stack.top()->ids;
            auto & ids = func->scope.ids;
            parent_ids.insert(parent_ids.end(), ids.begin(), ids.end());
        }
    }

    m_trace.pop();

    return reduced_expr;
}

expr_ptr func_reducer::reduce(expr_ptr expr)
{
    if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        for (auto & operand : op->operands)
            operand = reduce(operand), operand->location;
        return reduce_primitive(op);
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        for (auto & a_case : c->cases)
        {
            auto & d = a_case.first;
            auto & e = a_case.second;
            d = reduce(d), d->location;
            e = reduce(e), e->location;
        }
        return c;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            if (auto func = dynamic_pointer_cast<function>(id->expr.expr))
            {
                return m_copier.copy(func);
            }

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

            if (is_constant(id->expr))
                return id->expr;

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
                var->range = reduce(var->range);
        }

        m_scope_stack.push(&arr->scope);

        reduce(arr->scope);

        arr->expr = reduce(arr->expr);

        m_scope_stack.pop();

        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = reduce(app->object);
        for(auto & arg : app->args)
            arg = reduce(arg);
        return app;
    }
    else if (auto as = dynamic_pointer_cast<array_size>(expr))
    {
        as->object = reduce(as->object);
        if (as->dimension)
            as->dimension = reduce(as->dimension);

        m_type_checker.process(as);

        int dim = 0;
        if (as->dimension)
        {
            auto dim_const = dynamic_pointer_cast<constant<int>>(as->dimension.expr);
            assert(dim_const);
            dim = dim_const->value - 1;
        }

        auto arr_type = dynamic_pointer_cast<array_type>(as->object->type);
        assert(arr_type);
        assert(dim >= 0 && dim < arr_type->size.size());

        return make_shared<constant<int>>(arr_type->size[dim]);
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
        auto object = reduce(app->object);

        if (dynamic_pointer_cast<reference>(object))
        {
            if (verbose<functional::model>::enabled())
            {
                cout << "Aborting function application due to unresolved function"
                     << " at: " << app->location << endl;
            }

            return app;
        }

        auto func = dynamic_pointer_cast<function>(object);
        if (!func)
        {
            throw source_error("Not a function.", app->object.location);
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

}
}
