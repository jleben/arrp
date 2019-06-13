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
    p.set_print_scopes(true);

    if (auto fv = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto binding = m_context.find(fv);
        if (binding)
        {
            //if (verbose<type_checker>::enabled())
            {
                cout << "Substituting a reference to var " << fv << " = ";
                p.print(binding.value(), cout);
                cout << endl;
            }
            return m_copier.copy(binding.value());
        }
        else
        {
            //if (verbose<type_checker>::enabled())
            {
                cout << "No substitution for reference to var " << fv << endl;
            }
        }
    }

    return ref;
}

func_reduction::func_reduction(fn::name_provider & nm):
    m_name_provider(nm)
{
    m_printer.set_print_scopes(true);
}

void func_reduction::reduce(fn::id_ptr id)
{
    cerr << "Reducing ID: " << id << endl;

    id->type_expr = visit(id->type_expr);
    id->expr = visit(id->expr);

    cerr << "Reduced ID:";
    m_printer.print(id, cerr);
    cerr << endl;
}

fn::expr_ptr func_reduction::visit_func_app(const shared_ptr<fn::func_app> & app)
{
    cerr << "Application (Raw): ";
    m_printer.print(app, cerr);
    cerr << endl;

    app->object = visit(app->object);

    for (auto & arg : app->args)
    {
        // FIXME: Turn argument into local identifier if not a simple reference.
        arg = visit(arg);
    }

    cerr << "Application (Reduced): ";
    m_printer.print(app, cerr);
    cerr << endl;

    vector<expr_ptr> arg_exprs;
    for (auto & slot : app->args)
        arg_exprs.push_back(slot.expr);

    expr_ptr object = app->object;

    int applied_arg_count = 0;
    while(applied_arg_count < app->args.size())
    {
        cerr << "Remaining args = " << (app->args.size() - applied_arg_count) << endl;

        if (auto f = dynamic_pointer_cast<function>(object))
        {
            object = apply(f, arg_exprs, applied_arg_count);
            applied_arg_count += f->vars.size();

            cerr << "Applied = ";
            m_printer.print(object, cerr);
            cerr << endl;
        }
        else
        {
            break;
        }
    }

    int remaining_args = app->args.size() - applied_arg_count;

    if (remaining_args > 0)
    {
        if (auto e = dynamic_pointer_cast<external>(app->object.expr))
        {
            app->object = object;
            app->args = vector<expr_slot>(app->args.begin() + applied_arg_count,
                                          app->args.end());
            cerr << "Terminating application at external function." << endl;
            return app;
        }
        else
        {
            ostringstream msg;
            msg << "Can not apply remaining " << remaining_args << " arguments.";
            throw source_error(msg.str(), app->location);
        }
    }

    return object;
}

fn::expr_ptr func_reduction::apply(shared_ptr<fn::function> f,
                                   const vector<fn::expr_ptr> & args,
                                   int first_arg_idx)
{
    unordered_set<id_ptr> ids;
    copier copy(ids, m_name_provider);
    func_var_sub sub(copy);

    int applied_arg_count = std::min(f->vars.size(), args.size() - first_arg_idx);

    {
        func_var_sub::context_type::scope_holder local_ctx(sub.m_context);

        for (int i = 0; i < applied_arg_count; ++i)
        {
            sub.m_context.bind(f->vars[i], args[first_arg_idx + i]);
        }

        f->expr = sub(f->expr);
    }

    if (f->vars.size() > applied_arg_count)
    {
        f->vars = vector<func_var_ptr>(f->vars.begin() + applied_arg_count, f->vars.end());
        return f;
    }
    else
    {
        return f->expr;
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

            cerr << "Returning a copy of function instead of its name: "
                 << id->name << " => ";
            m_printer.print(f2, cerr);
            cerr << endl;
            return f2;
        }
    }

    return ref;
}


}
