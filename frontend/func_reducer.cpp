#include "func_reducer.hpp"
#include "error.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace functional {

class wrong_arg_count_error : public source_error
{
public:
    wrong_arg_count_error(int required, int actual, location_type loc):
        source_error(msg(required,actual), loc)
    {}
private:
    string msg(int required, int actual)
    {
        ostringstream text;
        text << " Wrong number of arguments ("
             << "expected: " << required << ", "
             << "actual: " << actual
             << ")."
                ;
        return text.str();
    }
};


expr_ptr func_reducer::apply(expr_ptr expr, const vector<expr_ptr> & args,
                              const location_type & loc)
{
    vector<func_var_ptr> vars;

    reduce_context_type::scope_holder scope(m_reduce_context);

    if (args.size())
    {
        auto func = dynamic_pointer_cast<function>(expr);
        if (!func)
            throw wrong_arg_count_error(0, args.size(), loc);
        else if(func->vars.size() < args.size())
            throw wrong_arg_count_error(func->vars.size(), args.size(), loc);

        for (int i = 0; i < args.size(); ++i)
        {
            m_reduce_context.bind(func->vars[i], args[i]);
        }

        if (func->vars.size() > args.size())
        {
            vars.insert(vars.end(),
                        func->vars.begin() + args.size(),
                        func->vars.end());
        }

        expr = func->expr;
    }

    auto reduced_expr = reduce(expr);

    if (vars.size())
        return make_shared<function>(vars, reduced_expr, loc);
    else
        return reduced_expr;
}

expr_ptr func_reducer::reduce(expr_ptr expr)
{
    if (auto i = dynamic_pointer_cast<constant<int>>(expr))
    {
        return i;
    }
    else if (auto d = dynamic_pointer_cast<constant<double>>(expr))
    {
        return d;
    }
    else if (auto b = dynamic_pointer_cast<constant<bool>>(expr))
    {
        return b;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        // FIXME: Can we operate on functions?
        for (auto & operand : op->operands)
            operand = reduce(operand);
        return op;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto a_var = dynamic_pointer_cast<array_var>(ref->var))
        {
            return ref;
        }
        else if (auto f_var = dynamic_pointer_cast<func_var>(ref->var))
        {
            // TODO: remember location of reference
            auto binding = m_reduce_context.find(f_var);
            if (binding)
                return binding.value();
            else
                return ref;
        }
        else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            // TODO: Remember that the id was already reduced.
            // TODO: remember location of reference
            id->expr = reduce(id->expr);
            if (auto func = dynamic_pointer_cast<function>(id->expr))
                return id->expr;
            else
                return ref;
        }
        else
        {
            throw error("Unexpected reference type.");
        }
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        for (auto & var : arr->vars)
        {
            if (var->range)
                var->range = reduce(var->range);
        }
        arr->expr = reduce(arr->expr);
        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = reduce(app->object);
        for(auto & arg : app->args)
            arg = reduce(arg);
        return app;
    }
    else if (auto func = dynamic_pointer_cast<function>(expr))
    {
        auto reduced_expr = reduce(func->expr);
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

        vector<expr_ptr> reduced_args;
        for (auto & arg : app->args)
            reduced_args.push_back(reduce(arg));

        auto reduced_func = apply(copy(func), reduced_args, app->location);
        return reduced_func;
    }
    else if (auto scope = dynamic_pointer_cast<expr_scope>(expr))
    {
        return reduce(scope->expr);
    }
    else
    {
        throw source_error("Unexpected expression type.", expr->location);
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
        for (auto & operand : op->operands)
            new_op->operands.push_back(copy(operand));
        return new_op;
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        auto new_arr = make_shared<array>();
        new_arr->location = arr->location;

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
            assert(binding);
            return make_shared<reference>(binding.value(), ref->location);
        }
        else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            auto binding = m_copy_context.find(id);
            if (binding)
                return make_shared<reference>(binding.value(), ref->location);
            else
                return make_shared<reference>(*ref);
        }
        else
        {
            throw error("Unexpected reference type.");
        }
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
        auto new_scope = make_shared<expr_scope>();
        new_scope->location = scope->location;
        for(auto & id : scope->ids)
        {
            auto new_expr = copy(id->expr);
            auto new_id = make_shared<identifier>(id->name, new_expr, id->location);
            new_scope->ids.push_back(id);
        }
        new_scope->expr = copy(scope->expr);
        return new_scope;
    }
    else
    {
        throw source_error("Unexpected expression type.", expr->location);
    }
}

}
}
