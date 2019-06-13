#include "scope_cleanup.hpp"

using namespace std;
using namespace stream;
using namespace stream::functional;

namespace arrp {

void scope_cleanup::clean(fn::scope & scope, fn::id_ptr id)
{
    m_used_ids.insert(id);
    visit_local_id(id);
    clean(scope);
}

void scope_cleanup::clean(fn::scope & e)
{
    vector<id_ptr> remaining;
    for (auto & id : e.ids)
    {
        if (m_used_ids.count(id))
            remaining.push_back(id);
    }
    e.ids = remaining;
}

void scope_cleanup::visit_scope(const shared_ptr<fn::scope_expr> & scope)
{
    visit(scope->value);
    clean(scope->local);
}

void scope_cleanup::visit_ref(const shared_ptr<fn::reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (m_used_ids.count(id))
            return;

        m_used_ids.insert(id);
        visit_local_id(id);
    }
}

}
