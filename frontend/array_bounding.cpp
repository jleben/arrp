#include "array_bounding.hpp"
#include "error.hpp"
#include "../utility/debug.hpp"

#include <algorithm>

using namespace std;

namespace stream {
namespace functional {

isl_domain_map isl_domain_constructor::map(expr_ptr e, const isl::context & ctx)
{
    auto space = isl::space(ctx, isl::set_tuple());
    m_map.domain = isl::set::universe(space);

    visit(e);

    return m_map;
}

void isl_domain_constructor::visit_array(const shared_ptr<array> & arr)
{
    for (auto & v : arr->vars)
        add_variable(v);

    visitor::visit_array(arr);
}

void isl_domain_constructor::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto v = dynamic_pointer_cast<array_var>(ref->var))
    {
        add_variable(v);
    }
}

void isl_domain_constructor::add_variable(array_var_ptr av)
{
    if ( std::find(m_map.vars.begin(), m_map.vars.end(), av)
         != m_map.vars.end() )
        return;

    m_map.vars.push_back(av);
    m_map.domain.add_dimensions(isl::space::variable, 1);

    int dim = m_map.vars.size() - 1;
    auto dv = m_map.domain.get_space().var(dim);

    m_map.domain.add_constraint(dv >= 0);

    if (auto upper_bound = dynamic_pointer_cast<int_const>(av->range.expr))
    {
        m_map.domain.add_constraint(dv < upper_bound->value);
    }
}


array_bounding::array_bounding():
    m_printer(m_ctx)
{
    //verbose<array_bounding>::enabled() = true;
}

void
array_bounding::bound(array_ptr a)
{
    m_inferred.clear();
    m_domain = stack<isl::set>();

    {
        isl_domain_constructor c;
        auto map = c.map(a, m_ctx);
        m_space_map.space = map.domain.get_space();
        m_space_map.vars = map.vars;
        m_domain.push(map.domain);
    }

    for (int i = 0; i < a->vars.size(); ++i)
    {
        bool is_unbound = (bool) dynamic_pointer_cast<infinity>(a->vars[i]->range.expr);
        if (!is_unbound)
            continue;

        if (verbose<array_bounding>::enabled())
            cout << "Will try to bound dimension " << i << endl;

        m_inferred.push_back({i, isl::set(m_space_map.space)});
    }

    if (m_inferred.empty())
    {
        if (verbose<array_bounding>::enabled())
            cout << "No dimensions to bound." << endl;
        return;
    }

    visit(a);

    for (auto & inferred : m_inferred)
    {
        auto avar = m_space_map.vars[inferred.index];

        if (inferred.invalid_domain.is_empty())
        {
            if (verbose<array_bounding>::enabled())
                cout << "Dimension " << inferred.index << " unbounded." << endl;
            continue;
        }

        auto var = m_space_map.space.var(inferred.index);
        auto min = inferred.invalid_domain.minimum(var).integer();

        avar->range =
                make_shared<int_const>(min);

        if (verbose<array_bounding>::enabled())
            cout << "Dimension " << inferred.index << " bounded by " << min << endl;

        if (min <= 0)
        {
            ostringstream msg;
            msg << "Size in dimension " << inferred.index
                << " is bounded to " << min << " by indexing expressions.";
            throw source_error(msg.str(), a->location);
        }
    }
}

void array_bounding::visit_array(const shared_ptr<array> & a)
{
    // FIXME: visit local IDs in the context of patterns from which they come
    for(auto & id : a->scope.ids)
        visit(id->expr);

    auto domain = current_domain();

    auto patterns_expr = dynamic_pointer_cast<array_patterns>(a->expr.expr);
    assert(patterns_expr);

    for (auto & pattern : patterns_expr->patterns)
    {
        auto pattern_dom = domain;

        for (int dim = 0; dim < pattern.indexes.size(); ++dim)
        {
            auto & index = pattern.indexes[dim];
            if (!index.is_fixed)
                continue;

            auto var = m_space_map.var(a->vars[dim]);
            pattern_dom.add_constraint(var == index.value);
        }

        if (verbose<array_bounding>::enabled())
        {
            cout << "Pattern domain: "; m_printer.print(pattern_dom); cout << endl;
        }

        auto stacked_pattern_dom = stack_scoped(pattern_dom, m_domain);

        if (pattern.domains)
            visit(pattern.domains);

        visit(pattern.expr);
    }
}

void array_bounding::visit_cases(const shared_ptr<case_expr> & e)
{
    for (auto & c : e->cases)
    {
        auto & constraint = c.first;
        auto & value = c.second;

        auto domain = current_domain();

        if (constraint)
        {
            auto const_domain = to_affine_set(constraint, m_space_map);

            domain &= const_domain;
        }

        if (verbose<array_bounding>::enabled())
        {
            cout << "Guard domain: ";
            m_printer.print(domain);
            cout << endl;
        }

        auto stacked_domain = push_domain(domain);

        visit(value);
    }
}

void array_bounding::visit_array_app(const shared_ptr<array_app> & a)
{
    if (!a->object->type || !a->object->type->array())
        return;

    auto ar_size = a->object->type->array()->size;

    for (int dim = 0; dim < a->args.size(); ++dim)
    {
        if (ar_size[dim] < 0)
            continue;

        auto & arg = a->args[dim];

        auto expr = to_affine_expr(arg, m_space_map);

        if (verbose<array_bounding>::enabled())
        {
            cout << "Indexing expression: "; m_printer.print(expr); cout << endl;
        }

        inferred_dim * inferred = nullptr;

        for (auto & candidate : m_inferred)
        {
            if (!expr.involves(isl::space::input, candidate.index))
                continue;

            if (!inferred)
                inferred = &candidate;
            else
                throw source_error("Indexing expression involves multiple inferred dimensions.",
                                   arg.location);
        }

        if (!inferred)
            continue;

        int involved_unbounded_dims = 0;

        for (int i = 0; i < (int)m_space_map.vars.size(); ++i)
        {
            if (!expr.involves(isl::space::input, i))
                continue;

            auto v = m_space_map.vars[i];
            bool is_bounded =
                    dynamic_pointer_cast<int_const>(v->range.expr) != nullptr;

            if (!is_bounded)
                ++involved_unbounded_dims;
        }

        if (involved_unbounded_dims > 1)
            throw source_error("Indexing expression involves multiple unbounded dimensions.",
                               arg.location);

        auto invalid_dom = current_domain();

        auto constraint = expr >= ar_size[dim];
        invalid_dom.add_constraint(constraint);

        inferred->invalid_domain |= invalid_dom;

        if (verbose<array_bounding>::enabled())
        {
            cout << "Adding invalid domain: "; m_printer.print(invalid_dom); cout << endl;
            cout << "New invalid domain: "; m_printer.print(inferred->invalid_domain); cout << endl;
        }
    }
}

isl::set array_bounding::current_domain()
{
    if (m_domain.size())
        return m_domain.top();
    else
        return isl::set::universe(m_space_map.space);
}

stacker<isl::set, std::stack<isl::set>>
array_bounding::push_domain(const isl::set & d)
{
    return stack_scoped(d, m_domain);
}

}
}
