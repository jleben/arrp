#include "constraint_solution.hpp"
#include "type_graph.hpp"
#include "../common/error.hpp"

#include <sstream>

using namespace std;

namespace arrp {

type_constraint_solver::type_constraint_solver(type_graph & graph, type_graph_printer & printer):
    m_graph(graph),
    m_printer(printer)
{

}

void type_constraint_solver::solve(const unordered_set<id_ptr> & ids)
{
    eliminate_equalities();

    cout << "-- Eliminated equalities:" << endl;
    m_printer.print(ids, m_graph, cout);

    eliminate_subtype_cycles();

    cout << "-- Eliminated subtype cycles:" << endl;
    for (auto r : m_graph.relations)
    {
        m_printer.print(r, cout);
        cout << endl;
    }
}

void type_constraint_solver::eliminate_equalities()
{
    auto it = m_graph.relations.begin();
    while(it != m_graph.relations.end())
    {
        type_relation * r = *it;
        if (r->kind == equal_type)
        {
            r->a->relations.remove(r);
            r->b->relations.remove(r);

            unify(r->a, r->b);

            delete *it;
            it = m_graph.relations.erase(it);
        }
        else
        {
            ++it;
            continue;
        }
    }
}

void type_constraint_solver::eliminate_subtype_cycles()
{
    m_node_trail.clear();

    for (type_relation * r : m_graph.relations)
        r->visited = false;

    auto it = m_graph.relations.begin();
    while(it != m_graph.relations.end())
    {
        type_relation * r = *it;

        assert(r->kind != equal_type);
        if (r->kind != sub_type)
        {
            ++it;
            continue;
        }

        if (!r->visited)
        {
            eliminate_subtype_cycles(r->a);
        }

        if (r->a == r->b)
        {
            // The relation is obsolete
            r->a->relations.remove(r);
            it = m_graph.relations.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool type_constraint_solver::eliminate_subtype_cycles(const type & t)
{
    stream::revertable<bool> node_visited_marker(t->visited, true);
    auto node_trail_marker = stream::stack_scoped(t, m_node_trail);

    // Find one cycle recursively.
    // When it is found, eliminate it, and return all the way
    // back to eliminate_subtype_cycles(), which will find the next
    // unvisited relation.

    cout << "Eliminating cycles accessible from: "
         << m_printer.name_for(t) << endl;

    for (auto r : t->relations)
    {
        if (r->visited)
        {
            continue;
        }

        if (r->kind != sub_type)
        {
            assert(r->kind != equal_type);
            continue;
        }

        // Skip if t is not the subtype in the relation
        if (t != r->a)
        {
            continue;
        }

        // Skip self relation
        // The relation is obsolete.
        // It will be removed in eliminate_subtype_cycles();
        if (r->b == t)
        {
            continue;
        }

        // If the supertype was already visited, this is a cycle.
        if (r->b->visited)
        {
            eliminate_subtype_cycle(r->b);
            return true;
        }

        // Else recurse into the other end of relation.
        // If any cycles where eliminated there, return.

        r->visited = true;

        if (eliminate_subtype_cycles(r->b))
            return true;
    }

    if(t.as<type_variable>())
    {
        auto v = t.as<type_variable>();
        for (auto & c : v->classes)
        {
            switch(c.kind)
            {
            case indexable_type: // parameter = the result of indexing
            case array_like_type: // parameter = the innermost element type
            {
                auto t2 = actual(c.parameters[0]);
                cout << "Array or indexable class with param: ";
                m_printer.print(t2, cout);
                cout << endl;
                if (t2 == t)
                {
                    continue;
                }
                else if(t2->visited)
                {
                    // FIXME: Unifying a parameter in a class with
                    // the variable to which the class applies creates a
                    // reference cycle.
                    // Maybe handle this when unifying?

                    // This is a cycle.
                    eliminate_subtype_cycle(t2);
                    return true;
                }
                else if (eliminate_subtype_cycles(t2))
                {
                    return true;
                }
                break;
            }
            default:
                break;
            }
        }
    }

    return false;
}

void type_constraint_solver::eliminate_subtype_cycle(const type & t)
{
    cout << "Cycle detected at:";
    m_printer.print(t, cout);
    cout << endl;

    type u = t;

    auto it = m_node_trail.rbegin();
    assert(it != m_node_trail.rend());

    while(*it != t)
    {
        const type & t2 = *it;

        cout << "Unifying with: ";
        m_printer.print(t2, cout);
        cout << endl;

        u = unify(t2, u);

        ++it;
        assert(it != m_node_trail.rend());
    }
}

type type_constraint_solver::unify(const type & raw_a, const type & raw_b)
{
    type a = actual(raw_a);
    type b = actual(raw_b);

    if (a == b)
        return b;

    auto av = a.as<type_variable>();
    auto bv = b.as<type_variable>();

    if (av && bv)
    {
        // FIXME: Check if one is structurally included in other's classes.

        // Move classes of a to b

        std::move(av->classes.begin(), av->classes.end(), back_inserter(bv->classes));
        av->classes.clear();

        // Move relations of a to b

        move_relations(a, b);

        // Point a to b

        a->value = b;

        return b;
    }
    else if (av)
    {
        // a is variable and b is not

        if (is_included_in(a, b))
        {
            ostringstream msg;
            msg << "Unification would result in recursive type: ";
            m_printer.print(a, msg);
            msg << " is included in ";
            m_printer.print(b, msg);
            throw stream::error(msg.str());
        }

        // Unify classes of with b

        type t = b;

        for (auto & c : av->classes)
        {
            t = unify(c, t);
        }

        av->classes.clear();

        // Move relations of a to b

        move_relations(a, t);

        // Point a to b

        a->value = t;

        return t;
    }
    else if (bv)
    {
        return unify(b,a);
    }
    else
    {
        return unify_concrete(a, b);
    }

    // if both are var:
    //   make one equal another and copy its constraints
    //   to this other one
    // if one is var and another is concrete:
    //   unify concrete with constraints
    // if both are concrete:
    //   they must be identical
}

type type_constraint_solver::unify_concrete(const type & a, const type & b)
{
    move_relations(a, b);
    a->value = b;

    if (a.as<infinity_type>() && b.as<infinity_type>())
    {
        return b;
    }
    else if (a.as<scalar_type>() && b.as<scalar_type>())
    {
        if (a.as<scalar_type>()->type == b.as<scalar_type>()->type)
            return b;
    }
    else if (a.as<array_type>() && b.as<array_type>())
    {
        auto array_a = a.as<array_type>();
        auto array_b = b.as<array_type>();
        array_b->element = unify(array_a->element, array_b->element);
        return b;
    }

    ostringstream msg;
    msg << "Types are not unifiable: " << endl;
    m_printer.print(a, msg);
    m_printer.print(b, msg);
    // FIXME: throw source_error
    throw stream::error(msg.str());
}

/*
    undefined,
    boolean,
    integer,
    real32,
    real64,
    complex32,
    complex64,
    infinity
    */
type type_constraint_solver::unify(type_class & c, const type & raw_t)
{
    using pt = primitive_type;

    type t = actual(raw_t);

    if (auto v = t.as<type_variable>())
    {
        v->classes.push_back(c);
        return t;
    }

    switch (c.kind) {
    case data_type:
    {
        if (t->is_data())
            return t;
        break;
    }
    case scalar_data_type:
    {
        if (t.as<scalar_type>())
            return t;
        break;
    }
    case numeric_type:
    {
        auto s = t.as<scalar_type>();
        if (!s)
            break;
        auto p = s->type;
        if (p == pt::integer || p == pt::real32 || p == pt::real64 ||
            p == pt::complex32 || p == pt::complex64)
            return t;
        break;
    }
    case simple_numeric_type:
    {
        auto s = t.as<scalar_type>();
        if (!s)
            break;
        auto p = s->type;
        if (p == pt::integer || p == pt::real32 || p == pt::real64)
            return t;
        break;
    }
    case real_numeric_type:
    {
        auto s = t.as<scalar_type>();
        if (!s)
            break;
        auto p = s->type;
        if (p == pt::real32 || p == pt::real64 ||
            p == pt::complex32 || p == pt::complex64)
            return t;
        break;
    }
    case complex_numeric_type: // parameter = real type
    {
        auto s = t.as<scalar_type>();
        if (!s)
            break;
        auto p = s->type;
        if (p == pt::complex32)
        {
            if (c.parameters.size())
                unify(c.parameters[0], new scalar_type(pt::real32));
            return t;
        }
        if (p == pt::complex64)
        {
            if (c.parameters.size())
                unify(c.parameters[0], new scalar_type(pt::real64));
            return t;
        }
        break;
    }
    case indexable_type: // parameter = the result of indexing
    {
        auto a = t.as<array_type>();
        if (a)
        {
            if (c.parameters.size())
                a->element = unify(c.parameters[0], a->element);
            return t;
        }
        auto s = t.as<scalar_type>();
        if (s)
        {
            if (c.parameters.size())
                return unify(c.parameters[0], t);
            return t;
        }
        break;
    }
    case array_like_type:
    {
        auto a = t.as<array_type>();
        if (a)
        {
            a->element = unify(c, a->element);
            return t;
        }
        auto s = t.as<scalar_type>();
        if (s)
        {
            if (c.parameters.size())
                return unify(c.parameters[0], t);
            return t;
        }
        break;
    }
    default:
        break;
    }

    ostringstream msg;
    msg << "Type does not meet constraint:" << endl;
    m_printer.print(t, msg);
    msg << endl;
    m_printer.print(c, msg);
    // FIXME: throw source_error
    throw stream::error(msg.str());
}

type type_constraint_solver::actual(const type & t)
{
    if (auto v = t.as<type_variable>())
    {
        if (v->value)
            return actual(v->value);

        return v;
    }

    return t;
}

bool type_constraint_solver::is_included_in(const type & aa, const type & bb)
{
    auto a = actual(aa);
    auto b = actual(bb);

    if (!a || !b)
        return false;

    if (a == b)
        return true;

    if (auto array = b.as<array_type>())
    {
        return is_included_in(a, array->element);
    }
    else if (auto func = b.as<function_type>())
    {
        for (auto & param : func->parameters)
        {
            if (is_included_in(a, param))
                return true;
        }
        return is_included_in(a, func->value);
    }

    else return false;
}

void type_constraint_solver::move_relations(const type & a, const type & b)
{
    for (auto r : a->relations)
    {
        if (r->a == a)
            r->a = b;
        if (r->b == a)
            r->b = b;
        b->relations.push_back(r);
    }

    a->relations.clear();
}

}
