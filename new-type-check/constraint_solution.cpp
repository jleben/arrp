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

void type_constraint_solver::solve()
{
    unify();
}

void type_constraint_solver::unify()
{
    auto it = m_graph.relations.begin();
    while(it != m_graph.relations.end())
    {
        bool unified = unify(*it);
        if(unified)
            it = m_graph.relations.erase(it);
        else
            ++it;
    }
}

bool type_constraint_solver::unify(type_relation * relation)
{
    if (relation->kind != equal_type)
        return false;

    unify(relation->a, relation->b);

    return true;
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
            unify(c, a->element);
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
