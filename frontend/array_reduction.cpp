#include "array_reduction.hpp"
#include "linear_expr_gen.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"

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
            bool was_reduced = m_ids.find(id) != m_ids.end();
            if (!was_reduced)
            {
                id->expr = reduce(id->expr);
            }

            bool is_const = is_constant(id->expr);
            if (is_const)
                return id->expr;

            if (!was_reduced)
                m_ids.insert(id);

            return eta_expand(ref);
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

    auto arr = dynamic_pointer_cast<array>(app->object);
    if (!arr)
    {
        throw source_error("Object of array application not an array.",
                           app->object->location);
    }

    vector<int> bounds = array_size(arr);

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

    {
        context_type::scope_holder scope(m_context);

        assert(arr->vars.size() >= app->args.size());
        for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
        {
            auto & arg = app->args[arg_idx];
            auto & var = arr->vars[arg_idx];
            m_context.bind(var, arg);
        }

        auto expr = beta_reduce(arr->expr);

        // Reduce primitive ops
        if (auto prim = dynamic_pointer_cast<primitive>(expr))
        {
            expr = reduce(prim);
        }

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

    vector<int> size;

    for (auto & operand : op->operands)
    {
        auto operand_size = array_size(operand);
        if (operand_size.empty())
            continue;
        if (!size.empty() && operand_size != size)
        {
            throw source_error("Operand size mismatch.", op->location);
        }
        size = operand_size;
    }

    if (size.size())
    {
        auto arr = make_shared<array>();

        for (auto s : size)
        {
            expr_ptr range = nullptr;
            if (s != array_var::unconstrained)
                range = make_shared<constant<int>>(s);
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);
        }

        for (auto & operand : op->operands)
        {
            auto op_arr = dynamic_pointer_cast<array>(operand);
            if (!op_arr)
                continue;
            assert(arr->vars.size() == op_arr->vars.size());

            context_type::scope_holder scope(m_context);

            for (int i = 0; i < arr->vars.size(); ++i)
            {
                auto ref = make_shared<reference>(arr->vars[i], location_type());
                m_context.bind(op_arr->vars[i], ref);
            }

            auto reduced_operand = beta_reduce(op_arr->expr);
            operand = reduced_operand;
        }

        arr->expr = reduce_primitive(op);

        return arr;
    }
    else
    {
        return reduce_primitive(op);
    }
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
        return array_size(arr);
    }

    return vector<int>();
}

vector<int> array_reducer::array_size(std::shared_ptr<array> arr)
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

expr_ptr array_reducer::beta_reduce(expr_ptr expr)
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
        arr->expr = beta_reduce(arr->expr);
        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = beta_reduce(app->object);
        for (auto & arg : app->args)
            arg = beta_reduce(arg);
        return app;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        for (auto & operand : op->operands)
            operand = beta_reduce(operand);
        return op;
    }
    else
        return expr;
}

expr_ptr array_reducer::eta_expand(std::shared_ptr<reference> ref)
{
    auto id = dynamic_pointer_cast<identifier>(ref->var);
    if (!id)
        return ref;
    auto arr = dynamic_pointer_cast<array>(id->expr);
    if (!arr)
        return ref;

    auto new_arr = make_shared<array>();
    auto new_app = make_shared<array_app>();

    for (auto & var : arr->vars)
    {
        assert( !var->range || dynamic_pointer_cast<constant<int>>(var->range) );
        auto new_var = make_shared<array_var>(*var);
        new_arr->vars.push_back(new_var);
        new_app->args.push_back(make_shared<reference>(new_var, location_type()));
    }

    new_app->object = ref;
    new_arr->expr = new_app;

    return new_arr;
}

string array_reducer::new_var_name()
{
    ++var_count;
    ostringstream text;
    text << "_v" << var_count;
    return text.str();
}


}
}
