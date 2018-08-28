#include "topo_sort.hpp"

#include <iostream>

using namespace std;

namespace arrp {

void topological_sort::process(scope & scope)
{
    sorted_scope sorted;

    for (auto & id : scope.ids)
    {
        m_scopes[id] = &sorted;
    }

    for (auto & id : scope.ids)
    {
        cout << "ID in scope: " << id->name << endl;

        if (m_visited_ids.find(id) != m_visited_ids.end())
        {
            cout << "Already visited." << endl;
            continue;
        }

        visit_id(id);
    }

    scope.ids = sorted.ids;
}

void topological_sort::visit_id(const id_ptr & id)
{
    cout << "Visiting ID: " << id->name << endl;

    m_visiting_ids.insert(id);
    visit(id->expr);
    m_visited_ids.insert(id);
    m_visiting_ids.erase(id);

    auto * scope = m_scopes[id];
    scope->ids.push_back(id);
}

void topological_sort::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        cout << "Ref: " << id->name  << endl;

        if (m_visiting_ids.find(id) != m_visiting_ids.end())
        {
            cout << "Recursion." << endl;
        }
        else if (m_visited_ids.find(id) != m_visited_ids.end())
        {
            cout << "Already visited." << endl;
        }
        else
        {
            visit_id(id);
        }
    }
}

void topological_sort::visit_array(const shared_ptr<stream::functional::array> & arr)
{
    for (auto & var : arr->vars)
    {
        if (var->range)
            visit(var->range);
    }

    process(arr->scope);

    visit(arr->expr);
}

void topological_sort::visit_func(const shared_ptr<function> & func)
{
    process(func->scope);

    visit(func->expr);
}

void topology_printer::visit_scope(scope & s)
{
    for (const auto & id : s.ids)
    {
        cout << indent() << id->name << endl;
        ++m_indent;
        visit(id->expr);
        --m_indent;
    }
}

void topology_printer::visit_func(const shared_ptr<function> & func)
{
    visit_scope(func->scope);
    visit(func->expr);
}
void topology_printer::visit_array(const shared_ptr<stream::functional::array> & arr)
{
    visit_scope(arr->scope);
    visit(arr->expr);
}

}
