#include "func_reduction.hpp"
#include "../utility/debug.hpp"
#include "../common/func_model_printer.hpp"

using namespace std;

namespace arrp {

using stream::verbose;
using namespace stream;
using functional::function;

func_reducer::func_reducer(name_provider & nmp):
    m_trace("trace"),
    m_name_provider(nmp),
    m_copier(m_ids, nmp),
    m_var_sub(m_copier)
{
    m_trace.set_enabled(false);
}

void func_reducer::process(const vector<id_ptr> & ids)
{
    for (auto id : ids)
        process(id);
}

void func_reducer::process(const id_ptr & id)
{
    if (m_processed_ids.find(id) != m_processed_ids.end())
        return;

    if (!id->expr)
        return;

    if (verbose<type_checker>::enabled())
    {
        cout << "Processing id " << id->name << endl;
    }

    bool is_visiting =
            std::find(m_processing_ids.begin(), m_processing_ids.end(), id)
            != m_processing_ids.end();

    if (is_visiting)
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "Recursion at id: " << id->name << endl;
        }
        return;
    }

    auto processing_id_token = stream::stack_scoped(id, m_processing_ids);

    id->expr = visit(id->expr);

    m_processed_ids.insert(id);

    if (dynamic_pointer_cast<function>(id->expr.expr) ||
        dynamic_pointer_cast<external>(id->expr.expr))
        return;

    m_ids.insert(id);
}

expr_ptr func_reducer::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (!id->expr)
        {
            throw source_error("Name '" + id->name + "' is used, but has no defined value.",
                               ref->location);
        }

        // FIXME: process explicit type?

        // Try to reduce the id's expression, if it is not a recursion.
        // If it is a recursion, we don't care about processing it now anyway,
        // since recursive functions are not allowed.
        process(id);

        ref->type2 = id->expr->type2;

        if (auto func = dynamic_pointer_cast<function>(id->expr.expr))
        {
            if (id->is_recursive)
            {
                throw type_error("Recursive use of function not allowed.",
                                      id->location);
            }
            if (verbose<type_checker>::enabled())
            {
                cout << "Reference to id " << id->name
                     << " is a function - using a copy of the function intead."
                     << endl;
            }
            return m_copier.copy(func);
        }
        else if (auto ext = dynamic_pointer_cast<external>(id->expr.expr))
        {
            if (!ext->is_input)
            {
                if (verbose<type_checker>::enabled())
                {
                    cout << "Reference to id " << id->name
                         << " is an external function call - using a copy of the call instead."
                         << endl;
                }
                return m_copier.copy(ext);
            }
        }

        return ref;
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        ref->type2 = new scalar_type(primitive_type::integer);
        return ref;
    }
    else
    {
        throw error("Function variable was not substituted.");
    }
}

expr_ptr func_reducer::visit_func(const shared_ptr<function> & func)
{
    return func;
}

expr_ptr func_reducer::visit_func_app(const shared_ptr<func_app> & app)
{
    app->object = visit(app->object);

    expr_ptr result;

    if (auto func = dynamic_pointer_cast<function>(app->object.expr))
    {
        // convert vector of slots to vector of expressions
        vector<expr_ptr> args;
        for (auto & arg : app->args)
            args.push_back(arg);

        result = apply(func, args, app->location);
    }
    else
    {
        if (auto ref = dynamic_pointer_cast<reference>(app->object.expr))
        {
            if (auto id = dynamic_pointer_cast<identifier>(ref->var))
            {
                if (id->is_recursive)
                {
                    throw source_error("Recursive function application not allowed.",
                                       app->location);
                }
            }
        }
        throw type_error("Not a function.", app->object.location);
    }

    return result;
}

expr_ptr func_reducer::apply
(expr_ptr e, const vector<expr_ptr> & args, const location_type & loc)
{
    if (verbose<type_checker>::enabled())
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
        throw type_error(msg.str(), loc);
    }

    if (verbose<type_checker>::enabled())
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

    printer p;

    reduce_context_type::scope_holder scope(m_var_sub.m_context);

    // Prepare arguments

    for (int i = 0; i < args.size(); ++i)
    {
        auto & var = func->vars[i];
        auto arg = args[i];
        arg = visit(arg);

        // FIXME: Don't lift simple expressions
        arg = lambda_lift(arg, var->qualified_name);
        if (verbose<type_checker>::enabled())
        {
            cout << "Lambda-lifting arg " << i << " = ";
            p.print(arg, cout);
            cout << endl;
        }
#if 0
        atomic_expr_check is_atomic;
        if (var->ref_count > 1 && !is_atomic(arg) && !arg->type->is_function())
        {
            if (verbose<type_checker>::enabled())
            {
                cout << "Lambda-lifting arg " << i << " = ";
                p.print(arg, cout);
                cout << endl;
            }

        }
#endif
        if (verbose<type_checker>::enabled())
        {
            cout << "+ bound var " << var << " = ";
            p.print(arg, cout);
            cout << endl;
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
        if (verbose<type_checker>::enabled())
        {
            cout << "Pushing scope of applied function:";
            cout << " (" << &func->scope << ")";
            cout << endl;
        }

        m_scope_stack.push(&func->scope);

        {
            auto id_copies = func->scope.ids;
            for (auto id : id_copies)
                process(id);
        }

        func->expr = visit(func->expr);

        if (verbose<type_checker>::enabled())
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

expr_ptr func_reducer::lambda_lift(expr_ptr e, const string & name)
{
    auto id_name = m_name_provider.new_name(name);
    auto id = make_shared<identifier>(id_name, e, location_type());
    m_ids.insert(id);
    if (m_scope_stack.size())
    {
        m_scope_stack.top()->ids.push_back(id);
        if (verbose<type_checker>::enabled())
        {
            cout << "Stored lambda-lifted expression with id " << id_name
                 << " into enclosing function scope."
                 << " (" << m_scope_stack.top() << ")"
                 << endl;
        }
    }
    else
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "No enclosing function scope for lambda-lifted expression with id "
                 << id_name
                 << endl;
        }
    }

    auto ref = make_shared<reference>(id, e->location);
    ref->type2 = e->type2;

    return ref;
}

expr_ptr func_var_sub::visit_ref(const shared_ptr<reference> & ref)
{
    printer p;

    if (auto fv = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto binding = m_context.find(fv);
        if (binding)
        {
            if (verbose<type_checker>::enabled())
            {
                cout << "Substituting a reference to var " << fv << " = ";
                p.print(binding.value(), cout);
                cout << endl;
            }
            return m_copier.copy(binding.value());
        }
        else
        {
            if (verbose<type_checker>::enabled())
            {
                cout << "No substitution for reference to var " << fv << endl;
            }
        }
    }

    return ref;
}

}

