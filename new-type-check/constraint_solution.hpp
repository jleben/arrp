#pragma once

#include "types.hpp"
#include "type_graph.hpp"

namespace arrp {

class type_constraint_solver
{
public:
    type_constraint_solver(type_graph &, type_graph_printer &);
    void solve();
    void unify();

private:
    bool unify(type_relation*);
    type unify(const type & a, const type & b);
    type unify_concrete(const type & a, const type & b);
    type unify(type_class & c, const type &);

    type actual(const type &);
    void move_relations(const type & a, const type & b);

    type_graph & m_graph;
    type_graph_printer & m_printer;
};

}
