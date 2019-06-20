#include "func_reduction.hpp"
#include "error.hpp"
#include "type_check.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/debug.hpp"

using namespace stream;
using namespace stream::functional;
using namespace std;

namespace arrp {

class func_var_sub : public fn::rewriter_base
{
public:
    using context_type = context<var_ptr, expr_ptr>;

    func_var_sub(copier & c): m_copier(c) {}

    expr_ptr operator()(const expr_ptr & e)
    {
        return visit(e);
    }

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;

    context_type m_context;

private:
    copier & m_copier;
};

expr_ptr func_var_sub::visit_ref(const shared_ptr<reference> & ref)
{
    printer p;

    if (auto fv = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto binding = m_context.find(fv);
        if (binding)
        {
            if (func_reduction::verbose())
            {
                cout << "Substituting a reference to var " << fv << " = ";
                p.print(binding.value(), cout);
                cout << endl;
            }
            return m_copier.copy(binding.value());
        }
        else
        {
            if (func_reduction::verbose())
            {
                cout << "No substitution for reference to var " << fv << endl;
            }
        }
    }

    return ref;
}

bool func_reduction::verbose()
{
    return stream::verbose<func_reduction>::enabled();
}

func_reduction::func_reduction(fn::name_provider & nm):
    m_name_provider(nm)
{
}

void func_reduction::reduce(fn::id_ptr id)
{
    if (verbose())
        cout << "Reducing ID: " << id << endl;

    id->type_expr = visit(id->type_expr);
    id->expr = try_expose_function(visit(id->expr));

    if (verbose())
    {
        cout << "Reduced ID:";
        m_printer.print(id, cout);
        cout << endl;
    }
}

fn::expr_ptr func_reduction::visit_func(const shared_ptr<fn::function> & f)
{
    // We wait never visit function bodies.
    // A function must be fully applied for its body to be further reduced.
    return f;
}

fn::expr_ptr func_reduction::visit_func_app(const shared_ptr<fn::func_app> & app)
{
    static int last_tag = 0;

    ++last_tag;
    int tag = last_tag;

    if (verbose())
    {
        cout << tag << " Application (Raw): ";
        m_printer.print(app, cout);
        cout << endl;
    }

    app->object = visit(app->object);

    for (auto & arg : app->args)
    {
        arg = visit(arg);
    }

    if (verbose())
    {
        cout << tag << " Application (Reduced): ";
        m_printer.print(app, cout);
        cout << endl;
    }

    vector<expr_ptr> arg_exprs;
    for (auto & slot : app->args)
        arg_exprs.push_back(slot.expr);

    expr_ptr object = app->object;

    int remaining_arg_count = app->args.size();
    auto * remaining_args = arg_exprs.data();
    while(remaining_arg_count > 0)
    {
        if (verbose())
            cout << tag << " Remaining args = " << remaining_arg_count << endl;

        object = try_expose_function(object);

        if (verbose())
        {
            cout << tag << " Trying to apply: "; m_printer.print(object, cout); cout << endl;
        }

        if (auto f = dynamic_pointer_cast<function>(object))
        {
            int applied_arg_count = std::min(int(f->vars.size()), remaining_arg_count);
            object = apply(f, remaining_args, applied_arg_count);
            remaining_args += applied_arg_count;
            remaining_arg_count -= applied_arg_count;

            if (verbose())
            {
                cout << tag << " About to reduce applied object:";
                m_printer.print(object, cout);
                cout << endl;
            }

            // Revisit to do reductions enabled by function variable substitutions.
            object = visit(object);

            if (verbose())
            {
                cout << tag << " Applied and reduced object:";
                m_printer.print(object, cout);
                cout << endl;
            }
        }
        else
        {
            if (verbose())
                cout << tag << " Not a function." << endl;
            break;
        }
    }

    if (remaining_arg_count > 0)
    {
        bool ok = false;

        if (auto ref = dynamic_pointer_cast<reference>(object))
        {
            if (dynamic_pointer_cast<func_var>(ref->var))
            {
                ok = true;
                if (verbose())
                    cout << "Terminating application at function variable." << endl;
            }
            else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
            {
                if (dynamic_pointer_cast<external>(id->expr.expr))
                {
                    ok = true;
                    if (verbose())
                        cout << "Terminating application at external function." << endl;
                }
            }
        }

        if (!ok)
        {
            ostringstream msg;
            msg << "Can not apply remaining " << remaining_arg_count << " arguments.";
            throw source_error(msg.str(), app->location);
        }

        app->object = object;
        app->args = vector<expr_slot>(remaining_args,
                                      remaining_args + remaining_arg_count);
        return app;
    }

    return object;
}

fn::expr_ptr func_reduction::apply(shared_ptr<fn::function> f,
                                   fn::expr_ptr* args,
                                   int applied_arg_count)
{
    unordered_set<id_ptr> ids;
    copier copy(ids, m_name_provider);
    func_var_sub sub(copy);

    assert_or_throw(applied_arg_count <= int(f->vars.size()));

    auto new_expr = f->expr;

    {
        func_var_sub::context_type::scope_holder local_ctx(sub.m_context);

        auto scope_e = make_shared<scope_expr>();

        // Turn each applied argument into identifier,
        // if it is not already a reference.
        for (int i = 0; i < applied_arg_count; ++i)
        {
            auto & var = f->vars[i];
            auto arg = args[i];

            if (!dynamic_pointer_cast<reference>(arg))
            {
                auto name = m_name_provider.new_name(var->qualified_name);
                auto id = make_shared<identifier>(name, arg, var->location);
                scope_e->local.ids.push_back(id);
                auto ref = make_shared<reference>(id, location_type());
                arg = ref;
            }

            sub.m_context.bind(var, arg);
        }

        new_expr = sub(new_expr);

        if (scope_e->local.ids.size())
        {
            scope_e->value = new_expr;
            new_expr = scope_e;
        }
    }

    if (f->vars.size() > applied_arg_count)
    {
        f->expr = new_expr;
        f->vars = vector<func_var_ptr>(f->vars.begin() + applied_arg_count, f->vars.end());
        return f;
    }
    else
    {
        return new_expr;
    }
}

fn::expr_ptr func_reduction::visit_ref(const shared_ptr<fn::reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        bool was_visited = m_visited_ids.count(id) > 0;

        m_visited_ids.insert(id);

        if (!was_visited)
        {
            reduce(id);
        }

        if (auto f = dynamic_pointer_cast<function>(id->expr.expr))
        {
            if (id->is_recursive)
                throw stream::source_error("Recursive function not allowed.", id->location);

            unordered_set<id_ptr> ids;
            copier copy(ids, m_name_provider);
            auto f2 =  copy.copy(f);

            if (verbose())
            {
                cout << "Returning a copy of function instead of its name: "
                     << id->name << " => ";
                m_printer.print(f2, cout);
                cout << endl;
            }
            return f2;
        }
    }

    return ref;
}

fn::expr_ptr func_reduction::visit_scope(const shared_ptr<fn::scope_expr> & scope)
{
    auto result = rewriter_base::visit_scope(scope);
    assert_or_throw(result == scope);

    // Remove local ids from visited ids, so they can be revisited when
    // further function variables are substituted.

    for (auto & id : scope->local.ids)
    {
        m_visited_ids.erase(id);
    }

    return result;
}

fn::expr_ptr func_reduction::try_expose_function(fn::expr_ptr e)
{
    auto sub_expr = e;

    while(auto scope = dynamic_pointer_cast<scope_expr>(sub_expr))
    {
        if (auto f = dynamic_pointer_cast<function>(scope->value.expr))
        {
            scope->value = f->expr;
            f->expr = e;
            return f;
        }
        sub_expr = scope->value;
    }

    return e;
}

}
