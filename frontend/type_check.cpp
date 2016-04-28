#include "type_check.hpp"
#include "linear_expr_gen.hpp"
#include "error.hpp"
#include "../common/func_model_printer.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

string text(primitive_op op, const vector<primitive_type> & args)
{
    ostringstream text;
    text << op
        << " ( ";
    for (auto & t : args)
        text << t << " ";
    text << ")";
    return text.str();
}

string text(const vector<int> & v)
{
    ostringstream text;
    int i = 0;
    for (auto & e : v)
    {
        if (i > 0)
            text << ",";
        text << e;
        ++i;
    }
    return text.str();
}

void type_checker::process(const expr_ptr & expr)
{
    visit(expr);
}

type_ptr type_checker::visit(const expr_ptr & expr)
{
    if (!expr->type)
        expr->type = visitor<type_ptr>::visit(expr);

    return expr->type;
}

type_ptr type_checker::visit_int(const shared_ptr<constant<int>> &)
{
    return make_shared<scalar_type>(primitive_type::integer);
}

type_ptr type_checker::visit_double(const shared_ptr<constant<double>> &)
{
    return make_shared<scalar_type>(primitive_type::real);
}

type_ptr type_checker::visit_bool(const shared_ptr<constant<bool>> &)
{
    return make_shared<scalar_type>(primitive_type::boolean);
}

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    type_ptr type;

    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        type = visit(id->expr);
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        type = make_shared<scalar_type>(primitive_type::integer);
    }
    else
    {
        throw error("Unexpected.");
    }

    return type;
}

type_ptr type_checker::visit_array_self_ref(const shared_ptr<array_self_ref> & self)
{
    vector<int> size;

    auto & arr = self->arr;

    for (auto & var : arr->vars)
    {
        if (var->range)
        {
            auto c = dynamic_pointer_cast<constant<int>>(var->range.expr);
            if (!c)
                throw error("Unexpected.");
            size.push_back(c->value);
        }
        else
        {
            size.push_back(-1);
        }
    }

    return make_shared<array_type>(size, primitive_type::undefined);
}

type_ptr type_checker::visit_primitive(const shared_ptr<primitive> & prim)
{
    vector<int> common_size;
    vector<primitive_type> elem_types;

    for (auto & operand : prim->operands)
    {
        auto type = visit(operand);

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw source_error("Function not allowed as operand.", operand.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            if (common_size.empty())
            {
                common_size = arr->size;
            }
            else if (common_size != arr->size)
            {
                ostringstream msg;
                msg << "Operand size mismatch:"
                    << text(common_size) << " & " << text(arr->size);
                throw source_error(msg.str(), prim->location);
            }
            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
        }
        else
        {
            throw error("Unexpected operand type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = result_type(prim->kind, elem_types);
    }
    catch (no_type &)
    {
        string msg("Invalid operand types: ");
        msg += text(prim->kind, elem_types);
        throw source_error(msg, prim->location);
    }
    catch (ambiguous_type &)
    {
        string msg("Ambiguous type resolution:");
        msg += text(prim->kind, elem_types);
        throw source_error(msg, prim->location);
    }

    if (common_size.empty())
        return make_shared<scalar_type>(result_elem_type);
    else
        return make_shared<array_type>(common_size, result_elem_type);
}

type_ptr type_checker::visit_cases(const shared_ptr<case_expr> & cexpr)
{
    vector<int> common_size;
    vector<primitive_type> elem_types;

    for (auto & c : cexpr->cases)
    {
        auto & domain = c.first;
        auto & expr = c.second;

        auto type = visit(expr);

        to_linear_set(domain);

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw source_error("Function not allowed in case.", expr.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            if (common_size.empty())
            {
                common_size = arr->size;
            }
            else if (common_size != arr->size)
            {
                ostringstream msg;
                msg << "Case size mismatch:"
                    << text(common_size) << " & " << text(arr->size);
                throw source_error(msg.str(), cexpr->location);
            }
            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
        }
        else
        {
            throw error("Unexpected case type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw source_error("Incompatible case types.", cexpr->location);
    }

    if (common_size.empty())
        return make_shared<scalar_type>(result_elem_type);
    else
        return make_shared<array_type>(common_size, result_elem_type);
}

type_ptr type_checker::visit_array(const shared_ptr<array> & arr)
{
    vector<int> size;

    for (auto & var : arr->vars)
    {
        if (var->range)
        {
            if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
            {
                if (c->value < 1)
                {
                    ostringstream msg;
                    msg << "Array bound not positive (" <<  c->value << ")";
                    throw source_error(msg.str(), var->range.location);
                }
                size.push_back(c->value);
            }
            else
            {
                throw source_error("Array bound not a constant integer.",
                                   var->range.location);
            }
        }
        else
        {
            size.push_back(-1);
        }
    }

    auto elem_type = visit(arr->expr);

    primitive_type prim_elem_type;

    if (auto func = dynamic_pointer_cast<function_type>(elem_type))
    {
        throw source_error("Function not allowed as array element.",
                           arr->expr.location);
    }
    else if (auto nested_arr = dynamic_pointer_cast<array_type>(elem_type))
    {
        size.insert(size.end(), nested_arr->size.begin(), nested_arr->size.end());
        prim_elem_type = nested_arr->element;
    }
    else if (auto scalar = dynamic_pointer_cast<scalar_type>(elem_type))
    {
        prim_elem_type = scalar->primitive;
    }

    return make_shared<array_type>(size, prim_elem_type);
}

type_ptr type_checker::visit_array_app(const shared_ptr<array_app> & app)
{
    auto object_type = dynamic_pointer_cast<array_type>(visit(app->object));

    if (!object_type)
        throw source_error("Object of array application is not an array.",
                           app->object.location);

    auto & object_size = object_type->size;

    if (app->args.size() > object_size.size())
    {
        ostringstream msg;
        msg << "Too many arguments in array application: "
            << object_type->size.size() << " expected, "
            << app->args.size() << " given.";
        throw source_error(msg.str(), app->location);
    }

    if(auto arr_self = dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        if (object_size.size() != app->args.size())
        {
            ostringstream msg;
            msg << "Array self reference partially applied."
                << " Expected " << object_size.size() << " arguments, but "
                << app->args.size() << " given.";
            throw source_error(msg.str(), app->location);
        }
    }
    else if (auto arr = dynamic_pointer_cast<array>(app->object.expr))
    {
        if (arr->is_recursive)
            throw source_error("Direct application of recursive arrays not supported.",
                               app->location);
    }

    for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];
        auto lin_arg = to_linear_expr(arg);
        auto max_arg = maximum(lin_arg);
        int bound = object_size[arg_idx];
        if (max_arg.is_constant())
        {
            int max_value = max_arg.constant();
            if (bound != array_var::unconstrained && max_value >= bound)
            {
                ostringstream msg;
                msg << "Argument range (" << max_value << ")"
                    << " out of array bound (" << bound << ")";
                throw source_error(msg.str(),
                                   arg.location);
            }
        }
        else
        {
            if (bound != array_var::unconstrained)
            {
                printer p;
                cout << "Unbounded argument: ";
                p.print(arg, cout); cout << endl;
                throw source_error("Unbounded argument to"
                                   " bounded array dimension.",
                                   arg.location);
            }
        }
    }

    vector<int> remaining_size(object_size.begin() + app->args.size(),
                               object_size.end());

    if (remaining_size.empty())
        return make_shared<scalar_type>(object_type->element);
    else
        return make_shared<array_type>(remaining_size, object_type->element);
}

type_ptr type_checker::visit_array_size(const shared_ptr<array_size> & as)
{
    throw error("Unexpected.");
}

type_ptr type_checker::visit_func_app(const shared_ptr<func_app> & app)
{
    auto func_type = dynamic_pointer_cast<function_type>(visit(app->object));
    if (!func_type)
        throw source_error("Not a function.", app->object->location);

    if (app->args.size() > func_type->arg_count)
    {
        ostringstream msg;
        msg << "Too many arguments in function application: "
            << func_type->arg_count << " expected, "
            << app->args.size() << " given.";
        throw source_error(msg.str(), app->location);
    }

    int remaining_arg_count = func_type->arg_count - app->args.size();

    if (remaining_arg_count)
        return make_shared<function_type>(remaining_arg_count);
    else
        throw error("Unexpected.");
}

type_ptr type_checker::visit_func(const shared_ptr<function> & func)
{
    return make_shared<function_type>(func->vars.size());
}

type_ptr type_checker::visit_affine(const shared_ptr<affine_expr> &)
{
    throw error("Not supported.");
}

}
}
