#include "array_reduction.hpp"
#include "linear_expr_gen.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"

using namespace std;

namespace stream {
namespace functional {

expr_ptr array_reducer::reduce(expr_ptr expr)
{
    if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        return reduce(arr);
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        return reduce(app);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        return reduce(op);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            // TODO: Remember that the id was already reduced.
            id->expr = reduce(id->expr);
        }
        return ref;
    }

    return expr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array> arr)
{
    for (auto & var : arr->vars)
    {
        if (var->range)
        {
            var->range = reduce(var->range);

            if (auto c = dynamic_pointer_cast<constant<int>>(var->range))
            {
                if (c->value < 1)
                {
                    ostringstream msg;
                    msg << "Non-positive variable range (" <<  c->value << ")";
                    throw source_error(msg.str(), var->range->location);
                }
            }
            else
            {
                throw source_error("Array bounds not a constant integer.",
                                   var->range->location);
            }
        }
    }

    arr->expr = reduce(arr->expr);

    if (auto nested_arr = dynamic_pointer_cast<array>(arr->expr))
    {
        arr->vars.insert(arr->vars.end(),
                         nested_arr->vars.begin(),
                         nested_arr->vars.end());
        arr->expr = nested_arr->expr;
    }

    return arr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array_app> app)
{
    app->object = reduce(app->object);

    vector<int> bounds = array_size(app->object);

    if (bounds.empty())
        throw source_error("Object of array application not an array.",
                           app->object->location);

    if (bounds.size() < app->args.size())
    {
        ostringstream msg;
        msg << "Too many arguments in array application: "
            << bounds.size() << " expected, "
            << app->args.size() << " given.";
        throw source_error(msg.str(), app->location);
    }

    for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];
        arg = reduce(arg);
        auto lin_arg = to_linear_expr(arg);
        auto max_arg = maximum(lin_arg);
        int bound = bounds[arg_idx];
        if (max_arg.is_constant())
        {
            int max_value = max_arg.constant();
            if (bound != array_var::unconstrained && max_value >= bound)
            {
                ostringstream msg;
                msg << "Argument range (" << max_value << ")"
                    << " out of array bound (" << bound << ")";
                throw source_error(msg.str(),
                                   arg->location);
            }
        }
        else
        {
            if (bound != array_var::unconstrained)
            {
                throw source_error("Unbounded argument to"
                                   " bounded array dimension.",
                                   arg->location);
            }
        }

        // TODO: store the linear expression
    }

    if (auto arr = dynamic_pointer_cast<array>(app->object))
    {
        context_type::scope_holder scope(m_context);

        assert(arr->vars.size() >= app->args.size());
        for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
        {
            auto & arg = app->args[arg_idx];
            auto & var = arr->vars[arg_idx];
            m_context.bind(var, arg);
        }

        auto expr = replace_vars_in(arr->expr);

        // TODO: reduce primitive ops after replacing vars

        if (arr->vars.size() > app->args.size())
        {
            vector<array_var_ptr> remaining_vars(arr->vars.begin() + app->args.size(),
                                                 arr->vars.end());
            arr->vars = remaining_vars;
            arr->expr = expr;
            arr->location = app->location;
            return arr;
        }
        else
        {
            return expr;
        }
    }

    return app;
}

expr_ptr array_reducer::reduce(std::shared_ptr<primitive> op)
{
    for (auto & operand : op->operands)
        operand = reduce(operand);
    // FIXME: handle array-type operands
    return reduce_primitive(op);
}

vector<int> array_reducer::array_size(expr_ptr expr)
{
    if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            expr = id->expr;
        }
    }

    if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        vector<int> s;
        for (auto & var : arr->vars)
        {
            if (var->range)
            {
                auto c = dynamic_pointer_cast<constant<int>>(var->range);
                assert(c);
                s.push_back(c->value);
            }
            else
            {
                s.push_back(array_var::unconstrained);
            }
        }
        return s;
    }

    return vector<int>();
}

expr_ptr array_reducer::replace_vars_in(expr_ptr expr)
{
    if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
        {
            auto binding = m_context.find(avar);
            if (binding)
                return binding.value();
        }
        return ref;
    }
    if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        arr->expr = replace_vars_in(arr->expr);
        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = replace_vars_in(app->object);
        for (auto & arg : app->args)
            arg = replace_vars_in(arg);
        return app;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        for (auto & operand : op->operands)
            operand = replace_vars_in(operand);
        return op;
    }
    else
        return expr;
}

}
}
