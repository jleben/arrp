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

        auto new_name = new_id_name(id->name);
        auto new_expr = copy(id->expr);
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
    if (verbose<functional::model>::enabled())
    {
        cout << "Function application at: " << loc << endl;
    }

    m_trace.push(loc);

    m_bound_var_count.push(0);

    vector<func_var_ptr> remaining_vars;

    reduce_context_type::scope_holder scope(m_beta_reduce_context);

    if (args.size())
    {
        auto func = dynamic_pointer_cast<function>(expr);
        if (!func)
            throw reduction_error
                (wrong_arg_count_msg(0, args.size()), loc);
        else if(func->vars.size() < args.size())
            throw reduction_error
                (wrong_arg_count_msg(func->vars.size(), args.size()), loc);

        for (int i = 0; i < args.size(); ++i)
        {
            m_beta_reduce_context.bind(func->vars[i], args[i]);
        }

        if (func->vars.size() > args.size())
        {
            remaining_vars.insert(remaining_vars.end(),
                        func->vars.begin() + args.size(),
                        func->vars.end());
        }

        expr = func->expr;
    }

    auto reduced_expr = reduce(expr);

    m_bound_var_count.pop();

    m_trace.pop();

    if (remaining_vars.size())
        return make_shared<function>(remaining_vars, reduced_expr, loc);
    else
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
                return id->expr;
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
                assert(!m_bound_var_count.empty());
                ++m_bound_var_count.top();
                return ref;
            }
        }
        // FIXME: count array var references as bounds vars too,
        // Get rid of such scopes later (in array reduction?).
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
        arr->expr = no_function(reduce(arr->expr), arr->expr->location);
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
        m_bound_var_count.push(0);
        auto reduced_expr = reduce(func->expr);
        m_bound_var_count.pop();

        if (auto func2 = dynamic_pointer_cast<function>(reduced_expr))
        {
            func->vars.insert(func->vars.end(), func2->vars.begin(), func2->vars.end());
            reduced_expr = func2->expr;
        }

        func->expr = reduced_expr;
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

        auto func_copy = copy(func);
        auto reduced_func = apply(func_copy, reduced_args, app->location);
        return reduced_func;
    }
    else if (auto scope = dynamic_pointer_cast<expr_scope>(expr))
    {
        vector<id_ptr> remaining_ids;

        for (auto id : scope->ids)
        {
            m_bound_var_count.push(0);

            if (verbose<functional::model>::enabled())
            {
                cout << "Reducing scope-local id \"" << id->name << "\"" << endl;
            }

            id->expr = reduce(id->expr);

            if (verbose<functional::model>::enabled())
            {
                cout << "Number of bound vars = " << m_bound_var_count.top() << endl;
            }

            if (m_bound_var_count.top() > 0)
                remaining_ids.push_back(id);

            m_bound_var_count.pop();
        }

        auto reduced_expr = reduce(scope->expr);

        if (remaining_ids.empty())
        {
            return reduced_expr;
        }
        else
        {
            scope->ids = remaining_ids;
            scope->expr = reduced_expr;
            return scope;
        }
    }
    else
    {
        return expr;
    }
}

expr_ptr func_reducer::copy(expr_ptr expr)
{
    if (auto i = dynamic_pointer_cast<constant<int>>(expr))
    {
        return make_shared<constant<int>>(*i);
    }
    else if (auto d = dynamic_pointer_cast<constant<double>>(expr))
    {
        return make_shared<constant<double>>(*d);
    }
    else if (auto b = dynamic_pointer_cast<constant<bool>>(expr))
    {
        return make_shared<constant<bool>>(*b);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        auto new_op = make_shared<primitive>();
        new_op->location = op->location;
        new_op->type = op->type;
        new_op->kind = op->kind;
        for (auto & operand : op->operands)
            new_op->operands.push_back(copy(operand));
        return new_op;
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        auto result = make_shared<case_expr>();
        result->location = c->location;
        for (auto & a_case : c->cases)
        {
            result->cases.emplace_back
                    (copy(a_case.first), copy(a_case.second));
        }
        return result;
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        auto new_arr = make_shared<array>();
        new_arr->location = arr->location;

        stacker<array_ptr> stackit(new_arr, m_array_copy_stack);

        copy_context_type::scope_holder scope(m_copy_context);

        for (auto & var : arr->vars)
        {
            expr_ptr new_range;
            if (var->range)
                new_range = copy(var->range);
            auto new_var = make_shared<array_var>(var->name, new_range, var->location);
            new_arr->vars.push_back(new_var);
            m_copy_context.bind(var, new_var);
        }

        new_arr->expr = copy(arr->expr);

        return new_arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        auto new_app = make_shared<array_app>();
        new_app->location = app->location;
        new_app->object = copy(app->object);
        for (auto & arg : app->args)
            new_app->args.push_back(copy(arg));
        return new_app;
    }
    else if (auto as = dynamic_pointer_cast<array_size>(expr))
    {
        auto new_as = make_shared<array_size>();
        new_as->location = as->location;
        new_as->object = copy(as->object);
        auto &dim = as->dimension;
        if (dim)
            new_as->dimension = copy(dim);
        return new_as;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto a_var = dynamic_pointer_cast<array_var>(ref->var))
        {
            auto binding = m_copy_context.find(a_var);
            assert(binding);
            return make_shared<reference>(binding.value(), ref->location);
        }
        else if (auto f_var = dynamic_pointer_cast<func_var>(ref->var))
        {
            auto binding = m_copy_context.find(f_var);
            if (binding)
                return make_shared<reference>(binding.value(), ref->location);
            else
                return make_shared<reference>(*ref);
        }
        else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            auto binding = m_copy_context.find(id);
            if (binding)
            {
                auto bound_id = dynamic_pointer_cast<identifier>(binding.value());
                assert(bound_id);
                m_ids.insert(bound_id);
                return make_shared<reference>(bound_id, ref->location);
            }
            else
                return make_shared<reference>(*ref);
        }
        else
        {
            throw error("Unexpected reference type.");
        }
    }
    else if (auto r = dynamic_pointer_cast<array_self_ref>(expr))
    {
        assert(!m_array_copy_stack.empty());
        auto arr = m_array_copy_stack.top();
        return make_shared<array_self_ref>(arr, r->location);
    }
    else if (auto func = dynamic_pointer_cast<function>(expr))
    {
        auto new_func = make_shared<function>();
        new_func->location = func->location;

        copy_context_type::scope_holder scope(m_copy_context);

        for (auto & var : func->vars)
        {
            auto new_var = make_shared<func_var>(*var);
            new_func->vars.push_back(new_var);
            m_copy_context.bind(var, new_var);
        }

        new_func->expr = copy(func->expr);

        return new_func;
    }
    else if (auto app = dynamic_pointer_cast<func_app>(expr))
    {
        auto new_app = make_shared<func_app>();
        new_app->location = app->location;
        new_app->object = copy(app->object);
        for (auto & arg : app->args)
            new_app->args.push_back( copy(arg) );
        return new_app;
    }
    else if (auto scope = dynamic_pointer_cast<expr_scope>(expr))
    {
        copy_context_type::scope_holder scope_scope(m_copy_context);

        auto new_scope = make_shared<expr_scope>();
        new_scope->location = scope->location;
        for(auto & id : scope->ids)
        {
            auto new_expr = copy(id->expr);
            auto new_name = new_id_name(id->name);
            auto new_id = make_shared<identifier>(new_name, new_expr, id->location);
            new_scope->ids.push_back(new_id);
            m_copy_context.bind(id, new_id);
        }
        new_scope->expr = copy(scope->expr);
        return new_scope;
    }
    else
    {
        throw reduction_error("Unexpected expression type.", expr->location);
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

string func_reducer::new_id_name(const string & base)
{
    int & count = id_counts[base];
    ++count;
    ostringstream text;
    text << base << ':' << count;
    return text.str();
}

}
}
