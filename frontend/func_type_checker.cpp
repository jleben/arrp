#include "func_type_checker.hpp"
#include "error.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace functional {

array_type type_checker::check(func_def_ptr func, const vector<array_type> & args)
{
    return check(func, args, location_type());
}

array_type type_checker::check
(func_def_ptr func,
 const vector<array_type> & args,
 const location_type & loc)
{
    if (args.size() != func->vars.size())
    {
        ostringstream msg;
        msg << "Function " << func->name
            << " expects " << func->vars.size() << " arguments"
            << " but " << args.size() << " given.";
        throw source_error(msg.str(), loc);
    }

    context_type::scope_holder scope(m_context);

    for (int i = 0; i < args.size(); ++i)
    {
        m_context.bind(func->vars[i], args[i]);
    }

    return check(func->expr);
}

array_type type_checker::check(expr_ptr expr)
{
    if (auto i = dynamic_pointer_cast<constant<int>>(expr))
    {
        return array_type(primitive_type::integer);
    }
    else if (auto d = dynamic_pointer_cast<constant<double>>(expr))
    {
        return array_type(primitive_type::real);
    }
    else if (auto b = dynamic_pointer_cast<constant<bool>>(expr))
    {
        return array_type(primitive_type::boolean);
    }
    else if (auto ref = dynamic_pointer_cast<array_var_ref>(expr))
    {
        return array_type(primitive_type::integer);
    }
    else if (auto ref = dynamic_pointer_cast<func_var_ref>(expr))
    {
        auto ctx_item = m_context.find(ref->var);
        assert(ctx_item);
        return ctx_item.value();
    }
    else if (auto ref = dynamic_pointer_cast<expr_ref>(expr))
    {
        return check(ref->expr);
    }
    else if (auto ref = dynamic_pointer_cast<func_ref>(expr))
    {
        // FIXME: redundant check of definition
        return check(ref->func, {}, ref->location);
    }
    else if (auto fapp = dynamic_pointer_cast<func_app>(expr))
    {
        return check(fapp);
    }
    else if (auto def = dynamic_pointer_cast<array_def>(expr))
    {
        return check(def);
    }
    else if (auto aapp = dynamic_pointer_cast<array_app>(expr))
    {
        return check(aapp);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        return check(op);
    }
    else
    {
        throw source_error("Unexpected expression type.", expr->location);
    }
}

array_type type_checker::check(std::shared_ptr<primitive> op)
{
    vector<array_type> operand_types;
    operand_types.reserve(op->operands.size());
    for (auto & operand : op->operands)
    {
        operand_types.push_back(check(operand));
    }

    vector<int> size;
    for (auto & type : operand_types)
    {
        if (type.size.empty())
            continue;
        if (size.empty())
            size = type.size;
        else if (size != type.size)
            throw source_error("Incompatible operand sizes.", op->location);
    }

    // TODO: check primitive type compatibility

    return array_type(primitive_type::integer, size);
}

array_type type_checker::check(std::shared_ptr<func_app> app)
{
    auto ref = dynamic_pointer_cast<func_ref>(app->object);
    if (!ref)
        throw source_error("Object of function application not a function.",
                           app->object->location);

    vector<array_type> arg_types;
    arg_types.reserve(app->args.size());
    for( auto & arg : app->args )
        arg_types.push_back( check(arg) );

    return check(ref->func, arg_types, app->location);
}

array_type type_checker::check(std::shared_ptr<array_def> def)
{
    auto expr_type = check(def->expr);

    vector<int> result_size(def->vars.size() + expr_type.size.size());
    int i = 0;
    for(; i < def->vars.size(); ++i)
    {
        auto & var = def->vars[i];
        if (auto c = dynamic_cast<constant<int>*>(var->range.get()))
        {
            if (c->value < 1)
            {
                location_type loc;
                loc.begin = loc.end = def->location.begin;
                loc.columns(1);

                ostringstream msg;
                msg << "Non-positive variable range (" <<  c->value << ")";
                throw source_error(msg.str(), loc);
            }
            result_size[i] = c->value;
        }
        else
        {
            result_size[i] = array_var::unconstrained;
        }
    }
    for(; i < result_size.size(); ++i)
    {
        result_size[i] = expr_type.size[i - def->vars.size()];
    }

    for (int dim = 0; dim < result_size.size(); ++dim)
    {
        if (result_size[dim] == array_var::unconstrained && dim != 0)
        {
            location_type loc;
            loc.begin = loc.end = def->location.begin;
            loc.columns(1);

            throw source_error("Only the first dimension can be unconstrained.", loc);
        }
    }

    return array_type(primitive_type::integer, result_size);
}

array_type type_checker::check(std::shared_ptr<array_app> app)
{
    auto object_type = check(app->object);

    if (app->args.size() > object_type.size.size())
    {
        ostringstream msg;
        msg << "Too many arguments: "
            << object_type.size.size() << " expected, "
            << app->args.size() << " given.";
        throw source_error(msg.str(), app->location);
    }

    for (int arg_idx = 0; arg_idx < (int) app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];
        auto expr = linear(arg);
        linexpr max_expr;
        for (auto & term : expr)
        {
            int value = term.second;
            if (term.first)
            {
                auto avar = dynamic_pointer_cast<array_var>(term.first);
                if (!avar)
                    throw source_error("Expression contains invalid variable type.",
                                       arg->location);
                if (auto c = dynamic_pointer_cast<constant<int>>(avar->range))
                    value *= std::max(0, c->value - 1);
                else // unconstrained
                {
                    max_expr = max_expr + term;
                    continue;
                }
            }
            max_expr = max_expr + value;
        }
        int var_count = max_expr.var_count();
        if (var_count > 1)
            throw source_error("More than 2 unconstrained variables.",
                               arg->location);
        if (var_count == 1)
        {
            if (object_type.size[arg_idx] != array_var::unconstrained)
                throw source_error("Unbounded argument to bounded variable.",
                                   arg->location);
        }
        else
        {
            assert(max_expr.is_constant());
            int max_value = max_expr.constant();
            int range = object_type.size[arg_idx];
            if (range != array_var::unconstrained && max_value >= range)
            {
                ostringstream msg;
                msg << "Argument bounds (" << max_value << ")"
                    << " larger than variable bounds (" << range << ")";
                throw source_error(msg.str(),
                                   arg->location);
            }
        }
    }

    array_type result_type(object_type.elem_type);
    if (app->args.size() < object_type.size.size())
        result_type.size = vector<int>(object_type.size.begin() + app->args.size(),
                                       object_type.size.end());

    return result_type;
}

linexpr type_checker::linear(expr_ptr e)
{
    if (auto c = dynamic_pointer_cast<constant<int>>(e))
    {
        return linexpr(c->value);
    }
    else if (auto v = dynamic_pointer_cast<array_var_ref>(e))
    {
        return linexpr(v->var);
    }
    // TODO: Function variables: get bound value, not type
    else if (auto op = dynamic_pointer_cast<primitive>(e))
    {
        switch(op->type)
        {
        case primitive_op::add:
            return (linear(op->operands[0]) + linear(op->operands[1]));
        case primitive_op::subtract:
            return (linear(op->operands[0]) + -linear(op->operands[1]));
        case primitive_op::negate:
            return (-linear(op->operands[0]));
        case primitive_op::multiply:
        {
            auto lhs = linear(op->operands[0]);
            auto rhs = linear(op->operands[1]);
            if (!lhs.is_constant() && !rhs.is_constant())
                throw source_error("Not a linear expression.", e->location);
            if (lhs.is_constant())
                return rhs * lhs.constant();
            else
                return lhs * rhs.constant();
        }
        default:
            throw source_error("Not a linear expression.", e->location);
        }
    }
    else
        throw source_error("Not a linear expression.", e->location);
}

}
}
