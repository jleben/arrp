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
    else if (auto ref = dynamic_pointer_cast<func_ref>(expr))
    {
        return check(ref->id->def, {}, ref->location);
    }
    else if (auto fapp = dynamic_pointer_cast<func_app>(expr))
    {
        return check(fapp);
    }
    else if (auto def = dynamic_pointer_cast<array_def>(expr))
    {
        return check(def);
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

    return check(ref->id->def, arg_types, app->location);
}

array_type type_checker::check(std::shared_ptr<array_def> def)
{
    auto expr_type = check(def->expr);

    vector<int> size(def->vars.size() + expr_type.size.size());
    int i = 0;
    for(; i < def->vars.size(); ++i)
    {
        auto & var = def->vars[i];
        if (auto c = dynamic_cast<constant<int>*>(var->range.get()))
            size[i] = c->value;
        else
            size[i] = array_var::unconstrained;
    }
    for(; i < size.size(); ++i)
    {
        size[i] = expr_type.size[i - def->vars.size()];
    }

    return array_type(primitive_type::integer, size);
}

}
}
