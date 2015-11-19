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
             << "required: " << required << ", "
             << "actual: " << actual
             << ")."
                ;
        return text.str();
    }
};

func_def_ptr func_reducer::reduce(func_def_ptr func, const vector<expr_ptr> & args,
                                  const location_type & loc)
{
    if (func->vars.size() != args.size())
        throw wrong_arg_count_error(func->vars.size(), args.size(), loc);

    auto reduced_func = make_shared<func_def>();
    reduced_func->name = func->name;
    reduced_func->location = func->location;

    context_type::scope_holder scope(m_context);

    for (int i = 0; i < args.size(); ++i)
    {
        m_context.bind(func->vars[i], args[i]);
    }

    for (auto def : func->defs)
    {
        if (def->vars.empty())
        {
            // FIXME: location
            auto reduced_def = reduce(def, {}, location_type());
            // FIXME: have to have func_id to bind to!
            m_func_context.bind(def, reduced_def);
        }
    }

    auto reduced_expr = reduce(func->expr);
    reduced_func->expr = reduced_expr;

    return reduced_func;
}

expr_ptr func_reducer::reduce(expr_ptr expr)
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
    else if (auto ref = dynamic_pointer_cast<func_ref>(expr))
    {
        auto ctx_item = m_func_context.find(ref->func);
        if (ctx_item)
            return make_shared<func_ref>(ctx_item.value(), ref->location);
        else
            return make_shared<func_ref>(*ref);
    }
    else if (auto ref = dynamic_pointer_cast<func_var_ref>(expr))
    {
        auto ctx_item = m_context.find(ref->var);
        assert(ctx_item);
        auto bound_expr = ctx_item.value();
        return make_shared<expr_ref>(bound_expr, ref->location);
    }
    else if (auto fapp = dynamic_pointer_cast<func_app>(expr))
    {
        auto object = reduce(fapp->object);
        auto ref = dynamic_pointer_cast<func_ref>(object);
        if (!ref)
        {
            throw source_error("Object of function application not a function.",
                               fapp->object->location);
        }

        vector<expr_ptr> reduced_args;
        for (auto & arg : fapp->args)
            reduced_args.push_back(reduce(arg));

        auto reduced_func = reduce(ref->func, reduced_args, fapp->location);
        auto reduced_ref = make_shared<func_ref>(reduced_func, ref->location);
        return reduced_ref;
    }
    else if (auto def = dynamic_pointer_cast<array_def>(expr))
    {
        auto reduced = make_shared<array_def>();

        for (auto & var : def->vars)
        {
            expr_ptr rrange;
            if (var->range)
                rrange = reduce(var->range);
            auto rvar = make_shared<array_var>(rrange);
            reduced->vars.push_back(rvar);
        }

        array_context_type::scope_holder scope(m_array_context);

        for (int v = 0; v < def->vars.size(); ++v)
        {
            m_array_context.bind(def->vars[v], reduced->vars[v]);
        }

        reduced->expr = reduce(def->expr);
        reduced->location = def->location;

        return reduced;
    }
    else if (auto ref = dynamic_pointer_cast<array_var_ref>(expr))
    {
        auto ctx_item = m_array_context.find(ref->var);
        assert(ctx_item);
        return make_shared<array_var_ref>(ctx_item.value(), ref->location);
    }
    else if (auto aapp = dynamic_pointer_cast<array_app>(expr))
    {
        auto reduced = make_shared<array_app>();
        reduced->object = reduce(aapp->object);
        for (auto & arg : aapp->args)
            reduced->args.push_back(reduce(arg));
        reduced->location = aapp->location;
        return reduced;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        auto reduced = make_shared<primitive>();
        reduced->location = op->location;
        reduced->type = op->type;
        for (auto & operand : op->operands)
            reduced->operands.push_back(reduce(operand));
        return reduced;
    }
    else
    {
        throw source_error("Unexpected expression type.", expr->location);
    }
}

}
}
