#include "reference_analysis.hpp"
#include "../utility/stacker.hpp"

#include <algorithm>

using namespace std;

namespace stream {
namespace functional {

reference_analysis::reference_analysis() {}

void reference_analysis::process(const vector<id_ptr> & ids)
{
    for(auto & id : ids)
        process(id);
}

void reference_analysis::process(id_ptr start)
{
    if (m_done_ids.find(start) != m_done_ids.end())
        return;

    auto id_iter = std::find(m_visited_ids.begin(), m_visited_ids.end(), start);

    if (id_iter != m_visited_ids.end())
    {
        while (id_iter != m_visited_ids.end())
        {
            (*id_iter)->is_recursive = true;
            ++id_iter;
        }
    }
    else
    {
        auto visited_id_token = stack_scoped(start, m_visited_ids);

        visit(start->expr);
    }

    m_done_ids.insert(start);
}

void reference_analysis::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        process(id);
    }
}

void reference_analysis::visit_scope(const shared_ptr<scope_expr> & scope)
{
    process(scope->local.ids);
    visit(scope->value);
}

}
}
