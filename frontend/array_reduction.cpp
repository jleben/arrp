#include "array_reduction.hpp"
#include "array_inflate.hpp"
#include "linear_expr_gen.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"
#include "../utility/debug.hpp"
#include "../common/func_model_printer.hpp"

#include <sstream>

/*
Normal form of expression
(omitting syntax elements that are not expressions).

Note: An expression may not be fully eta-expanded,
it will be expanded when assigned to an id though.

normal =
  simple | ref | array_app | array
  | external | input

primitive = simple*

array_app = ref : array_t

array =  simple | case | external

case = (simple | external : scalar_t)*

simple =
  ref : scalar_t |
  literal : scalar_t |
  primitive : scalar_t |
  array_app : scalar_t |
  external : scalar_t
*/

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

expr_ptr make_int(int v)
{
    return make_signed_int(v);
}

expr_ptr empty_set()
{
    return make_shared<bool_const>(false);
}

expr_ptr universe_set()
{
    return make_shared<bool_const>(true);
}

bool is_empty_set(expr_ptr e)
{
    auto b = dynamic_pointer_cast<bool_const>(e);
    return bool(b) && b->value == false;
}

bool is_universe_set(expr_ptr e)
{
    auto b = dynamic_pointer_cast<bool_const>(e);
    return bool(b) && b->value == true;
}

expr_ptr unite(expr_ptr a, expr_ptr b)
{
    if (is_universe_set(a) || is_universe_set(b))
        return universe_set();
    if (is_empty_set(a))
        return b;
    if (is_empty_set(b))
        return a;
    auto r = make_shared<primitive>(primitive_op::logic_or, a, b);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr intersect(expr_ptr a, expr_ptr b)
{
    if (is_empty_set(a) || is_empty_set(b))
        return empty_set();
    if (is_universe_set(a))
        return b;
    if (is_universe_set(b))
        return a;
    auto r = make_shared<primitive>(primitive_op::logic_and, a, b);
    r->type = make_shared<scalar_type>(primitive_type::boolean);
    return r;
}

expr_ptr negate(expr_ptr a)
{
    if (is_universe_set(a))
        return empty_set();
    if (is_empty_set(a))
        return universe_set();
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

class free_array_vars : private visitor<void>
{
public:
    free_array_vars(expr_ptr e)
    {
        visit(e);
    }

    const unordered_set<array_var_ptr> & result() const { return m_free_vars; }

    void visit_array(const shared_ptr<stream::functional::array> & arr) override
    {
        for (auto & var : arr->vars)
            m_bound_vars.insert(var);

        visitor::visit_array(arr);

        for (auto & var : arr->vars)
            m_bound_vars.erase(var);
    }

    virtual void visit_ref(const shared_ptr<reference> & ref) override
    {
        if (auto v = dynamic_pointer_cast<array_var>(ref->var))
        {
            if (!m_bound_vars.count(v))
                m_free_vars.insert(v);
        }
    }

private:
    unordered_set<array_var_ptr> m_free_vars;
    unordered_set<array_var_ptr> m_bound_vars;
};

array_reducer::array_reducer(name_provider & nmp):
    m_name_provider(nmp),
    m_copier(m_additional_ids, nmp),
    m_sub(m_copier, m_printer)
{
    m_printer.set_print_var_address(true);
}

void array_reducer::process(scope & s)
{
    m_additional_ids.clear();

    for (auto & id : s.ids)
        process(id);

    for (auto & id : m_additional_ids)
        s.ids.push_back(id);
}

void array_reducer::process(id_ptr id)
{
    if (verbose<array_reducer>::enabled())
    {
        cout << "Processing id: ";
        m_printer.print(id, cout);
        cout << endl;
    }

    id->expr = eta_expand(reduce(id->expr));

    if (verbose<array_reducer>::enabled())
    {
        cout << "Processed id: ";
        m_printer.print(id, cout);
        cout << endl;
    }
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
    else if (auto app = dynamic_pointer_cast<func_app>(expr))
    {
        return reduce(app);
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

    return expr;
}

expr_ptr array_reducer::reduce(std::shared_ptr<array> arr)
{
    for (auto & var : arr->vars)
    {
        var->range = reduce(var->range);
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

    // Cg (global constraint)
    expr_ptr Cg = universe_set();

    for (auto & pattern : ap->patterns)
    {
        if (verbose<array_reducer>::enabled())
            cout << "Processing an array pattern..." << endl;

        // Create pattern constraint expressions

        // Cp (pattern constraint)
        expr_ptr Cp = universe_set();

        {
            int dim_idx = 0;
            for (auto & index : pattern.indexes)
            {
                if (verbose<array_reducer>::enabled())
                    cout << ".. Index: " << dim_idx << " fixed: " << index.is_fixed << endl;

                if (index.is_fixed)
                {
                    auto v = make_ref(ar->vars[dim_idx]);
                    auto constraint = equal(v, make_int(index.value));
                    Cp = intersect(Cp, constraint);
                }

                ++dim_idx;
            }
        }

        if (pattern.domains)
        {
            auto cexpr = dynamic_pointer_cast<case_expr>(pattern.domains.expr);
            for (auto & c : cexpr->cases)
            {
                // Cs (this sub-domain constraint)
                auto Cs = c.first;

                // Ce = Cp && Cs (expression contraint)
                auto Ce = intersect(Cp, Cs);

                // D = Cg && Ce (expression domain)
                auto D = intersect(Cg, Ce);
                c.first = D;

                subdomains->cases.push_back(c);

                Cg = intersect(Cg, negate(Ce));
            }
        }
#if 0
        if (pattern.expr)
        {
            auto Ce = Cp;

            auto D = intersect(Cg, Ce);
            subdomains->cases.emplace_back(expr_slot(D), pattern.expr);

            Cg = intersect(Cg, negate(Ce));
        }
#endif
    }

    // If only one subdomain, and it has no constraints,
    // return its expression.

    if (subdomains->cases.size() == 1)
    {
        auto & c = subdomains->cases.front();
        auto & domain = c.first;
        auto & expr = c.second;
        if (is_universe_set(domain))
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

    // Lambda lift if not a simple array

    bool must_lambda_lift = false;

    if (auto arr = dynamic_pointer_cast<array>(app->object.expr))
    {
        must_lambda_lift =
                        arr->is_recursive ||
                        dynamic_pointer_cast<case_expr>(arr->expr.expr) ||
                        dynamic_pointer_cast<func_app>(arr->expr.expr);
    }
    else if (dynamic_pointer_cast<func_app>(app->object.expr))
    {
        must_lambda_lift = true;
    }

    if (must_lambda_lift)
        app->object = lambda_lift(app->object, "_tmp");

    // Is there anything left to do?

    auto ar_type = dynamic_pointer_cast<array_type>(app->object->type);

    bool is_overapplied =
            !ar_type || ar_type->size.size() < app->args.size();

    bool is_normal =
            (dynamic_pointer_cast<array_self_ref>(app->object.expr)
             || dynamic_pointer_cast<reference>(app->object.expr))
            && !is_overapplied;

    if (is_normal)
        return app;

    // Reduce

    vector<expr_ptr> args(app->args.begin(), app->args.end());
    return apply(app->object, args);
}

expr_ptr array_reducer::reduce(std::shared_ptr<func_app> app)
{
    // At first pass, app->object can only be a reference to an external id.
    // We inline the external.

    if (auto ref = dynamic_pointer_cast<reference>(app->object.expr))
    {
        auto id = dynamic_pointer_cast<identifier>(ref->var);
        auto ext = dynamic_pointer_cast<external>(id->expr.expr);
        app->object = ext;
    }

    for(auto & arg : app->args)
    {
        arg = reduce(arg);

        // Lift complex arguments to external calls.
        if (dynamic_pointer_cast<array>(arg.expr))
        {
            arg = lambda_lift(arg, "_tmp");
        }
    }

    return app;
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
        bool must_lambda_lift = false;

        // NOTE:
        // Pointwise operation is valid for an array with name recursion,
        // since the recursion refers to the resulting array.
        if (auto arr = dynamic_pointer_cast<array>(operand.expr))
        {
            must_lambda_lift =
                    arr->is_recursive ||
                    dynamic_pointer_cast<case_expr>(arr->expr.expr) ||
                    (dynamic_pointer_cast<func_app>(arr->expr.expr) &&
                     arr->expr->type->is_array());
        }
        else if (dynamic_pointer_cast<func_app>(operand.expr) &&
                 operand->type->is_array())
        {
            must_lambda_lift = true;
        };

        if (must_lambda_lift)
            operand = lambda_lift(operand, "_tmp");
    }

    auto arr = make_shared<array>();
    arr->type = result_arr_type;

    // Create vars for result array

    arr->vars = make_array_vars(result_arr_type->size);

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

        bool must_lambda_lift = false;
        if (dynamic_pointer_cast<array>(operand.expr))
        {
            must_lambda_lift =
                    dynamic_pointer_cast<func_app>(operand.expr) &&
                    operand->type->is_array();
        }
        else if (dynamic_pointer_cast<func_app>(operand.expr))
        {
            must_lambda_lift = operand->type->is_array();
        }

        if (must_lambda_lift)
            operand = lambda_lift(operand, "_tmp");
    }

    auto result_arr_type = dynamic_pointer_cast<array_type>(op->type);
    assert(result_arr_type);

    auto arr = make_shared<array>();
    arr->type = result_arr_type;

    // Create vars for result array

    arr->vars = make_array_vars(result_arr_type->size);
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
                     make_int(current_index));
        }
        else
        {
            expr_ptr lb =
                    make_shared<primitive>
                    (primitive_op::compare_geq,
                     make_shared<reference>(arr->vars.front()),
                     make_int(current_index));
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
                         make_int(current_index + current_size - 1));
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
                        (arr_op->vars[0]->range,
                        location_type());

                expr_ptr arg = make_shared<primitive>
                        (primitive_op::add,
                         make_ref(v), make_int(current_index));
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
                    auto offset = make_int(current_index);

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
        auto & part = a_case.second;
        part = reduce(part);

        bool must_lambda_lift = false;

        if (dynamic_pointer_cast<array>(part.expr))
        {
            // FIXME: I think part.expr should be the body of array instead.
            must_lambda_lift =
                    dynamic_pointer_cast<func_app>(part.expr) &&
                    part->type->is_array();
        }
        else if (dynamic_pointer_cast<func_app>(part.expr))
        {
            must_lambda_lift = part->type->is_array();
        }

        if (must_lambda_lift)
            part = lambda_lift(part, "_tmp");
    }

    array_ptr arr;
    auto result_arr_type = dynamic_pointer_cast<array_type>(cexpr->type);
    if (result_arr_type)
    {
        arr = make_shared<array>();
        arr->type = result_arr_type;
        arr->vars = make_array_vars(result_arr_type->size);

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
            s.push_back(c->signed_value());
        }
        else
        {
            s.push_back(array_var::unconstrained);
        }
    }
    return s;
}

vector<array_var_ptr> array_reducer::make_array_vars(const array_size_vec & size)
{
    vector<array_var_ptr> vars;
    vars.reserve(size.size());

    for (auto dim_size : size)
    {
        expr_ptr range = nullptr;
        if (dim_size == array_var::unconstrained)
            range = make_shared<infinity>();
        else
            range = make_int(dim_size);
        auto v = make_shared<array_var>(range, location_type());
        vars.push_back(v);
    }

    return vars;
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
            args.push_back(make_int(0));
        else
            args.push_back(make_shared<reference>(vars[d], location_type(), make_int_type()));
    }

    return args;
}

expr_ptr array_reducer::apply(expr_ptr expr, const vector<expr_ptr> & given_args)
{
    assert(expr->type);

    if (verbose<array_reducer>::enabled())
    {
        cout << expr->location << ": ";
        cout << "Applying: ";
        m_printer.print(expr, cout);
        cout << " : " << *expr->type;
        cout << endl;
    }

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

            // Run substitution on arr rather than arr->expr
            // to include array's local ids.
            m_sub(arr);
            result = arr->expr;
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
        if ( dynamic_pointer_cast<func_app>(ar->expr.expr) ||
             ar->vars.size() == ar_type->size.size() )
        {
            // Assume it is fully expanded.
            return expr;
        }

        is_recursive_array = ar->is_recursive;
    }
    // Prevent eta expansion of inputs and external calls:
    else if (dynamic_pointer_cast<external>(expr) ||
             dynamic_pointer_cast<func_app>(expr))
    {
        return expr;
    }

    auto new_ar = make_shared<array>();

    new_ar->vars = make_array_vars(ar_type->size);

    vector<expr_ptr> args;
    for (auto & var : new_ar->vars)
        args.push_back(make_shared<reference>(var, location_type(), make_int_type()));

    new_ar->expr = apply(expr, args);
    new_ar->type = ar_type;
    new_ar->is_recursive = is_recursive_array;

    return new_ar;
}

expr_ptr array_reducer::lambda_lift(expr_ptr e, const string & name)
{
    // Find free array variables in e
    free_array_vars free_vars(e);

    // Make array a = [i,j,... -> e] where i,j,... are free variables in e.
    auto arr = make_shared<stream::functional::array>();
    array_ref_sub var_sub(m_copier, m_printer);
    array_ref_sub::var_map::scope_holder sub_scope(var_sub.vars);
    array_size_vec extra_dims;
    for (auto & v : free_vars.result())
    {
        // Make a new variable with range equal to free variable.
        auto v2 = make_shared<array_var>(v->range, location_type());
        arr->vars.push_back(v2);
        extra_dims.push_back(arrp::array_size(v));

        // Substitute free variable with the new variable.
        var_sub.vars.bind(v, make_ref(v2));
    }
    arr->expr = var_sub(e);
    arr->type = arrp::inflate_type(arr->expr->type, extra_dims);

    // Make new global id n = a;
    auto unique_name = m_name_provider.new_name(name);
    auto id = make_shared<identifier>(unique_name, arr, location_type());
    m_additional_ids.insert(id);
    id->expr = reduce(id->expr);

    // Make expression n[i,j,...] and return it instead of e.
    auto ref = make_ref(id);
    auto app = make_shared<array_app>();
    app->object = ref;
    for (auto & v : free_vars.result())
        app->args.push_back(expr_slot(make_ref(v)));
    app->type = e->type;

    return app;
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
