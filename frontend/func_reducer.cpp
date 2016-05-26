#include "func_reducer.hpp"
#include "prim_reduction.hpp"
#include "error.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <iostream>
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

string wrong_arg_count_msg(int required, int actual)
{
    ostringstream text;
    text << " Wrong number of arguments ("
         << "expected: " << required << ", "
         << "actual: " << actual
         << ")."
            ;
    return text.str();
}

func_reducer::func_reducer(name_provider & nmp):
    m_trace("trace"),
    m_name_provider(nmp),
    m_copier(m_ids, nmp),
    m_type_checker(m_trace)
{
    m_trace.set_enabled(false);
}

id_ptr func_reducer::reduce(id_ptr id, const vector<expr_ptr> & args)
{
    if (m_ids.find(id) == m_ids.end())
    {
        if (verbose<func_reducer>::enabled())
        {
            cout << "Reducing expression of id " << id->name << endl;
        }

        id->expr = reduce(id->expr);

        m_type_checker.process(id->expr);

        m_ids.insert(id);
    }

    if (!args.empty())
    {
        if (verbose<func_reducer>::enabled())
        {
            cout << "Applying args to id " << id->name << endl;
        }

        auto func = dynamic_pointer_cast<function>(id->expr.expr);
        if (!func)
        {
            ostringstream msg;
            msg << "Id " << id->name << " is not a function. "
                << " It can not be applied to arguments.";
            throw source_error(msg.str(), id->location);
        }

        auto new_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_name, id->location);
        new_id->expr.location = id->expr.location;

        auto func_copy =
                dynamic_pointer_cast<function>(m_copier.copy(id->expr).expr);

        new_id->expr = apply(func_copy, args, location_type());

        return new_id;
    }
    else
    {
        return id;
    }
}

expr_ptr func_reducer::apply
(expr_ptr e, const vector<expr_ptr> & args, const location_type & loc)
{
    if (verbose<func_reducer>::enabled())
    {
        cout << "+ Function application at: " << loc << endl;
    }

    auto arg_it = args.begin();
    std::shared_ptr<function> f;
    while((f = dynamic_pointer_cast<function>(e)) && arg_it != args.end())
    {
        vector<expr_ptr> applied_args;
        while(arg_it != args.end() && applied_args.size() < f->vars.size())
            applied_args.push_back(*arg_it++);

        e = do_apply(f, applied_args, loc);
    }
    if (arg_it != args.end())
    {
        ostringstream msg;
        msg << "Too many arguments in function application. "
            << (arg_it - args.begin()) << "expected." << endl;
        throw source_error(msg.str(), loc, m_trace);
    }

    if (verbose<func_reducer>::enabled())
    {
        cout << "- Function application at: " << loc << endl;
    }

    return e;
}

expr_ptr func_reducer::do_apply
(shared_ptr<function> func,
 const vector<expr_ptr> & args,
 const location_type & loc)
{
    assert_or_throw(func->vars.size() >= args.size());

    reduce_context_type::scope_holder scope(m_var_sub.m_context);

    // Prepare arguments

    for (int i = 0; i < args.size(); ++i)
    {
        auto & var = func->vars[i];
        auto arg = args[i];
        m_type_checker.process(arg);

        if (var->ref_count > 1 && !dynamic_pointer_cast<reference>(arg)
                && arg->type->is_data())
        {
            auto arg_id_name = m_name_provider.new_name(var->qualified_name);
            auto arg_id = make_shared<identifier>(arg_id_name, arg, location_type());
            arg = make_shared<reference>(arg_id, arg->location);

            m_ids.insert(arg_id);
            if (m_scope_stack.size())
            {
                m_scope_stack.top()->ids.push_back(arg_id);
                if (verbose<func_reducer>::enabled())
                {
                    cout << "Stored id for multi-ref argument " << var->name
                         << " into enclosing function scope."
                         << " (" << m_scope_stack.top() << ")"
                         << endl;
                }
            }
            else
            {
                if (verbose<func_reducer>::enabled())
                {
                    cout << "No enclosing function scope for id for multi-ref argument "
                         << var->name
                         << endl;
                }
            }
        }

        if (verbose<func_reducer>::enabled())
        {
            cout << "+ bound var: " << var << endl;
        }
        m_var_sub.m_context.bind(var, arg);
    }

    // Remember if this is a partial application

    bool is_partial_app = func->vars.size() > args.size();

    // Reduce

    m_trace.push(loc);

    // Substitute

    m_var_sub(func);

    // Reduce

    if (!is_partial_app)
    {
        if (verbose<func_reducer>::enabled())
        {
            cout << "Pushing scope of applied function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.push(&func->scope);

        reduce(func->scope);

        func->expr = reduce(func->expr);

        m_type_checker.process(func->expr);

        if (verbose<func_reducer>::enabled())
        {
            cout << "Popping scope of applied function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.pop();
    }

    m_trace.pop();

    if (is_partial_app)
    {
        // Return partially applied function

        vector<func_var_ptr> remaining_vars
                (func->vars.begin() + args.size(),
                 func->vars.end());

        func->vars = remaining_vars;

        return func;
    }
    else
    {
        // Add ids to enclosing scope, if any.

        if (!m_scope_stack.empty())
        {
            auto & parent_ids = m_scope_stack.top()->ids;
            auto & ids = func->scope.ids;
            parent_ids.insert(parent_ids.end(), ids.begin(), ids.end());
        }

        // Return function expression

        return func->expr;
    }
}

expr_ptr func_reducer::reduce(expr_ptr expr)
{
    return visit(expr);
}

expr_ptr func_reducer::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (auto func = dynamic_pointer_cast<function>(id->expr.expr))
        {
            return m_copier.copy(func);
        }

        if (m_ids.find(id) == m_ids.end())
        {
            if (verbose<func_reducer>::enabled())
            {
                cout << "Reducing id " << id->name
                     << " referenced at " << ref->location << endl;
            }

            stacker<location_type, tracing_stack<location_type>> tracer(m_trace);
            tracer.push(location_type()); // Separator

            id->expr = reduce(id->expr);

            m_ids.insert(id);
        }

        if (is_constant(id->expr))
            return id->expr;

        return ref;
    }
    else
    {
        return ref;
    }
}

expr_ptr func_reducer::visit_primitive(const shared_ptr<primitive> & prim)
{
    auto result = dynamic_pointer_cast<primitive>(rewriter_base::visit_primitive(prim));
    assert(result);
    return reduce_primitive(result);
}

expr_ptr func_reducer::visit_array(const shared_ptr<array> & arr)
{
    for (auto & var : arr->vars)
    {
        if (var->range)
            var->range = reduce(var->range);
    }

    m_scope_stack.push(&arr->scope);

    reduce(arr->scope);

    arr->expr = reduce(arr->expr);

    m_scope_stack.pop();

    return arr;
}

expr_ptr func_reducer::visit_array_size(const shared_ptr<array_size> & as)
{
    as->object = reduce(as->object);
    if (as->dimension)
        as->dimension = reduce(as->dimension);

    m_type_checker.process(as);

    int dim = 0;
    if (as->dimension)
    {
        auto dim_const = dynamic_pointer_cast<constant<int>>(as->dimension.expr);
        assert(dim_const);
        dim = dim_const->value - 1;
    }

    auto arr_type = dynamic_pointer_cast<array_type>(as->object->type);
    assert(arr_type);
    assert(dim >= 0 && dim < arr_type->size.size());

    return make_shared<constant<int>>(arr_type->size[dim]);
}

expr_ptr func_reducer::visit_func(const shared_ptr<function> & func)
{
    return func;

#if 0
    printer p;

    if (verbose<func_reducer>::enabled())
    {
        cout << "Pushing scope of reduced function:";
        cout << " (" << &func->scope << ")";
        cout << endl;
    }

    m_scope_stack.push(&func->scope);

    reduce(func->scope);

    auto reduced_expr = reduce(func->expr);

    m_scope_stack.pop();

    if (auto func2 = dynamic_pointer_cast<function>(reduced_expr))
    {
        func->vars.insert(func->vars.end(), func2->vars.begin(), func2->vars.end());
        func->scope.ids.insert(func->scope.ids.end(),
                               func2->scope.ids.begin(), func2->scope.ids.end());
        reduced_expr = func2->expr;
    }

    func->expr = reduced_expr;

    if (verbose<func_reducer>::enabled())
    {
        cout << "Popping scope of reduced function:";
        cout << " (" << &func->scope << ")";
        cout << endl;
    }

    return func;
#endif
}

expr_ptr func_reducer::visit_func_app(const shared_ptr<func_app> & app)
{
    app->object = reduce(app->object);

    for (auto & arg : app->args)
    {
        arg = reduce(arg);
    }

    auto func = dynamic_pointer_cast<function>(app->object.expr);
    if (!func)
    {
        throw source_error("Not a function.", app->object.location);
    }

    // convert vector of slots to vector of expressions
    vector<expr_ptr> reduced_args;
    for (auto & arg : app->args)
        reduced_args.push_back(arg);

    auto reduced_func = apply(func, reduced_args, app->location);
    return reduced_func;
}

void func_reducer::reduce(scope & s)
{
    // Create a copy, because the set of ids
    // might be modified along the way
    auto ids = s.ids;

    for (auto id : ids)
    {
        if (verbose<func_reducer>::enabled())
            cout << "Reducing scope-local id \"" << id->name << "\"" << endl;

        id->expr = reduce(id->expr);
    }
}

expr_ptr func_var_sub::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto fv = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto binding = m_context.find(fv);
        if (binding)
        {
            if (verbose<func_reducer>::enabled())
            {
                cout << "Substituting a reference to var: " << fv << endl;
            }
            return binding.value();
        }
        else
        {
            if (verbose<func_reducer>::enabled())
            {
                cout << "No substitution for reference to var: " << fv << endl;
            }
        }
    }

    return ref;
}

}
}
