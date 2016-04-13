#include "array_reduction.hpp"
#include "linear_expr_gen.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"
#include "../utility/debug.hpp"
#include "../common/func_model_printer.hpp"

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

void array_reducer::process(id_ptr id)
{
    // FIXME: Don't store propagated constants.

    bool was_processed = m_ids.find(id) != m_ids.end();
    if (was_processed)
        return;

    m_ids.insert(id);

    m_declared_vars.push_back(nullptr);
    m_unbound_vars.emplace();

    id->expr = eta_expand(reduce(id->expr));

    auto & unbound_vars = m_unbound_vars.top();

    if (!unbound_vars.empty())
    {
        // Upgrade this id's expression to array

        vector<array_var_ptr> ordered_unbound_vars;

        for (auto & var : m_declared_vars)
        {
            if (var == nullptr)
                continue;
            if (unbound_vars.find(var) != unbound_vars.end())
            {
                ordered_unbound_vars.push_back(var);
            }
        }

        assert(ordered_unbound_vars.size() == unbound_vars.size());

        m_context.enter_scope();

        vector<array_var_ptr> new_vars;

        for (auto & var : ordered_unbound_vars)
        {
            assert( !var->range || dynamic_pointer_cast<constant<int>>(var->range) );

            auto new_var = make_shared<array_var>(*var);
            new_var->location = location_type();
            new_vars.push_back(new_var);

            auto new_ref = make_shared<reference>(new_var, location_type());
            m_context.bind(var, new_ref);
        }

        auto ar = make_shared<array>();
        ar->vars = new_vars;
        ar->expr = substitute(id->expr);

        m_context.exit_scope();

        // Reduce to get rid of nested arrays
        id->expr = reduce(ar);


        // Substite for references to this id

        expr_ptr sub = make_shared<reference>(id, location_type());
        sub = eta_expand(sub);
        vector<expr_ptr> args;
        for (auto & var : ordered_unbound_vars)
        {
            auto ref = make_shared<reference>(var, location_type());
            args.push_back(ref);
        }
        sub = apply(sub, args);

        m_id_sub.emplace(id, sub);
    }

    m_unbound_vars.pop();
    m_declared_vars.pop_back();

    if (verbose<functional::model>::enabled())
    {
        cout << "Reduced array: ";
        printer p;
        p.print(id, cout);
        cout << endl;
    }
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
    else if (auto as = dynamic_pointer_cast<functional::array_size>(expr))
    {
        return reduce(as);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        return reduce(op);
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        return reduce(c);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            process(id);

            bool is_const = is_constant(id->expr);
            if (is_const)
                return id->expr;

            // Is there a substitution for references to this id?
            auto sub_it = m_id_sub.find(id);
            if (sub_it != m_id_sub.end())
                return sub_it->second;
        }
        else if (auto var = dynamic_pointer_cast<array_var>(ref->var))
        {
            assert(!m_unbound_vars.empty());
            bool is_bound = false;
            for (auto v = m_declared_vars.rbegin();
                 v != m_declared_vars.rend() && *v != nullptr; ++v)
            {
                if (*v == var)
                {
                    is_bound = true;
                    break;
                }
            }

            if (!is_bound)
            {
                m_unbound_vars.top().insert(var);
            }
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

    for (auto & var : arr->vars)
        m_declared_vars.push_back(var);

    arr->expr = eta_expand(reduce(arr->expr));

    for (int i = 0; i < (int) arr->vars.size(); ++i)
        m_declared_vars.pop_back();

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

    auto object = eta_expand(app->object);
    auto arr = dynamic_pointer_cast<array>(object);
    bool is_self = false;

    if (arr)
    {
        if (arr->is_recursive)
            throw source_error("Direct application of recursive arrays not supported.",
                               app->location);
    }
    else if(auto arr_self = dynamic_pointer_cast<array_self_ref>(app->object))
    {
        arr = arr_self->arr;
        is_self = true;
    }

    if (!arr)
    {
        throw source_error("Object of array application not an array.",
                           app->object->location);
    }

    vector<int> bounds = array_size(arr);

    if (is_self)
    {
        if (bounds.size() != app->args.size())
        {
            ostringstream msg;
            msg << "Array self reference partially applied."
                << " Expected " << bounds.size() << " arguments, but "
                << app->args.size() << " given.";
            throw source_error(msg.str(), app->location);
        }
    }
    else if (bounds.size() < app->args.size())
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

        // arg = make_shared<affine_expr>(lin_arg);
    }

    if (!is_self)
    {
        return apply(arr, app->args);
    }

    return app;
}

expr_ptr array_reducer::reduce(std::shared_ptr<functional::array_size> as)
{
    as->object = reduce(as->object);

    auto size = array_size(as->object);
    if (size.size() < 1)
    {
        throw source_error("Not an array.",
                           as->object->location);
    }

    int dim;
    if (as->dimension)
    {
        as->dimension = reduce(as->dimension);
        auto cint = dynamic_pointer_cast<constant<int>>(as->dimension);
        if (!cint)
        {
            throw source_error("Array dimension index not a constant integer.",
                               as->location);
        }
        int val = cint->value;
        if (val < 1 || val > (int)size.size())
        {
            throw source_error("Array dimension index out of bounds.",
                               as->location);
        }
        dim = val - 1;
    }
    else
    {
        dim = 0;
    }

    int dim_size = size[dim];
    if (dim_size < 0)
    {
        throw source_error("Array dimension is infinite.",
                           as->location);
    }

    return make_shared<constant<int>>(dim_size);
}

expr_ptr array_reducer::reduce(std::shared_ptr<primitive> op)
{
    for (auto & operand : op->operands)
        operand = eta_expand(reduce(operand));

    vector<int> result_size;

    for (auto & operand : op->operands)
    {
        if (auto arr = dynamic_pointer_cast<array>(operand))
        {
            if (arr->is_recursive)
                throw source_error("Recursive arrays not supported as operands.",
                                   arr->location);
        }

        auto operand_size = array_size(operand);
        if (operand_size.empty())
            continue;
        if (!result_size.empty() && operand_size != result_size)
        {
            throw source_error("Operand size mismatch.", op->location);
        }
        result_size = operand_size;
    }

    if (!result_size.empty())
    {
        auto arr = make_shared<array>();

        // Create vars for result array

        for (auto s : result_size)
        {
            expr_ptr range = nullptr;
            if (s != array_var::unconstrained)
                range = make_shared<constant<int>>(s);
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);
        }

        // Reduce arrays in operands:

        for (auto & operand : op->operands)
        {
            vector<expr_ptr> args;
            for (auto & v : arr->vars)
                args.push_back(make_shared<reference>(v, location_type()));

            operand = apply(operand, args);
        }

        // Check subdomains in operands:

        for (auto & operand : op->operands)
        {
            if (dynamic_pointer_cast<case_expr>(operand))
            {
                throw source_error("Case expression not supported as operand.",
                                   operand->location);
            }
        }

        arr->expr = reduce_primitive(op);

        return arr;
    }
    else
    {
        return reduce_primitive(op);
    }
}

expr_ptr array_reducer::reduce(std::shared_ptr<case_expr> cexpr)
{
    vector<int> common_size;

    for (auto & a_case : cexpr->cases)
    {
        auto & domain = a_case.first;
        auto & expr = a_case.second;
        auto src_expr = expr;

        to_linear_set(domain);
        expr = eta_expand(reduce(expr));

        auto size = array_size(expr);
        if (size.empty())
            continue;
        if (!common_size.empty() && size != common_size)
        {
            throw source_error("Subdomain has elements of different size"
                               " than other subdomains.",
                               src_expr->location);
        }
        common_size = size;
    }

    if (!common_size.empty())
    {
        auto arr = make_shared<array>();

        for (auto s : common_size)
        {
            expr_ptr range = nullptr;
            if (s != array_var::unconstrained)
                range = make_shared<constant<int>>(s);
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);
        }

        for (auto & a_case : cexpr->cases)
        {
            auto & expr = a_case.second;

            // Reduce array
            vector<expr_ptr> args;
            for (auto & v : arr->vars)
                args.push_back(make_shared<reference>(v, location_type()));

            expr = apply(expr, args);

            if (dynamic_pointer_cast<case_expr>(expr))
            {
                // FIXME: location of reduced expression
                throw source_error("Nested cases not supported.", expr->location);
            }
        }

        arr->expr = cexpr;

        return arr;
    }

    return cexpr;
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

expr_ptr array_reducer::apply(expr_ptr expr, const vector<expr_ptr> & args)
{
    if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        // This handles recursive array applications
        app->args.insert(app->args.end(), args.begin(), args.end());
        return app;
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        assert(arr->vars.size() >= args.size());

        context_type::scope_holder scope(m_context);

        for (int i = 0; i < args.size(); ++i)
        {
            m_context.bind(arr->vars[i], args[i]);
        }

        auto expr = substitute(arr->expr);

        // Reduce primitive ops
        if (auto prim = dynamic_pointer_cast<primitive>(expr))
        {
            expr = reduce(prim);
        }

        if (arr->vars.size() > args.size())
        {
            vector<array_var_ptr> remaining_vars(arr->vars.begin() + args.size(),
                                                 arr->vars.end());
            arr->vars = remaining_vars;
            arr->expr = expr;
            arr->location = location_type();
            // FIXME: location
            return arr;
        }
        else
        {
            return expr;
        }
    }

    return expr;
}

expr_ptr array_reducer::substitute(expr_ptr expr)
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
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        for (auto & a_case : c->cases)
        {
            a_case.first = substitute(a_case.first);
            a_case.second = substitute(a_case.second);
        }
        return c;
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        arr->expr = substitute(arr->expr);
        return arr;
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->object = substitute(app->object);
        for (auto & arg : app->args)
            arg = substitute(arg);
        return app;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        for (auto & operand : op->operands)
            operand = substitute(operand);
        return op;
    }
    else
        return expr;
}

expr_ptr array_reducer::eta_expand(expr_ptr expr)
{
    auto ref = dynamic_pointer_cast<reference>(expr);
    if (ref)
        return eta_expand(ref);
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
