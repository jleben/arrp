#pragma once

#include "types.hpp"
#include "type_graph.hpp"
#include "../utility/stacker.hpp"

namespace arrp {

class type_constraint_solver
{
public:
    type_constraint_solver(type_graph &, type_graph_printer &);
    void solve();
    void eliminate_equalities();
    void eliminate_subtype_cycles();

private:
    type unify(const type & a, const type & b);
    type unify_concrete(const type & a, const type & b);
    type unify(type_class & c, const type &);

    bool eliminate_subtype_cycles(const type &);
    void eliminate_subtype_cycle(const type &);

    type actual(const type &);
    void move_relations(const type & a, const type & b);

    type_graph & m_graph;
    type_graph_printer & m_printer;

    using visiting_nodes_t = stream::stack_adapter<list<type>>;
    visiting_nodes_t m_node_trail;
};

}
