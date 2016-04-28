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

array_reducer::array_reducer(name_provider & nmp):
    m_declared_vars("declared var"),
    m_name_provider(nmp),
    m_copier(m_processed_ids, nmp)
{
    m_declared_vars.set_enabled(verbose<functional::model>::enabled());

    m_printer.set_print_scopes(false);
    m_printer.set_print_var_address(true);
}

id_ptr array_reducer::process(id_ptr id)
{
    printer p;
    p.set_print_scopes(false);
    p.set_print_var_address(true);

    bool was_processed = m_processed_ids.find(id) != m_processed_ids.end();
    if (was_processed)
        return id;

    if (verbose<functional::model>::enabled())
    {
        cout << "Processing id: ";
        p.print(id, cout);
        cout << endl;
    }

    m_processed_ids.insert(id);

    m_declared_vars.push(nullptr);
    m_unbound_vars.emplace();

    id->expr = eta_expand(reduce(id->expr));

    if (verbose<functional::model>::enabled())
    {
        cout << "Processed id: ";
        p.print(id, cout);
        cout << endl;
    }

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
                if (verbose<functional::model>::enabled())
                    cout << "Id has free var: " << var << endl;

                ordered_unbound_vars.push_back(var);
            }
        }

        assert(ordered_unbound_vars.size() == unbound_vars.size());

        m_context.enter_scope();

        vector<array_var_ptr> new_vars;

        for (auto & var : ordered_unbound_vars)
        {
            assert( !var->range || dynamic_pointer_cast<constant<int>>(var->range.expr) );

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

        auto expr = reduce(ar);
        auto new_id_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_id_name, expr, id->location);
        m_processed_ids.insert(new_id);

        // Substite for references to this id

        if (verbose<functional::model>::enabled())
            cout << "Creating substitute for id: " << id << endl;

        expr_ptr sub = make_shared<reference>(new_id, location_type());
        sub = eta_expand(sub);
        vector<expr_ptr> args;
        for (auto & var : ordered_unbound_vars)
        {
            auto ref = make_shared<reference>(var, location_type());
            args.push_back(ref);
            if (verbose<functional::model>::enabled())
                cout << "Adding var to substitute: " << var << endl;
        }
        sub = apply(sub, args);

        m_id_sub.emplace(id, sub);

        if (verbose<functional::model>::enabled())
        {
            cout << "Expanded id " << id << " to ";
            p.print(new_id, cout); cout << endl;

            cout << "Substitute = "; p.print(sub, cout); cout << endl;
        }

        id = new_id;
    }

    m_unbound_vars.pop();
    m_declared_vars.pop();

    return id;
}

expr_ptr array_reducer::reduce(expr_ptr expr)
{
    printer p;
    if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        return reduce(arr);
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        //cout << "++ app: "; p.print(app, cout); cout << endl;
        auto r = reduce(app);
        //cout << "-- app: "; p.print(r, cout); cout << endl;
        return r;
    }
    else if (auto as = dynamic_pointer_cast<functional::array_size>(expr))
    {
        return reduce(as);
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        //cout << "++ primitive: "; p.print(op, cout); cout << endl;
        auto r = reduce(op);
        //cout << "-- primitive: "; p.print(r, cout); cout << endl;
        return r;
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        //cout << "++ case: "; p.print(c, cout); cout << endl;
        auto r = reduce(c);
        //cout << "-- case: "; p.print(r, cout); cout << endl;
        return r;
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        return reduce(ref);
    }
    else if (auto self = dynamic_pointer_cast<array_self_ref>(expr))
    {
        auto sub_it = m_array_ref_sub.find(self->arr);
        if (sub_it == m_array_ref_sub.end())
            return expr;

        array_ptr sub = sub_it->second;

        self->arr = sub;

        auto app = make_shared<array_app>();
        app->object = self;
        for (auto & var : sub->vars)
            app->args.emplace_back(make_shared<reference>(var, location_type()));

        return app;
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

            if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
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

    decl_var_stacker declared_vars(m_declared_vars);

    for (auto & var : arr->vars)
    {
        declared_vars.push(var);
    }

    arr->expr = eta_expand(reduce(arr->expr));

    if (auto nested_arr = dynamic_pointer_cast<array>(arr->expr.expr))
    {
        // replace array self references

        m_array_ref_sub[nested_arr] = arr;

        reduce(nested_arr);
        arr->expr = nested_arr->expr;

        m_array_ref_sub.erase(nested_arr);

        // combine array vars

        arr->vars.insert(arr->vars.end(),
                         nested_arr->vars.begin(),
                         nested_arr->vars.end());
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
    else if(auto arr_self = dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        arr = arr_self->arr;
        is_self = true;
    }
    else if (auto nested_app = dynamic_pointer_cast<array_app>(app->object.expr))
    {
        // Despite eta expansion above, this is possible
        // in case of array self-reference.
        vector<expr_ptr> args(app->args.begin(), app->args.end());
        return apply(nested_app, args);
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
                printer p;
                cout << "Unbounded argument: ";
                p.print(arg, cout); cout << endl;
                throw source_error("Unbounded argument to"
                                   " bounded array dimension.",
                                   arg->location);
            }
        }

        // arg = make_shared<affine_expr>(lin_arg);
    }

    if (!is_self)
    {
        vector<expr_ptr> args(app->args.begin(), app->args.end());
        return apply(arr, args);
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
        auto cint = dynamic_pointer_cast<constant<int>>(as->dimension.expr);
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
    vector<bool> operand_is_array;

    for (auto & operand : op->operands)
    {
        if (auto arr = dynamic_pointer_cast<array>(operand.expr))
        {
            if (arr->is_recursive)
                throw source_error("Recursive arrays not supported as operands.",
                                   arr->location);
        }

        auto operand_size = array_size(operand);

        if (!operand_size.empty())
        {
            if (!result_size.empty() && operand_size != result_size)
            {
                throw source_error("Operand size mismatch.", op->location);
            }
            result_size = operand_size;
        }

        operand_is_array.push_back(!operand_size.empty());
    }

    if (!result_size.empty())
    {
        auto arr = make_shared<array>();

        // Create vars for result array

        decl_var_stacker decl_vars(m_declared_vars);

        for (auto s : result_size)
        {
            expr_ptr range = nullptr;
            if (s != array_var::unconstrained)
                range = make_shared<constant<int>>(s);
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);

            decl_vars.push(v);
        }

        // Reduce arrays in operands:

        for (int i = 0; i < (int)op->operands.size(); ++i)
        {
            if (!operand_is_array[i])
                continue;

            auto & operand = op->operands[i];
            vector<expr_ptr> args;
            for (auto & v : arr->vars)
                args.push_back(make_shared<reference>(v, location_type()));

            operand = apply(operand, args);
        }

        // Check subdomains in operands:

        for (auto & operand : op->operands)
        {
            if (dynamic_pointer_cast<case_expr>(operand.expr))
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

        decl_var_stacker decl_vars(m_declared_vars);

        for (auto s : common_size)
        {
            expr_ptr range = nullptr;
            if (s != array_var::unconstrained)
                range = make_shared<constant<int>>(s);
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);
            decl_vars.push(v);
        }

        for (auto & a_case : cexpr->cases)
        {
            auto & expr = a_case.second;

            // Reduce array
            vector<expr_ptr> args;
            for (auto & v : arr->vars)
                args.push_back(make_shared<reference>(v, location_type()));

            expr = apply(expr, args);

            if (dynamic_pointer_cast<case_expr>(expr.expr))
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

expr_ptr array_reducer::reduce(std::shared_ptr<reference> ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        process(id);

        // Is there a substitution for references to this id?
        auto sub_it = m_id_sub.find(id);
        if (sub_it != m_id_sub.end())
        {
            auto sub = m_copier.copy(sub_it->second);

            if (verbose<functional::model>::enabled())
            {
                cout << "Substituting expanded id: "
                     << id << " -> "; m_printer.print(sub, cout);
                cout << endl;
            }

            // Reduce to detect free vars
            return reduce(sub);
        }

        bool is_const = is_constant(id->expr);
        if (is_const)
            return id->expr;

        if (verbose<functional::model>::enabled())
        {
            cout << "Storing final id: " << id << endl;
        }
        m_final_ids.insert(id);
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
            if (verbose<functional::model>::enabled())
            {
                cout << "Free var: " << var << endl;
            }
        }
    }
    return ref;
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
            auto c = dynamic_pointer_cast<constant<int>>(var->range.expr);
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

        {
            context_type::scope_holder scope(m_context);

            for (int i = 0; i < args.size(); ++i)
            {
                m_context.bind(arr->vars[i], args[i]);
            }

            expr = substitute(arr->expr);
        }

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
    else
    {
        throw error("Unexpected object of array application.");
    }
}

expr_ptr array_reducer::substitute(expr_ptr expr)
{
    printer p;
    p.set_print_scopes(false);
    p.set_print_var_address(true);

    if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
        {
            auto binding = m_context.find(avar);
            if (binding)
            {
                if (verbose<functional::model>::enabled())
                {
                    cout << "Substituting array var " << avar << " with ";
                    p.print(binding.value(), cout);
                    cout << endl;
                }

                return binding.value();
            }
            else if (verbose<functional::model>::enabled())
            {
                cout << "No substitution for array var " << avar << endl;
            }
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
    auto arr = dynamic_pointer_cast<array>(id->expr.expr);
    if (!arr)
        return ref;

    auto new_arr = make_shared<array>();
    auto new_app = make_shared<array_app>();

    for (auto & var : arr->vars)
    {
        assert( !var->range || dynamic_pointer_cast<constant<int>>(var->range.expr) );
        auto new_var = make_shared<array_var>(*var);
        new_arr->vars.push_back(new_var);
        new_app->args.emplace_back(make_shared<reference>(new_var, location_type()));
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
