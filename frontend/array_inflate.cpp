#include "array_inflate.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <list>
#include <vector>

using namespace std;

namespace arrp {

using stream::verbose;

lift_local_ids::lift_local_ids(scope & global):
    m_global(global)
{
    auto ids = global.ids;
    for (auto & id : ids)
    {
        visit_local_id(id);
    }
}

expr_ptr lift_local_ids::visit_scope(const shared_ptr<scope_expr> & scope)
{
    for(auto & id : scope->local.ids)
    {
        m_global.ids.push_back(id);
        visit_local_id(id);
    }

    return visit(scope->value);
}

// Generates set { <a, { b : a referenced in b }> } with a and b some IDs.
class analyze_references : public visitor<void>
{
public:
    array_inflate::array_reference_set process(const unordered_set<id_ptr> & ids)
    {
        m_refs.clear();

        for (auto & id : ids)
        {
            m_current_id = id;
            visit(id->expr);
        }

        return m_refs;
    }

    virtual void visit_ref(const shared_ptr<reference> & ref) override
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            m_refs[id].insert(m_current_id);
        }
    }

    virtual void visit_local_id(const id_ptr &) override {} // Ignore

private:
    array_inflate::array_reference_set m_refs;
    id_ptr m_current_id;
};

class analyze_free_vars : public visitor<void>
{
public:
    using info_set = array_inflate::array_var_info_set;

    info_set process(const unordered_set<id_ptr> & ids)
    {
        m_info.clear();

        for (auto & id : ids)
        {
            m_current_id = id;
            visit(id->expr);
        }

        return m_info;
    }

    virtual void visit_array(const shared_ptr<stream::functional::array> & arr) override
    {
        stream::stacker<array_var_ptr, var_stack_type> bound_vars(m_bound_vars);

        for (auto & var : arr->vars)
        {
            bound_vars.push(var);
            m_info[var].source = m_current_id;
        }

        visitor::visit_array(arr);
    }

    virtual void visit_ref(const shared_ptr<reference> & ref) override
    {
        if (auto v = dynamic_pointer_cast<array_var>(ref->var))
        {
            if (std::find(m_bound_vars.begin(), m_bound_vars.end(), v) == m_bound_vars.end())
                m_info[v].free.insert(m_current_id);
        }
    }

    virtual void visit_local_id(const id_ptr &) override {} // Ignore

private:
    using var_stack_type = stream::stack_adapter<std::deque<array_var_ptr>>;
    var_stack_type m_bound_vars;
    info_set m_info;
    id_ptr m_current_id;
};

void array_inflate::process(const scope & global)
{
    unordered_set<id_ptr> ids(global.ids.begin(), global.ids.end());
    process(ids);
}

void array_inflate::process(const unordered_set<id_ptr> & ids)
{
    m_references.clear();
    m_free_vars.clear();
    {
        analyze_references analyzer;
        m_references = analyzer.process(ids);
    }
    {
        analyze_free_vars analyzer;
        m_free_vars = analyzer.process(ids);
    }

    if (verbose<array_inflate>::enabled())
    {
        for (auto & [v, info] : m_free_vars)
        {
            cout << "Free var: " << v << " | source: " << info.source
                 << " | in: ";
            for (auto & id : info.free)
                cout << id << " ";
            cout << endl;
        }

        for (auto & [id, refs] : m_references)
        {
            cout << "Reference to " << id->name << " in: ";
            for (auto & id : refs)
                cout << id->name << " ";
            cout << endl;
        }
    }

    for (auto & free_var : m_free_vars)
    {
        auto & var = free_var.first;
        auto & info = free_var.second;

        if (info.free.empty())
            continue;

        // Start with involved ids equal to those with the free variable and the source
        unordered_set<id_ptr> involved_ids;
        involved_ids = info.free;
        involved_ids.insert(info.source);

        // Expand set of involved ids to closure under reference relations,
        // with the exception of the source of the free variable.

        close_under_references(involved_ids, { info.source });

        if (verbose<array_inflate>::enabled())
        {
            cout << "=== Inflation for var " << var->name << " from " << info.source << endl;
            cout << "Involved IDs: ";
            for (auto & id : involved_ids)
                cout << id << " , ";
            cout << endl;
        }

        // Inflate involved ids

        m_inflation.involved_ids = involved_ids;
        m_inflation.var = var;
        m_inflation.var_info = info;

        for (auto & id : involved_ids)
        {
            inflate(id);
        }
    }
}

void array_inflate::close_under_references
(unordered_set<id_ptr> & ids, const unordered_set<id_ptr> & exceptions)
{
    list<id_ptr> to_check(ids.begin(), ids.end());

    while(!to_check.empty())
    {
        auto id = to_check.front();
        to_check.pop_front();

        if (exceptions.count(id))
            continue;

        auto & users = m_references[id];
        for (auto & user : users)
        {
            if (ids.count(user))
                continue;

            ids.insert(user);
            to_check.push_back(user);
        }
    }
}

int array_size(const array_var_ptr & var)
{
    if (auto c = dynamic_pointer_cast<int_const>(var->range.expr))
    {
        return c->signed_value();
    }
    else
    {
       return array_var::unconstrained;
    }
}

type_ptr inflate_type(const type_ptr & t, const array_var_ptr & var)
{
    return inflate_type(t, { array_size(var) });
}

type_ptr inflate_type(const type_ptr & t, const array_size_vec & size)
{
    array_size_vec new_size = size;

    if(t->is_array())
    {
        const auto * nested_arr_type = t->array();
        new_size.insert(new_size.end(), nested_arr_type->size.begin(), nested_arr_type->size.end());
        return make_shared<array_type>(new_size, nested_arr_type->element);
    }
    else
    {
        return make_shared<array_type>(new_size, t->scalar()->primitive);
    }
}

void array_inflate::inflate(const id_ptr & id)
{
    if (id != m_inflation.var_info.source)
    {
        if (verbose<array_inflate>::enabled())
            cout << "*** ID " << id << " is not source, so wrapping it into array." << endl;

        // Wrap into an array with one additional variable.
        // Apply this variable to references of other ids that are also being inflated.

        auto arr = make_shared<stream::functional::array>();
        auto v = make_shared<array_var>(m_inflation.var->range, location_type());
        arr->vars = { v };
        arr->expr = id->expr;
        arr->type = inflate_type(arr->expr->type, v);

        m_inflation.substitute_var = v;

        id->expr = visit(arr);
    }
    else
    {
        if (verbose<array_inflate>::enabled())
            cout << "*** ID " << id << " is source." << endl;

        // This is the source of the variable that caused the inflation,
        // so apply this variable to references to other inflated ids.
        m_inflation.substitute_var = m_inflation.var;

        id->expr = visit(id->expr);
    }
}

expr_ptr array_inflate::visit_ref(const shared_ptr<reference> & e)
{
    if (auto id = dynamic_pointer_cast<identifier>(e->var))
    {
        if (verbose<array_inflate>::enabled())
            cout << "Reference to ID " << id->name << endl;

        // If this is an inflated array, apply current array's added variable to it.

        // Do nothing if the reference is the source, since it is never inflated.
        if (id == m_inflation.var_info.source)
        {
            if (verbose<array_inflate>::enabled())
                cout << "Source. Ignore." << endl;
            return e;
        }

        // Do nothing if the referenced id is not being inflated.
        if (!m_inflation.involved_ids.count(id))
        {
            if (verbose<array_inflate>::enabled())
                cout << "Not involved. Ignore." << endl;
            return e;
        }

        if (verbose<array_inflate>::enabled())
            cout << "Applying new variable." << endl;

        // FIXME: Add types to new expressions.
        auto app = make_shared<array_app>();
        auto arg = make_shared<reference>(m_inflation.substitute_var);
        app->object = e;
        app->args = { expr_slot(arg) };

        app->type = e->type;
        arg->type = make_int_type();
        e->type = inflate_type(e->type, m_inflation.substitute_var);

        return app;
    }
    else if (auto v = dynamic_pointer_cast<array_var>(e->var))
    {
        // Replace free variable with current array's added variable.

        if (v != m_inflation.var)
            return e;
        if (v == m_inflation.substitute_var)
            return e;

        if (verbose<array_inflate>::enabled())
            cout << "Replacing free variable reference." << endl;

        return make_shared<reference>(m_inflation.substitute_var, location_type(), e->type);
    }

    return e;
}

}
