#include "scope_cleanup.hpp"

using namespace std;
using namespace stream;
using namespace stream::functional;

namespace arrp {

void scope_cleanup::clean(fn::scope & scope)
{
    for (auto & id : scope.ids)
    {
        if (id->is_output)
        {
            id->ref_count = 1;
            m_used_ids.insert(id);
        }
        else
        {
            id->ref_count = 0;
        }
    }

    for (auto & id : scope.ids)
    {
        if (id->is_output)
            visit_local_id(id);
    }

    remove_unused(scope);
}

void scope_cleanup::remove_unused(fn::scope & e)
{
    vector<id_ptr> remaining;
    for (auto & id : e.ids)
    {
        if (m_used_ids.count(id))
            remaining.push_back(id);
    }
    e.ids = remaining;
}

expr_ptr scope_cleanup::visit_scope(const shared_ptr<fn::scope_expr> & scope)
{
    for (auto & id : scope->local.ids)
        id->ref_count = 0;

    visit(scope->value);
    remove_unused(scope->local);

    if (scope->local.ids.empty())
        return scope->value;
    else
        return scope;
}

expr_ptr scope_cleanup::visit_ref(const shared_ptr<fn::reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        ++id->ref_count;

        if (m_used_ids.count(id))
            return ref;

        m_used_ids.insert(id);
        visit_local_id(id);
    }

    return ref;
}

}
