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

expr_ptr make_ref(array_var_ptr var)
{
    return make_shared<reference>(var, location_type(), make_int_type());
}

expr_ptr make_ref(id_ptr id)
{
    return make_shared<reference>(id, location_type(),
                                  id->expr ? id->expr->type : nullptr);
}

expr_ptr unite(expr_ptr a, expr_ptr b)
{
    if (!a)
        return b;
    if (!b)
        return a;
    auto r = make_shared<primitive>(primitive_op::logic_or, a, b);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr intersect(expr_ptr a, expr_ptr b)
{
    if (!a)
        return b;
    if (!b)
        return a;
    auto r = make_shared<primitive>(primitive_op::logic_and, a, b);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr negate(expr_ptr a)
{
    if (!a)
        return nullptr;
    auto r = make_shared<primitive>(primitive_op::negate, a);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr equal(expr_ptr a, expr_ptr b)
{
    auto r = make_shared<primitive>(primitive_op::compare_eq, a, b);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr int_expr(int v)
{
    auto r = make_shared<int_const>(v);
    r->type = make_int_type();
    return r;
}

array_reducer::array_reducer(name_provider & nmp):
    m_declared_vars("declared var"),
    m_name_provider(nmp),
    m_copier(m_processed_ids, nmp),
    m_sub(m_copier, m_printer)
{
    m_declared_vars.set_enabled(verbose<array_reducer>::enabled());

    m_printer.set_print_scopes(false);
    m_printer.set_print_var_address(true);
}

id_ptr array_reducer::process(id_ptr id)
{
    bool was_processed = m_processed_ids.find(id) != m_processed_ids.end();
    if (was_processed)
        return id;

    if (verbose<array_reducer>::enabled())
    {
        cout << "Processing id: ";
        m_printer.print(id, cout);
        cout << endl;
    }

    m_processed_ids.insert(id);

    m_declared_vars.push(nullptr);
    m_unbound_vars.emplace();

    id->expr = eta_expand(reduce(id->expr));

    if (verbose<array_reducer>::enabled())
    {
        cout << "Processed id: ";
        m_printer.print(id, cout);
        cout << endl;
    }

    auto & unbound_vars = m_unbound_vars.top();

    if (!unbound_vars.empty())
    {
        // Expand this id's expression to array

        vector<array_var_ptr> ordered_unbound_vars;

        for (auto & var : m_declared_vars)
        {
            if (var == nullptr)
                continue;
            if (unbound_vars.find(var) != unbound_vars.end())
            {
                if (verbose<array_reducer>::enabled())
                    cout << "Id has free var: " << var << endl;

                ordered_unbound_vars.push_back(var);
            }
        }

        assert(ordered_unbound_vars.size() == unbound_vars.size());

        m_sub.vars.enter_scope();

        // Make new vars and args for self reference

        vector<array_var_ptr> new_vars;
        vector<expr_ptr> new_args;

        for (auto & var : ordered_unbound_vars)
        {
            auto name = "i" + to_string(new_vars.size());
            auto new_var =
                    make_shared<array_var>(name, var->range, location_type());
            new_vars.push_back(new_var);

            auto ref = make_shared<reference>(new_var, location_type(), make_int_type());
            m_sub.vars.bind(var, ref);

            new_args.push_back(make_ref(new_var));
        }

        // Make the array

        auto ar = make_shared<array>();
        ar->vars = new_vars;
        ar->expr = id->expr;

        auto ar_size = array_size(ar);
        ar->type = make_shared<array_type>(ar_size, ar->expr->type);

        // Make new id

        auto new_id_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_id_name, ar, id->location);

        // Make new self ref

        {
            expr_ptr self_ref = make_shared<reference>(new_id, location_type(), ar->type);
            self_ref = apply(self_ref, new_args);
            m_sub.vars.bind(id, self_ref);
        }

        // Substitute vars and self references in new id

        new_id->expr = m_sub(new_id->expr);

        m_sub.vars.exit_scope();

        // Reduce new id to get rid of nested arrays

        m_processed_ids.insert(new_id); // Mark before reducing to avoid recursion

        new_id->expr = reduce(new_id->expr);

        // Expansion done

        if (verbose<array_reducer>::enabled())
        {
            cout << "Expanded id: " << id << " => ";
            m_printer.print(new_id, cout);
            cout << " : " << *new_id->expr->type << endl;
        }

        // Make substite for references to this id

        if (verbose<array_reducer>::enabled())
            cout << "Creating substitute for id: " << id << endl;

        expr_ptr sub = make_shared<reference>(new_id, location_type(), new_id->expr->type);

        vector<expr_ptr> args;
        for (auto & var : ordered_unbound_vars)
        {
            auto ref = make_shared<reference>(var, location_type(), make_int_type());
            args.push_back(ref);
            if (verbose<array_reducer>::enabled())
                cout << "Adding var to substitute: " << var << endl;
        }
        sub = apply(sub, args);

        m_id_sub.emplace(id, sub);

        if (verbose<array_reducer>::enabled())
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
    else if (auto op = dynamic_pointer_cast<operation>(expr))
    {
        return reduce(op);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        return reduce(ref);
    }

    return expr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array> arr)
{
    for (auto & var : arr->vars)
    {
        var->range = reduce(var->range);
    }

    decl_var_stacker declared_vars(m_declared_vars);

    for (auto & var : arr->vars)
    {
        declared_vars.push(var);
    }

    if (auto patterns = dynamic_pointer_cast<array_patterns>(arr->expr.expr))
    {
        arr->expr = reduce(arr, patterns);
    }

    arr->expr = eta_expand(reduce(arr->expr));

    if (auto nested_arr = dynamic_pointer_cast<array>(arr->expr.expr))
    {
        // Replace array self references

        m_sub.arrays[nested_arr] = arr;

        nested_arr->expr = m_sub(nested_arr->expr);

        m_sub.arrays.erase(nested_arr);

        // Reduce nested applications of self references

        reduce(nested_arr);

        // Merge nested array expression and vars

        arr->expr = nested_arr->expr;

        int d = arr->vars.size();
        for (auto & var : nested_arr->vars)
        {
            var->name = "i" + to_string(d);
            arr->vars.push_back(var);
        }

        arr->is_recursive |= nested_arr->is_recursive;
    }

    return arr;
}

expr_ptr array_reducer::reduce
(std::shared_ptr<array> ar,
 std::shared_ptr<array_patterns> ap)
{
    auto subdomains = make_shared<case_expr>();
    subdomains->type = ap->type;

    array_ref_sub::var_map::scope_holder scope(m_sub.vars);

    expr_ptr previous_domain;

    for (auto & pattern : ap->patterns)
    {
        if (verbose<array_reducer>::enabled())
            cout << "Processing an array pattern..." << endl;

        // Create pattern constraint expressions

        expr_ptr pattern_constraint;

        {
            int dim_idx = 0;
            for (auto & index : pattern.indexes)
            {
                if (!index.is_fixed)
                    continue;

                auto v = make_ref(ar->vars[dim_idx]);
                auto constraint = equal(v, int_expr(index.value));
                pattern_constraint =
                        intersect(pattern_constraint, constraint);

                ++dim_idx;
            }
        }

        // Create subdomain expressions

        expr_ptr pattern_domain =
                intersect(negate(previous_domain), pattern_constraint);

        expr_ptr prev_case_domain;

        if (pattern.domains)
        {
            auto cexpr = dynamic_pointer_cast<case_expr>(pattern.domains.expr);
            for (auto & c : cexpr->cases)
            {
                auto dom = c.first;
                dom = intersect(negate(prev_case_domain), dom);
                dom = intersect(pattern_domain, dom);
                prev_case_domain = unite(prev_case_domain, c.first);
                c.first = dom;
                subdomains->cases.push_back(c);
            }
        }

        auto last_case = intersect(pattern_domain, negate(prev_case_domain));
        subdomains->cases.emplace_back(expr_slot(last_case), pattern.expr);

        previous_domain = unite(previous_domain, pattern_constraint);
    }

    // Subtitute vars in local ids

    if (verbose<array_reducer>::enabled())
        cout << "Substituting vars in local ids..." << endl;
    for (auto & id : ar->scope.ids)
    {
        if (verbose<array_reducer>::enabled())
            cout << ".. Local id: " << id << endl;
        id->expr = m_sub(id->expr);
    }
    if (verbose<array_reducer>::enabled())
        cout << ".. Done substituting vars in local ids." << endl;

    // If only one subdomain, and it has no constraints,
    // return its expression.

    if (subdomains->cases.size() == 1)
    {
        auto & c = subdomains->cases.front();
        auto & domain = c.first;
        auto & expr = c.second;
        if (!domain)
            return expr;
    }

    return subdomains;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array_app> app)
{
    app->object = reduce(app->object);

    for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];
        arg = reduce(arg);
    }

    bool requires_reduction = true;

    if (auto arr = dynamic_pointer_cast<array>(app->object.expr))
    {
        // TODO: Are name recursions in array OK?
        // I think so.
        // Any problem will show up as scheduling problem due to circular deps.

        bool is_irreducible =
                arr->is_recursive ||
                dynamic_pointer_cast<case_expr>(arr->expr.expr);

        if (is_irreducible)
        {
            auto name = m_name_provider.new_name("tmp");
            auto id = make_shared<identifier>(name, arr, location_type());

            auto ref = make_ref(id);
            // Process id and detect free vars in substitution,
            // by reducing the reference:
            ref = reduce(ref);

            app->object = ref;
        }

        requires_reduction = true;
    }
    else if (dynamic_pointer_cast<array_app>(app->object.expr))
    {
        requires_reduction = true;
    }
    else if (dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        requires_reduction = false;
    }
    else
    {
        auto object_ar = dynamic_pointer_cast<array_type>(app->object->type);
        if (object_ar && object_ar->size.size() >= app->args.size())
            requires_reduction = false;
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

    // Lambda-lift operands if necessary

    for (auto & operand : op->operands)
    {
        // NOTE:
        // Pointwise operation is valid for an array with name recursion,
        // since the recursion refers to the resulting array.
        if (auto arr = dynamic_pointer_cast<array>(operand.expr))
        {
            bool is_irreducible =
                    arr->is_recursive ||
                    dynamic_pointer_cast<case_expr>(arr->expr.expr);

            if (is_irreducible)
            {
                auto name = m_name_provider.new_name("tmp");
                auto id = make_shared<identifier>(name, arr, location_type());

                auto ref = make_ref(id);
                // Process id and detect free vars in substitution,
                // by reducing the reference:
                ref = reduce(ref);

                operand = ref;
            }
        }
    }

    auto arr = make_shared<array>();
    arr->type = result_arr_type;

    // Create vars for result array

    decl_var_stacker decl_vars(m_declared_vars);

    for (auto dim_size : result_arr_type->size)
    {
        expr_ptr range = nullptr;
        if (dim_size != array_var::unconstrained)
            range = make_shared<int_const>(dim_size, location_type(), make_int_type());
        auto v = make_shared<array_var>(new_var_name(), range, location_type());
        arr->vars.push_back(v);
        decl_vars.push(v);
    }

    // Reduce arrays in operands:

    for (auto & operand : op->operands)
    {
        if (!operand->type->is_array())
            continue;

        auto args = broadcast(arr->vars, operand->type->array()->size);

        operand = apply(operand, args);
    }

    arr->expr = reduce_primitive(op);
    arr->expr->type = make_shared<scalar_type>(result_arr_type->element);

    return arr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<operation> op)
{
    for (auto & operand : op->operands)
    {
        operand = reduce(operand);
    }

    auto result_arr_type = dynamic_pointer_cast<array_type>(op->type);
    assert(result_arr_type);

    auto arr = make_shared<array>();
    arr->type = result_arr_type;

    // Create vars for result array

    decl_var_stacker decl_vars(m_declared_vars);

    for (auto dim_size : result_arr_type->size)
    {
        expr_ptr range = nullptr;
        if (dim_size != array_var::unconstrained)
            range = make_shared<int_const>(dim_size, location_type(), make_int_type());
        auto v = make_shared<array_var>(new_var_name(), range, location_type());
        arr->vars.push_back(v);
        decl_vars.push(v);
    }

    assert(arr->vars.size());

    auto domains = make_shared<case_expr>();
    domains->type = make_shared<scalar_type>(result_arr_type->element);
    arr->expr = domains;

    int current_index = 0;

    bool has_nested_cases = false;

    for (auto & operand : op->operands)
    {
        auto ar_type = dynamic_pointer_cast<array_type>(operand->type);

        // Find out size of this elem in first result dimension

        int current_size;
        switch(op->kind)
        {
        case operation::array_enumerate:
        {
            current_size = 1;
            break;
        }
        case operation::array_concatenate:
        {
            if (ar_type)
                current_size = ar_type->size.front();
            else
                current_size = 1;
            break;
        }
        default:
            throw error("Unexpected operation type.");
        }

        // Create conditional expression for bounds

        expr_ptr bounds;
        if (current_size == 1)
        {
            bounds = make_shared<primitive>
                    (primitive_op::compare_eq,
                     make_shared<reference>(arr->vars.front()),
                     make_shared<int_const>(current_index));
        }
        else
        {
            expr_ptr lb =
                    make_shared<primitive>
                    (primitive_op::compare_geq,
                     make_shared<reference>(arr->vars.front()),
                     make_shared<int_const>(current_index));
            if (current_size == -1)
            {
                bounds = lb;
            }
            else
            {
                expr_ptr ub =
                        make_shared<primitive>
                        (primitive_op::compare_leq,
                         make_shared<reference>(arr->vars.front()),
                         make_shared<int_const>(current_index + current_size - 1));
                bounds = make_shared<primitive>
                        (primitive_op::logic_and, lb, ub);
            }
        }

        // Expand recursive applications

        auto arr_op = dynamic_pointer_cast<array>(operand.expr);
        if (arr_op && arr_op->is_recursive)
        {
            switch(op->kind)
            {
            case operation::array_enumerate:
            {
                vector<expr_ptr> args = { make_ref(arr->vars[0]) };
                expr_ptr self = make_shared<array_self_ref>(arr, location_type(), arr->type);
                self = apply(self, args);

                m_sub.array_recursions[arr_op] = self;
                operand = m_sub(operand);
                m_sub.array_recursions.erase(arr_op);

                operand = reduce(operand);

                break;
            }
            case operation::array_concatenate:
            {
                expr_ptr self = make_shared<array_self_ref>(arr, location_type(), arr->type);

                auto v = make_shared<array_var>
                        (new_var_name(), arr_op->vars[0]->range,
                        location_type());

                expr_ptr arg = make_shared<primitive>
                        (primitive_op::add,
                         make_ref(v), int_expr(current_index));
                arg->type = make_int_type();

                vector<expr_ptr> args = { arg };

                self = apply(self, args);

                auto arr = make_shared<array>();
                arr->vars.push_back(v);
                arr->expr = self;
                arr->type = arr_op->type;

                m_sub.array_recursions[arr_op] = arr;
                operand = m_sub(operand);
                m_sub.array_recursions.erase(arr_op);

                break;
            }
            default:
                throw error("Unexpected operation type.");
            }
        }

        // Substitute vars in array operand

        if (ar_type)
        {
            vector<expr_ptr> args;

            switch(op->kind)
            {
            case operation::array_enumerate:
            {
                vector<array_var_ptr> vars(arr->vars.begin()+1, arr->vars.end());
                args = broadcast(vars, ar_type->size);
                break;
            }
            case operation::array_concatenate:
            {
                args = broadcast(arr->vars, ar_type->size);

                // Override first arg with special treatment

                expr_ptr first_arg = make_shared<reference>
                        (arr->vars.front(), location_type(), make_int_type());
                if (current_index > 0)
                {
                    auto offset =
                            make_shared<int_const >
                            (current_index, location_type(), make_int_type());

                    first_arg = make_shared<primitive>
                            (primitive_op::subtract, first_arg, offset);
                    first_arg->type = make_int_type();
                }

                args[0] = first_arg;

                break;
            }
            default:
                throw error("Unexpected operation type.");
            }

            operand = apply(operand, args);
        }

        if (dynamic_pointer_cast<case_expr>(operand.expr))
        {
            has_nested_cases = true;
        }

        // Store as a case

        domains->cases.emplace_back(expr_slot(bounds), operand);

        current_index += current_size;
    }

    if (has_nested_cases)
    {
        arr->expr = reduce(arr->expr);
    }

    return arr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<case_expr> cexpr)
{
    for (auto & a_case : cexpr->cases)
    {
        auto & expr = a_case.second;
        expr = reduce(expr);
    }

    array_ptr arr;
    auto result_arr_type = dynamic_pointer_cast<array_type>(cexpr->type);
    if (result_arr_type)
    {
        arr = make_shared<array>();
        arr->type = result_arr_type;

        decl_var_stacker decl_vars(m_declared_vars);

        for (auto dim_size : result_arr_type->size)
        {
            expr_ptr range = nullptr;
            if (dim_size != array_var::unconstrained)
                range = make_shared<int_const>(dim_size, location_type(), make_int_type());
            auto v = make_shared<array_var>(new_var_name(), range, location_type());
            arr->vars.push_back(v);
            decl_vars.push(v);
        }

        for (auto & c : cexpr->cases)
        {
            auto & expr = c.second;

            if (!expr->type->is_array())
                continue;

            // Update array recursions

            if (auto nested_ar = dynamic_pointer_cast<array>(expr.expr))
            {
                arr->is_recursive |= nested_ar->is_recursive;

                auto self = make_shared<array_self_ref>(arr, location_type(), result_arr_type);

                m_sub.array_recursions[nested_ar] = self;
                expr = m_sub(expr);
                m_sub.array_recursions.erase(nested_ar);
            }

            // Replace array variables

            auto args = broadcast(arr->vars, expr->type->array()->size);

            expr = apply(expr, args);
        }
    }

    // Merge nested cases

    auto new_case_expr = make_shared<case_expr>();

    for (auto & c : cexpr->cases)
    {
        auto & domain = c.first;
        auto & expr = c.second;

        if (auto nested_case_expr = dynamic_pointer_cast<case_expr>(expr.expr))
        {
            for (auto & c2 : nested_case_expr->cases)
            {
                auto merged_domain = intersect(domain, c2.first);
                new_case_expr->cases.emplace_back(expr_slot(merged_domain), c2.second);
            }
        }
        else
        {
            new_case_expr->cases.push_back(c);
        }
    }

    if (arr)
    {
        new_case_expr->type = make_shared<scalar_type>(result_arr_type->element);
        arr->expr = new_case_expr;
        return arr;
    }
    else
    {
        new_case_expr->type = cexpr->type;
        return new_case_expr;
    }
}

expr_ptr array_reducer::reduce(std::shared_ptr<reference> ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (ref->is_recursion)
            return ref;

        process(id);

        // Is there a substitution for references to this id?
        auto sub_it = m_id_sub.find(id);
        if (sub_it != m_id_sub.end())
        {
            auto sub = m_copier.copy(sub_it->second);

            if (verbose<array_reducer>::enabled())
            {
                cout << "Substituting expanded id: "
                     << id << " -> "; m_printer.print(sub, cout);
                cout << endl;
            }

            // Reduce to detect free vars
            return reduce(sub);
        }

        if (verbose<array_reducer>::enabled())
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
            if (verbose<array_reducer>::enabled())
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
        if (auto c = dynamic_pointer_cast<int_const>(var->range.expr))
        {
            s.push_back(c->value);
        }
        else
        {
            s.push_back(array_var::unconstrained);
        }
    }
    return s;
}

vector<expr_ptr>
array_reducer::broadcast(const vector<array_var_ptr> & vars, const array_size_vec & size)
{
    assert(size.size() <= vars.size());

    vector<expr_ptr> args;
    args.reserve(size.size());

    for (int d = 0; d < size.size(); ++d)
    {
        if (size[d] == 1)
            args.push_back(make_shared<int_const>(0));
        else
            args.push_back(make_shared<reference>(vars[d], location_type(), make_int_type()));
    }

    return args;
}

expr_ptr array_reducer::apply(expr_ptr expr, const vector<expr_ptr> & given_args)
{
    assert(expr->type);

    if (verbose<array_reducer>::enabled())
        cout << "Applied type: " << *expr->type << endl;

    auto ar_type = dynamic_pointer_cast<array_type>(expr->type);

    if (!ar_type)
        return expr;

    int used_arg_count = min(given_args.size(), ar_type->size.size());
    vector<expr_ptr> args(given_args.begin(),
                          given_args.begin() + used_arg_count);

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

    if (verbose<array_reducer>::enabled())
        cout << "=> Application result type: " << *result_type << endl;

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
            array_ref_sub::var_map::scope_holder scope(m_sub.vars);

            for (int i = 0; i < args.size(); ++i)
            {
                m_sub.vars.bind(arr->vars[i], args[i]);
            }

            result = m_sub(arr->expr);
        }

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

        // Reduce primitive ops
        result = reduce(result);

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
            range = make_shared<int_const>(dim_size, location_type(), make_int_type());
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

expr_ptr array_ref_sub::visit_ref(const shared_ptr<reference> & ref)
{
    auto binding = vars.find(ref->var);
    if (binding)
    {
        if (verbose<array_reducer>::enabled())
        {
            cout << "Substituting var " << ref->var << " with ";
            m_printer.print(binding.value(), cout);
            cout << endl;
        }

        return m_copier.copy(binding.value());
    }
    else if (verbose<array_reducer>::enabled())
    {
        cout << "No substitution for var " << ref->var << endl;
    }

    return ref;
}

expr_ptr array_ref_sub::visit_array_self_ref(const shared_ptr<array_self_ref> & self)
{
    auto sub_it = arrays.find(self->arr);

    if (sub_it != arrays.end())
    {
        array_ptr sub = sub_it->second;

        auto new_self = make_shared<array_self_ref>(sub, location_type(), sub->type);

        auto app = make_shared<array_app>();
        app->object = new_self;
        app->type = self->type;
        for (auto & var : sub->vars)
            app->args.emplace_back(make_shared<reference>(var, location_type(), make_int_type()));

        if (verbose<array_reducer>::enabled())
        {
            cout << "Substituting array self reference: ";
            cout << self->arr;
            cout << " : " << *self->type << " -> ";
            m_printer.print(app, cout);
            cout << endl;
        }

        return app;
    }

    auto self_sub_it = array_recursions.find(self->arr);
    if (self_sub_it != array_recursions.end())
    {
        auto & sub = self_sub_it->second;
        if (verbose<array_reducer>::enabled())
        {
            cout << "Substituting array self reference: "
                 << self->arr << " : " << *self->type << " -> ";
            m_printer.print(sub, cout);
            cout << endl;
        }

        return m_copier.copy(sub);
    }

    return self;
}

}
}
