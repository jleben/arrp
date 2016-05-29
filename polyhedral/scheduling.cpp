/*
Compiler for language for stream processing

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "scheduling.hpp"
#include "../common/error.hpp"

#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <isl-cpp/space.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/matrix.hpp>
#include <isl-cpp/utility.hpp>
#include <isl-cpp/printer.hpp>
#include <isl-cpp/schedule.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <limits>
#include <cmath>
#include <cstdlib>

using namespace std;

namespace stream {

static int gcd(int a, int b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

static int lcm(int a, int b)
{
    int div = gcd(a, b);
    return div ? (a / div * b) : 0;
}

namespace polyhedral {

statement *statement_for( const isl::identifier & id )
{
    return reinterpret_cast<statement*>(id.data);
}

array * array_for( const isl::identifier & id)
{
    return reinterpret_cast<array*>(id.data);
}

scheduler::scheduler( model & m ):
    m_printer(m.context),
    m_model(m),
    m_model_summary(m)
{}

polyhedral::schedule
scheduler::schedule(bool optimize, const vector<reversal> & reversals)
{
    if (verbose<scheduler>::enabled())
        cout << endl << "### Scheduling ###" << endl;

    if (verbose<scheduler>::enabled())
    {
        cout << "Domains:" << endl;
        print_each_in(m_model_summary.domains);
        cout << "Write relations:" << endl;
        print_each_in(m_model_summary.write_relations);
        cout << "Read relations:" << endl;
        print_each_in(m_model_summary.read_relations);
        cout << "Dependencies:" << endl;
        print_each_in(m_model_summary.dependencies);
    }

    if (verbose<polyhedral::model>::enabled())
    {
        cout << endl << "### Domains: ###" << endl;
        m_printer.print(m_model_summary.domains);
        cout << endl;

        cout << endl << "### Dependencies:" << endl;
        m_printer.print(m_model_summary.dependencies);
        cout << endl;
    }

    polyhedral::schedule schedule(m_model.context);

    schedule.tree = make_schedule(m_model_summary.domains,
                                   m_model_summary.dependencies,
                                   optimize);

    schedule.full = schedule.tree.map().in_domain(m_model_summary.domains);

    if (verbose<scheduler>::enabled())
    {
        cout << "Schedule:" << endl;
        isl_printer_print_schedule(m_printer.get(), schedule.tree.get());
        cout << endl;

        cout << endl << "Schedule map:" << endl;
        print_each_in(schedule.full);
        cout << endl;
    }

    if (verbose<polyhedral::model>::enabled())
    {
        cout << endl << "### Schedule: ###" << endl;
        m_printer.print(schedule.full);
        cout << endl;
    }

    make_periodic_schedule(schedule);

    if (verbose<scheduler>::enabled())
    {
        cout << "Prelude schedule:" << endl;
        print_each_in(schedule.prelude);
        cout << endl;

        cout << "Periodic schedule:" << endl;
        print_each_in(schedule.period);
        cout << endl;
    }

    for (auto & reversal : reversals)
    {
        cout << "Reversing: " << reversal.stmt_name
             << " @ " << reversal.dim << endl;

        stmt_ptr stmt;
        for (auto & s : m_model.statements)
        {
            if (s->name == reversal.stmt_name)
            {
                stmt = s;
                break;
            }
        };

        if (!stmt)
            throw error("Can not reverse schedule: No statement named " + reversal.stmt_name);

        auto sched = schedule.period.in_domain(stmt->domain);

        auto union_sched_range = sched.range();
        isl::space sched_space(nullptr);
        union_sched_range.for_each([&](const isl::set & s){
            sched_space = s.get_space();
            return false;
        });

        if (reversal.dim < 0 || reversal.dim >= sched_space.dimension(isl::space::variable))
            throw error("Schedule dimension out of range: ");

        auto sched_range = union_sched_range.set_for(sched_space);
        auto t = sched_space.var(reversal.dim);
        auto min_t = sched_range.minimum(t);
        auto max_t = sched_range.maximum(t);

        if (min_t.is_infinity() || max_t.is_infinity())
            throw error("Schedule dimension is infinite.");

        cout << "Schedule dimension range: " << min_t.integer() << "," << max_t.integer() << endl;

#if 1
        {
            // t2 = max + min - t1;
            // t1 = max + min - t2;

            auto sched_map_space = isl::space::from(sched_space, sched_space);

            int dim = reversal.dim;

            isl_multi_aff * transform =
                    isl_multi_aff_identity(sched_map_space.copy());

            isl_aff * out = isl_multi_aff_get_aff(transform, dim);
            out = isl_aff_set_coefficient_si(out, isl_dim_in, dim, -1);
            out = isl_aff_set_constant_si(out, max_t.integer() + min_t.integer());

            transform = isl_multi_aff_set_aff(transform, dim, out);

            schedule.period.subtract(sched);

            schedule.period = schedule.period |
                    isl_union_map_preimage_range_multi_aff
                    (sched.copy(), transform);
        }
#endif
    }

    if (verbose<scheduler>::enabled())
    {
        cout << "Periodic schedule with reversals:" << endl;
        print_each_in(schedule.period);
        cout << endl;
    }

    return schedule;
}


isl_bool print_schedule_node_info(isl_schedule_node *node, void *user)
{
    auto type = isl_schedule_node_get_type(node);
    if (type == isl_schedule_node_band)
    {
        cout << "band is permutable = "
             << isl_schedule_node_band_get_permutable(node) << endl;
    }
    else
    {
        cout << "not a band" << endl;
    }

    return isl_bool_true;
}


isl_bool
scheduler::add_schedule_constraints
(struct isl_scheduler * sched)
{
    // Schedule validity test
    // FIXME: It is possible that the schedule contains a
    // "set" node before a "band" node.

    int sched_dim = isl_scheduler_get_current_dim(sched);
    if (sched_dim != 0)
        return isl_bool_false;

    if (verbose<scheduler>::enabled())
        cout << "Adding schedule constraints for first dimension..." << endl;

    // FIXME: current node not accessible anymore
#if 0
    {
        isl_schedule_node * node = isl_scheduler_get_current_node(sched);
        while(node)
        {
            auto type = isl_schedule_node_get_type(node);
            if (type == isl_schedule_node_sequence)
            {
                cerr << "The schedule has a sequence node before a band node." << endl;
                isl_schedule_node_free(node);
                return isl_bool_error;
            }
            if (!isl_schedule_node_has_parent(node))
                break;
            node = isl_schedule_node_parent(node);
        }
        isl_schedule_node_free(node);
    }
#endif

    // FIXME: Add constraint:
    // Streaming coefficients for dependent statements are equal to
    // the ratio of the amounts of array they consume/produce in
    // a single streaming iteration.

    // Add constraint:
    // The coefficient for streaming statement dimension > 0.

    isl::basic_set coefs(isl_scheduler_get_user_coefs(sched));
    auto cnstr_space = isl::local_space(coefs.get_space());

    bool new_constraints = false;

    for (auto & stmt : m_model.statements)
    {
        if (!stmt->is_infinite)
            continue;

        int coef_pos = isl_scheduler_get_user_coef_pos
                (sched, stmt->domain.get_space().get());
        if (coef_pos < 0)
        {
            continue;
        }

        if (verbose<scheduler>::enabled())
            cout << ".. Adding constraints for statement: " << stmt->name << endl;

        int n_params = stmt->domain.get_space().dimension(isl::space::parameter);
        int var_coef_pos = coef_pos + n_params;
        auto coef = cnstr_space(isl::space::variable, var_coef_pos);
        coefs.add_constraint(coef > 0);
        new_constraints = true;
    }

    if (!new_constraints)
        return isl_bool_false;

    isl_scheduler_set_user_coefs(sched, coefs.copy());
    return isl_bool_true;
}

isl::schedule scheduler::make_schedule
(const isl::union_set & domains, const isl::union_map & dependencies,
 bool optimize)
{
    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    isl_options_set_schedule_whole_component(domains.ctx().get(), 0);

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());

    constr = isl_schedule_constraints_set_constraint_filter
            (constr, &add_schedule_constraints_helper, this);

    constr = isl_schedule_constraints_set_validity(constr, dependencies.copy());

    if (optimize)
    {
        auto proximity_deps = make_proximity_dependencies(dependencies);
        constr = isl_schedule_constraints_set_proximity(constr, proximity_deps.copy());
        //constr = isl_schedule_constraints_set_coincidence(constr, proximity_deps.copy());
    }

    isl_schedule * sched =
            isl_schedule_constraints_compute_schedule(constr);
    assert(sched);

    return sched;
}

isl::union_map
scheduler::make_proximity_dependencies(const isl::union_map & dependencies)
{
    // Collect sensible proximity dependencies.
    // Drop those which can not possible be satisfied
    // = a source has infinite number of sinks

    isl::union_map proximity_deps(dependencies.ctx());

    dependencies.for_each([&](const isl::map & m)
    {
        m.for_each([&](const isl::basic_map & dep)
        {
            int source_dim = m.get_space().dimension(isl::space::input);
            int sink_dim = m.get_space().dimension(isl::space::output);

            auto sink_pairs = (dep * dep).wrapped();

            isl::local_space space(sink_pairs.get_space());
            auto sink_a = space(isl::space::variable, source_dim);
            auto sink_b = space(isl::space::variable, source_dim + sink_dim);
            auto max_sink_distance = sink_pairs.maximum(sink_b - sink_a);

            if (!max_sink_distance.is_infinity())
            {
                if (verbose<scheduler>::enabled()) {
                    cout << "Accepting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }

                proximity_deps = proximity_deps | dep;
            }
            else
            {
                if (verbose<scheduler>::enabled()) {
                    cout << "Rejecting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }
            }

            return true;
        });
        return true;
    });

    return proximity_deps;
}

void
scheduler::make_periodic_schedule(polyhedral::schedule & sched)
{
    isl_schedule_node * domain_node = isl_schedule_get_root(sched.tree.get());
    assert_or_throw(isl_schedule_node_get_type(domain_node) == isl_schedule_node_domain);

    isl_schedule_node * root = isl_schedule_node_get_child(domain_node, 0);
    assert_or_throw(root != nullptr);

    isl_schedule_node * infinite_band = nullptr;
    bool root_is_sequence = false;
    int root_seq_elems = 0;

    auto node_is_infinite = [&sched](isl_schedule_node * node) -> bool
    {
        isl::union_set domain = isl_schedule_node_get_domain(node);
        domain = domain & sched.full.domain();
        bool is_infinite = false;
        domain.for_each([&](const isl::set & stmt_domain){
            auto i0 = stmt_domain.get_space().var(0);
            auto i0_max = stmt_domain.maximum(i0);
            if (i0_max.is_infinity())
            {
                is_infinite = true;
                return false;
            }
            return true;
        });
        return is_infinite;
    };

    auto root_type = isl_schedule_node_get_type(root);
    if (root_type == isl_schedule_node_band)
    {
        if (node_is_infinite(root))
            infinite_band = isl_schedule_node_copy(root);
    }
    else
    {
        assert_or_throw(root_type == isl_schedule_node_sequence);
        root_is_sequence = true;
        int elem_count = root_seq_elems = isl_schedule_node_n_children(root);
        for (int i = 0; i < elem_count; ++i)
        {
            if (infinite_band)
            {
                throw error("Schedule sequence contains infinite element that is not last.");
            }

            isl_schedule_node * filter = isl_schedule_node_get_child(root, i);
            assert_or_throw(filter != nullptr);
            assert_or_throw(isl_schedule_node_get_type(filter) == isl_schedule_node_filter);

            if (isl_schedule_node_has_children(filter))
            {
                isl_schedule_node * child = isl_schedule_node_get_child(filter, 0);
                assert_or_throw(child != nullptr);
                auto type = isl_schedule_node_get_type(child);
                if (type == isl_schedule_node_band)
                {
                    if (node_is_infinite(child))
                        infinite_band = isl_schedule_node_copy(child);
                }
                else
                {
                    assert_or_throw(type == isl_schedule_node_leaf);
                }
                isl_schedule_node_free(child);
            }

            isl_schedule_node_free(filter);
        }
    }

    if (infinite_band)
    {
        isl::union_map infinite_sched =
                isl_schedule_node_get_subtree_schedule_union_map(infinite_band);

        int prelude_dur = compute_prelude_duration(infinite_sched, sched.full.domain());

        int period_dur = compute_period_duration(infinite_sched);

        infinite_sched = infinite_sched.in_domain(sched.full.domain());

        isl::union_set prelude_dom(sched.full.ctx()), period_dom(sched.full.ctx());

        if (prelude_dur > 0)
        {
            // Add prelude part of the infinite schedule part.
            infinite_sched.for_each([&](isl::map & m)
            {
                m.limit_above(isl::space::output, 0, prelude_dur - 1);
                prelude_dom = prelude_dom | m.domain();
                return true;
            });
        }

        if (root_is_sequence && root_seq_elems >= 2)
        {
            // Add all finite sequence elements (all other than the last).
            sched.full.for_each([&](isl::map & m){
                m.limit_above(isl::space::output, 0, root_seq_elems - 2);
                prelude_dom = prelude_dom | m.domain();
                return true;
            });
        }

        {
            infinite_sched.for_each([&](isl::map & m)
            {
                m.limit_below(isl::space::output, 0, prelude_dur);
                m.limit_above(isl::space::output, 0, prelude_dur + period_dur - 1);
                period_dom = period_dom | m.domain();
                return true;
            });
        }

        sched.prelude_tree = sched.tree;
        sched.prelude_tree.intersect_domain(prelude_dom);
        sched.prelude = sched.prelude_tree.map().in_domain(prelude_dom);

        sched.period_tree = sched.tree;
        sched.period_tree.intersect_domain(period_dom);
        sched.period = sched.period_tree.map().in_domain(period_dom);
    }
    else
    {
        sched.prelude_tree = sched.tree;
        sched.prelude = sched.full;
    }

    infinite_band = isl_schedule_node_free(infinite_band);
    root = isl_schedule_node_free(root);
    domain_node = isl_schedule_node_free(domain_node);
}

#if 1
int scheduler::compute_period_duration
(const isl::union_map & schedule)
{
    if (verbose<scheduler>::enabled())
        cout << endl << "Computing period duration.." << endl;

    int common_period = 0;

    isl::union_set unbounded_domains(m_model.context);

    m_model_summary.domains.for_each([&](const isl::set & s){
        auto id = s.id();
        auto stmt = statement_for(id);
        assert(stmt);

        if (!stmt->is_infinite)
            return true;

        s.for_each([&](isl::basic_set & bs){

            isl::basic_set unbounded =
                    isl::basic_set::universe(bs.get_space());

            bs = isl_basic_set_detect_equalities(bs.copy());

            // Only add constraints that don't involve infinite dim,
            // or that are equalities.

            auto constraints =
                    isl_basic_set_get_constraint_list(bs.get());
            auto n = isl_constraint_list_n_constraint(constraints);
            for (int i = 0; i < n; ++i)
            {
                isl::constraint c =
                        isl_constraint_list_get_constraint(constraints, i);
                if (!isl_constraint_involves_dims(c.get(), isl_dim_set, 0, 1))
                    unbounded.add_constraint(c);
                else if (c.is_equality())
                    unbounded.add_constraint(c);
            };
            isl_constraint_list_free(constraints);

            unbounded_domains = unbounded_domains | unbounded;

            return true;
        });

        return true;
    });

    if (false && verbose<scheduler>::enabled())
    {
        cout << "Unbounded domains:" << endl;
        m_printer.print_each_in(unbounded_domains);
    }

    auto deps =
            m_model_summary.read_relations.in_domain(unbounded_domains).inverse()
            (m_model_summary.write_relations.in_domain(unbounded_domains));

    deps.map_domain_through(schedule);
    deps.map_range_through(schedule);

    vector<int> periods;

    deps.for_each( [&](const isl::map & m ){
        m.for_each( [&](const isl::basic_map & bm){

            if (verbose<scheduler>::enabled())
            {
                cout << "Time dep: "; m_printer.print(bm); cout << endl;
            }

            isl::matrix eq =
                    isl_basic_map_equalities_matrix
                    (bm.get(),
                     isl_dim_in,
                     isl_dim_out,
                     isl_dim_div,
                     isl_dim_param,
                     isl_dim_cst);

            isl::matrix ineq =
                    isl_basic_map_inequalities_matrix
                    (bm.get(),
                     isl_dim_in,
                     isl_dim_out,
                     isl_dim_div,
                     isl_dim_param,
                     isl_dim_cst);

            assert(eq.column_count() == ineq.column_count());

            // Combine eq and ineq, and drop column for constants

            isl::matrix c(m_model.context,
                          eq.row_count() + ineq.row_count(),
                          eq.column_count() - 1);

            for (auto row = 0; row < eq.row_count(); ++row)
            {
                for (auto col = 0; col < c.column_count(); ++col)
                {
                    c(row,col) = eq(row,col).value();
                }
            }
            for (auto row = 0; row < ineq.row_count(); ++row)
            {
                for (auto col = 0; col < c.column_count(); ++col)
                {
                    int offset_row = row + eq.row_count();
                    c(offset_row,col) = ineq(row,col).value();
                }
            }

            auto n = c.nullspace();

            if (false && verbose<scheduler>::enabled())
            {
                cout << "Nullspace:" << endl;
                isl::print(n);
                cout << endl;
            }

            int n_dim = bm.get_space().dimension(isl::space::input);

            assert_or_throw(n.column_count() == 1);

            auto t_src = n(0,0).value();
            auto t_dst = n(n_dim,0).value();

            assert_or_throw(t_src.is_integer());
            assert_or_throw(t_dst.is_integer());
            assert_or_throw(t_src.integer() == t_dst.integer());

            if (verbose<scheduler>::enabled())
                cout << "Period = " << t_src.integer() << endl;

            periods.push_back(t_src.integer());

            return true;
        });
        return true;
    });

    common_period = 1;
    for (const auto & d : periods)
    {
        common_period = lcm(common_period, d);
    }

    if (verbose<scheduler>::enabled())
        cout << "Common period = " << common_period << endl;


    auto & m = m_model_summary;
    auto write_sched = m.write_relations.in_domain(unbounded_domains);
    write_sched.map_domain_through(schedule);

    write_sched.for_each([&](isl::map ws){

        int a1 = 0, a2 = 0;

        {
            ws = isl_map_lower_bound_si(ws.copy(), isl_dim_in, 0, 0);
            auto a = ws.range();
            a1 = a.minimum(a.get_space().var(0)).integer();
        }
        {
            ws = isl_map_lower_bound_si(ws.copy(), isl_dim_in, 0, common_period);
            auto a = ws.range();
            a2 = a.minimum(a.get_space().var(0)).integer();
        }

        auto period = a2 - a1;

        auto ar = array_for(ws.id(isl::space::output));

        if (verbose<scheduler>::enabled())
            cout << "Array " << ar->name << " period = " << period << endl;

        ar->period = period;

        return true;
    });

    return common_period;
}
#endif

int scheduler::compute_prelude_duration(const isl::union_map & schedule,
                                        const isl::union_set & domains)
{
    using namespace isl;

    // Find time in schedule after which
    // the periodic schedule can be extracted.
    // The schedule before this time will become the "prelude."

    // That is,
    // find earliest time by which all finite statements are completed,
    // and all infinite statements are in steady state.
    // Steady state = the infinite scheduling hyperplane does not
    // cut through any hyperplane bounding the infinite dimension
    // of the domain.

    if (verbose<scheduler>::enabled())
        cout << endl << "Computing prelude duration..." << endl;

    int offset = std::numeric_limits<int>::min();

    auto bounded_schedule = schedule.in_domain(domains);
    bounded_schedule.for_each( [&](map & stmt_sched)
    {
        auto id = stmt_sched.id(isl::space::input);
        auto stmt = statement_for(id);
        auto stmt_unbounded_sched = schedule.map_for(stmt_sched.get_space());

        if (stmt->is_infinite)
        {
            stmt_sched.for_each([&](basic_map & s)
            {
                // NOTE: Assuming each basic map is infinite too.

                auto domain = s.domain();

                domain = isl_basic_set_detect_equalities(domain.copy());

                if (verbose<scheduler>::enabled())
                {
                    cout << "----" << endl;
                    cout << "Domain: ";
                    m_printer.print(domain); cout << endl;
                }

                isl::basic_set domain_inf_hull =
                        isl_basic_set_drop_constraints_involving_dims
                        (domain.copy(), isl_dim_set, 0, 1);

                if (verbose<scheduler>::enabled())
                {
                    cout << "No constraints: ";
                    m_printer.print(domain_inf_hull); cout << endl;
                }

                auto constraints =
                        isl_basic_set_get_constraint_list(domain.get());
                auto n = isl_constraint_list_n_constraint(constraints);

                auto domain_with_eqs = domain_inf_hull;
                for (int i = 0; i < n; ++i)
                {
                    isl::constraint c =
                            isl_constraint_list_get_constraint(constraints, i);
                    if (!isl_constraint_involves_dims(c.get(), isl_dim_set, 0, 1))
                        continue;
                    if (c.is_equality())
                        domain_with_eqs.add_constraint(c);
                };

                auto domain_with_ineqs = domain_inf_hull;
                for (int i = 0; i < n; ++i)
                {
                    isl::constraint c =
                            isl_constraint_list_get_constraint(constraints, i);
                    if (!isl_constraint_involves_dims(c.get(), isl_dim_set, 0, 1))
                        continue;
                    if (!c.is_equality())
                        domain_with_ineqs.add_constraint(c);
                };

                isl_constraint_list_free(constraints);

                if (verbose<scheduler>::enabled())
                {
                    cout << "Eqs: ";
                    m_printer.print(domain_with_eqs); cout << endl;
                    cout << "Ineqs: ";
                    m_printer.print(domain_with_ineqs); cout << endl;
                }

                auto not_in_domain = isl::set(domain_with_eqs) - domain_with_ineqs;

                if (verbose<scheduler>::enabled())
                {
                    cout << "Not in domain: ";
                    m_printer.print(not_in_domain); cout << endl;
                }

                auto prelude_sched = stmt_unbounded_sched(not_in_domain);

                if (verbose<scheduler>::enabled())
                {
                    cout << "Prelude sched: ";
                    m_printer.print(prelude_sched); cout << endl;
                }

                auto t = prelude_sched.get_space().var(0);
                auto latest_prelude = prelude_sched.maximum(t);

                assert(latest_prelude.is_integer());

                if (verbose<scheduler>::enabled())
                    cout << "Latest: " << latest_prelude.integer() << endl;

                offset = std::max(offset, (int) latest_prelude.integer() + 1);

                if (verbose<scheduler>::enabled())
                    cout << ">> Offset: " << offset << endl;

                return true;
            });
        }
        else
        {
            auto range = stmt_sched.range();
            auto latest_time = range.maximum(range.get_space().var(0));
            assert(latest_time.is_integer());
            offset = std::max(offset, (int) latest_time.integer() + 1);
        }
        return true;
    });

    if (verbose<scheduler>::enabled())
    {
        cout << "Prelude duration = " << offset << endl;
    }

    return offset;
}

isl::union_map scheduler::prelude_schedule
(const isl::union_map & schedule, int prelude)
{
    using namespace isl;

    isl::union_map sched_prelude(m_model.context);

    schedule.for_each([&](isl::map & stmt_sched)
    {
        auto stmt = statement_for(stmt_sched.id(isl::space::input));
        assert(stmt);

        if (!stmt->is_infinite)
        {
            sched_prelude = sched_prelude | stmt_sched;
            return true;
        }

        auto range = stmt_sched.range();
        auto time = range.get_space()(isl::space::variable, 0);
        range.add_constraint(time < prelude);

        stmt_sched = stmt_sched.in_range(range);

        sched_prelude = sched_prelude | stmt_sched;
        return true;
    });

    return sched_prelude;
}

isl::union_map scheduler::periodic_schedule
(const isl::union_map & schedule, int prelude, int period)
{
    using namespace isl;

    isl::union_map sched_period(m_model.context);

    schedule.for_each([&](isl::map & stmt_sched)
    {
        auto stmt = statement_for(stmt_sched.id(isl::space::input));
        assert(stmt);

        if (!stmt->is_infinite)
            return true;

        auto range = stmt_sched.range();
        auto time = range.get_space()(isl::space::variable, 0);
        range.add_constraint(time >= prelude);
        range.add_constraint(time < (prelude + period));

        stmt_sched = stmt_sched.in_range(range);

        // FIXME: Check correctness of the following:
#if 0
        // Compute domain indexes corresponding to schedule period
        auto domain = stmt_sched.domain();
        auto i = domain.get_space()(space::variable, 0);
        auto min_i = domain.minimum(i);
        assert(min_i.is_integer());

        // Translate domain to 0
        if (min_i.integer() != 0)
        {
            auto domain_map_space =
                    space::from(domain.get_space(), domain.get_space());
            isl_multi_aff * translation =
                    isl_multi_aff_identity(domain_map_space.copy());
            isl_aff * i = isl_multi_aff_get_aff(translation, 0);
            i = isl_aff_set_constant_val(i, min_i.copy());
            isl_multi_aff_set_aff(translation, 0, i);

            auto translated_sched =
                    isl_map_preimage_domain_multi_aff
                    (stmt_sched.copy(), translation);

            stmt_sched = translated_sched;
        }

        // store original offset
        int offset = min_i.integer();
        if (stmt->write_relation.array)
        {
            auto & array = stmt->write_relation.array;
            if (!array->period_offset)
                array->period_offset = offset;
            else if (array->period_offset != offset)
                cerr << "WARNING: different offsets for same buffer." << endl;
        }
#endif
        sched_period = sched_period | stmt_sched;

        return true;
    });

    return sched_period;
}

void scheduler::print_each_in( const isl::union_set & us )
{
    us.for_each ( [&](const isl::set & s){
       m_printer.print(s);
       cout << endl;
       return true;
    });
}

void scheduler::print_each_in( const isl::union_map & um )
{
    um.for_each ( [&](const isl::map & m){
       m_printer.print(m);
       cout << endl;
       return true;
    });
}

}
}
