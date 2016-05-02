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
    bool was_processed = m_processed_ids.find(id) != m_processed_ids.end();
    if (was_processed)
        return id;

    if (verbose<functional::model>::enabled())
    {
        cout << "Processing id: ";
        m_printer.print(id, cout);
        cout << endl;
    }

    m_processed_ids.insert(id);

    m_declared_vars.push(nullptr);
    m_unbound_vars.emplace();

    id->expr = eta_expand(reduce(id->expr));

    if (verbose<functional::model>::enabled())
    {
        cout << "Processed id: ";
        m_printer.print(id, cout);
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

            auto ref = make_shared<reference>(new_var, location_type(), make_int_type());
            m_context.bind(var, ref);
        }

        auto ar = make_shared<array>();
        ar->vars = new_vars;
        ar->expr = substitute(id->expr);

        auto ar_size = array_size(ar);
        ar->type = make_shared<array_type>(ar_size, ar->expr->type);

        m_context.exit_scope();

        // Reduce expression to get rid of nested arrays

        auto expr = reduce(ar);

        // Make new id

        auto new_id_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_id_name, expr, id->location);
        m_processed_ids.insert(new_id);

        if (verbose<functional::model>::enabled())
        {
            cout << "Expanded id: " << id << " => ";
            m_printer.print(new_id, cout);
            cout << " : " << *new_id->expr->type << endl;
        }

        // Substite for references to this id

        if (verbose<functional::model>::enabled())
            cout << "Creating substitute for id: " << id << endl;

        expr_ptr sub = make_shared<reference>(new_id, location_type(), expr->type);

        vector<expr_ptr> args;
        for (auto & var : ordered_unbound_vars)
        {
            auto ref = make_shared<reference>(var, location_type(), make_int_type());
            args.push_back(ref);
            if (verbose<functional::model>::enabled())
                cout << "Adding var to substitute: " << var << endl;
        }
        sub = apply(sub, args);

        m_id_sub.emplace(id, sub);

        if (verbose<functional::model>::enabled())
        {
            cout << "Expanded id " << id << " to ";
            m_printer.print(new_id, cout);
            cout << " : " << *new_id->expr->type;
            cout << endl;

            cout << "Substitute = ";
            m_printer.print(sub, cout);
            cout << " : " << *sub->type;
            cout << endl;
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
        throw error("Unexpected.");
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

        if (verbose<functional::model>::enabled())
        {
            cout << "Substituting array self reference: "
                 << self->arr << " : " << *self->type
                 << " -> "
                 << sub << " : " << *sub->type
                 << endl;
        }

        auto original_type = self->type;

        self->arr = sub;
        self->type = sub->type;

        auto app = make_shared<array_app>();
        app->object = self;
        app->type = original_type;
        for (auto & var : sub->vars)
            app->args.emplace_back(make_shared<reference>(var, location_type(), make_int_type()));

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

        arr->is_recursive = nested_arr->is_recursive;
    }

    return arr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array_app> app)
{
    app->object = reduce(app->object);

    for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];
        arg = reduce(arg);
    }

    bool requires_reduction = false;

    if (auto arr = dynamic_pointer_cast<array>(app->object.expr))
    {
        if (arr->is_recursive)
            throw source_error("Application of recursive arrays not supported.",
                               app->location);
        if (dynamic_pointer_cast<case_expr>(arr->expr.expr))
            throw source_error("Application of arrays with subdomains not supported.",
                               app->location);

        requires_reduction = true;
    }
    else if (dynamic_pointer_cast<array_app>(app->object.expr))
    {
        requires_reduction = true;
    }

    if (!requires_reduction)
        return app;

    vector<expr_ptr> args(app->args.begin(), app->args.end());
    return apply(app->object, args);
}

expr_ptr array_reducer::reduce(std::shared_ptr<primitive> op)
{
    for (auto & operand : op->operands)
        operand = reduce(operand);

    auto result_arr_type = dynamic_pointer_cast<array_type>(op->type);
    if (!result_arr_type)
        return reduce_primitive(op);

    for (auto & operand : op->operands)
    {
        if (auto arr = dynamic_pointer_cast<array>(operand.expr))
        {
            if (arr->is_recursive)
                throw source_error("Recursive arrays not supported as operands.",
                                   arr->location);
        }
    }

    auto arr = make_shared<array>();

    // Create vars for result array

    decl_var_stacker decl_vars(m_declared_vars);

    for (auto dim_size : result_arr_type->size)
    {
        expr_ptr range = nullptr;
        if (dim_size != array_var::unconstrained)
            range = make_shared<constant<int>>(dim_size, location_type(), make_int_type());
        auto v = make_shared<array_var>(new_var_name(), range, location_type());
        arr->vars.push_back(v);
        decl_vars.push(v);
    }

    // Reduce arrays in operands:

    for (auto & operand : op->operands)
    {
        if (!operand->type->is_array())
            continue;

        vector<expr_ptr> args;
        for (auto & v : arr->vars)
            args.push_back(make_shared<reference>(v, location_type(), make_int_type()));

        operand = apply(operand, args);
    }

    // Check subdomains in operands:

    for (auto & operand : op->operands)
    {
        if (dynamic_pointer_cast<case_expr>(operand.expr))
        {
            throw source_error("Case expression not supported as operand.",
                               operand.location);
        }
    }

    arr->expr = reduce_primitive(op);

    arr->expr->type = make_shared<scalar_type>(result_arr_type->element);
    arr->type = result_arr_type;

    return arr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<case_expr> cexpr)
{
    bool is_recursive_array = false;

    for (auto & a_case : cexpr->cases)
    {
        auto & expr = a_case.second;

        expr = reduce(expr);

        if (auto arr = dynamic_pointer_cast<array>(expr.expr))
        {
            is_recursive_array |= arr->is_recursive;
        }
    }

    auto result_arr_type = dynamic_pointer_cast<array_type>(cexpr->type);
    if (!result_arr_type)
        return cexpr;

    auto arr = make_shared<array>();

    decl_var_stacker decl_vars(m_declared_vars);

    for (auto dim_size : result_arr_type->size)
    {
        expr_ptr range = nullptr;
        if (dim_size != array_var::unconstrained)
            range = make_shared<constant<int>>(dim_size, location_type(), make_int_type());
        auto v = make_shared<array_var>(new_var_name(), range, location_type());
        arr->vars.push_back(v);
        decl_vars.push(v);
    }

    for (auto & a_case : cexpr->cases)
    {
        auto & expr = a_case.second;
        if (!expr->type->is_array())
            continue;

        vector<expr_ptr> args;
        for (auto & v : arr->vars)
            args.push_back(make_shared<reference>(v, location_type(), make_int_type()));

        // FIXME: Update array recursions?
        // Although a recursion would usually appear inside a case,
        // which would be rejected just below.

        expr = apply(expr, args);
    }

    for (auto & a_case : cexpr->cases)
    {
        auto & expr = a_case.second;
        if (dynamic_pointer_cast<case_expr>(expr.expr))
        {
            throw source_error("Nested cases not supported.", expr.location);
        }
    }

    arr->expr = cexpr;

    arr->expr->type = make_shared<scalar_type>(result_arr_type->element);
    arr->type = result_arr_type;
    arr->is_recursive = is_recursive_array;

    return arr;
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
    if (verbose<functional::model>::enabled())
        cout << "Applying : " << *expr->type << endl;

    auto ar_type = dynamic_pointer_cast<array_type>(expr->type);
    assert(ar_type);

    type_ptr result_type;
    {
        int remaining_var_size = ar_type->size.size() - args.size();
        assert(remaining_var_size >= 0);
        if (remaining_var_size > 0)
        {
            array_size_vec result_size
                    (ar_type->size.begin() + args.size(),
                     ar_type->size.end());
            result_type = make_shared<array_type>(result_size, ar_type->element);
        }
        else
        {
            result_type = make_shared<scalar_type>(ar_type->element);
        }
    }

    if (verbose<functional::model>::enabled())
        cout << "Applying : => " << *result_type << endl;

    if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        app->args.insert(app->args.end(), args.begin(), args.end());
        app->type = result_type;
        return app;
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        assert(arr->vars.size() >= args.size());

        expr_ptr result;

        {
            context_type::scope_holder scope(m_context);

            for (int i = 0; i < args.size(); ++i)
            {
                m_context.bind(arr->vars[i], args[i]);
            }

            result = substitute(arr->expr);
        }

        // Reduce primitive ops
        result = reduce(result);

        if (arr->vars.size() > args.size())
        {
            vector<array_var_ptr> remaining_vars(arr->vars.begin() + args.size(),
                                                 arr->vars.end());
            arr->vars = remaining_vars;
            arr->expr = result;
            arr->location = location_type();

            result = arr;
        }
        // FIXME: Else, update recursions of this array?

        result->type = result_type;

        return result;
    }
    else
    {
        auto app = make_shared<array_app>();
        app->object = expr;
        for (auto & arg : args)
            app->args.emplace_back(arg);
        app->type = result_type;
        return app;
        //throw error("Unexpected object of array application.");
    }
}

expr_ptr array_reducer::substitute(expr_ptr expr)
{
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
                    m_printer.print(binding.value(), cout);
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
    auto ar_type = dynamic_pointer_cast<array_type>(expr->type);
    if (!ar_type)
        return expr;

    bool is_recursive_array = false;

    if (auto ar = dynamic_pointer_cast<array>(expr))
    {
        if (ar->vars.size() == ar_type->size.size())
        {
            // Assume it is fully expanded.
            return expr;
        }

        is_recursive_array = ar->is_recursive;
    }

    auto new_ar = make_shared<array>();
    vector<expr_ptr> args;
    for (int dim_size : ar_type->size)
    {
        expr_ptr range;
        if (dim_size != array_var::unconstrained)
            range = make_shared<constant<int>>(dim_size, location_type(), make_int_type());
        auto var = make_shared<array_var>(new_var_name(), range, location_type());
        new_ar->vars.push_back(var);
        args.push_back(make_shared<reference>(var, location_type(), make_int_type()));
    }

    new_ar->expr = apply(expr, args);
    new_ar->type = ar_type;
    new_ar->is_recursive = is_recursive_array;

    return new_ar;
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
