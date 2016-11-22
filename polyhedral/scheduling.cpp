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
#include "utility.hpp"
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
        m_printer.print_each_in(m_model_summary.domains);
        cout << "Write relations:" << endl;
        m_printer.print_each_in(m_model_summary.write_relations);
        cout << "Read relations:" << endl;
        m_printer.print_each_in(m_model_summary.read_relations);
        cout << "Dependencies:" << endl;
        m_printer.print_each_in(m_model_summary.dependencies);
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
        cout << endl << "Schedule:" << endl;
        isl_printer_print_schedule(m_printer.get(), schedule.tree.get());
        cout << endl;

        cout << endl << "Schedule map:" << endl;
        m_printer.print_each_in(schedule.full);
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
        cout << endl << "Prelude schedule:" << endl;
        m_printer.print_each_in(schedule.prelude);

        cout << endl << "Periodic schedule:" << endl;
        m_printer.print_each_in(schedule.period);

        cout << endl << "Tiled schedule:" << endl;
        m_printer.print_each_in(schedule.tiled);

        bool ok;

        cout << "Validating full schedule:" << endl;
        ok = validate_schedule(schedule.full);
        cout << (ok ? "Valid." : "Invalid!") << endl;

        cout << "Validating prelude schedule:" << endl;
        ok = validate_schedule(schedule.prelude);
        cout << (ok ? "Valid." : "Invalid!") << endl;

        cout << "Validating period schedule:" << endl;
        ok = validate_schedule(schedule.period);
        cout << (ok ? "Valid." : "Invalid!") << endl;

        cout << "Validating tiled schedule:" << endl;
        ok = validate_schedule(schedule.tiled);
        cout << (ok ? "Valid." : "Invalid!") << endl;
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

    if (verbose<scheduler>::enabled() && !reversals.empty())
    {
        cout << "Periodic schedule with reversals:" << endl;
        m_printer.print_each_in(schedule.period);
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

#if 0
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
#endif
isl::schedule scheduler::make_schedule
(const isl::union_set & domains, const isl::union_map & dependencies,
 bool optimize)
{
    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    isl_options_set_schedule_whole_component(domains.ctx().get(), m_schedule_whole);

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());
#if 0
    constr = isl_schedule_constraints_set_constraint_filter
            (constr, &add_schedule_constraints_helper, this);
#endif
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

        if (verbose<scheduler>::enabled())
        {
          cout << "Infinite part of schedule:" << endl;
          m_printer.print_each_in(infinite_sched);
          cout << endl;
        }


        auto tiling = find_periodic_tiling(infinite_sched);

        infinite_sched = infinite_sched.in_domain(sched.full.domain());

        // Extract statement domains for prelude and period

        isl::union_set prelude_dom(sched.full.ctx()), period_dom(sched.full.ctx());

        if (tiling.offset > 0)
        {
            // Add prelude part of the infinite schedule part.
            // = stream dim < tiling.offset
            infinite_sched.for_each([&](isl::map & m)
            {
                m.limit_above(isl::space::output, tiling.dim, tiling.offset - 1);
                prelude_dom = prelude_dom | m.domain();
                return true;
            });
        }

        if (root_is_sequence && root_seq_elems >= 2)
        {
            // Add all finite sequence elements (all other than the last).
            // = sequence index (zero dim of full schedule) < (#sequence - 1)
            sched.full.for_each([&](isl::map & m){
                m.limit_above(isl::space::output, 0, root_seq_elems - 2);
                prelude_dom = prelude_dom | m.domain();
                return true;
            });
        }

        {
            // Extract period domains
            // = prelude dur <= stream dim < tiling.offset + tiling.size
            infinite_sched.for_each([&](isl::map & m)
            {
                m.limit_below(isl::space::output, tiling.dim, tiling.offset);
                m.limit_above(isl::space::output, tiling.dim, tiling.offset + tiling.size - 1);
                period_dom = period_dom | m.domain();
                return true;
            });
        }


        // NOTE: Derive maps from sched.full instead of
        // from trees, to ensure common space.

        sched.prelude_tree = sched.tree;
        sched.prelude_tree.intersect_domain(prelude_dom);
        sched.prelude = sched.full.in_domain(prelude_dom);

        sched.period_tree = sched.tree;
        sched.period_tree.intersect_domain(period_dom);
        sched.period = sched.full.in_domain(period_dom);

        // Create tiled schedule, used for storage allocation.

        {
            isl::union_map tiled(m_model.context);

            isl::space sched_space(nullptr);
            sched.full.for_each([&](isl::map & m)
            {
                sched_space = m.get_space().range();
                return false;
            });
            int n_dim = sched_space.dimension(isl::space::variable);

            int full_stream_dim = tiling.dim;
            if (root_is_sequence)
                full_stream_dim += 1;

            // Tile periodic part

            {
                isl::union_set periodic_dom(m_model.context);
                infinite_sched.for_each([&](isl::map & m)
                {
                    m.limit_below(isl::space::output, tiling.dim, tiling.offset);
                    periodic_dom = periodic_dom | m.domain();
                    return true;
                });

                tiled = sched.full.in_domain(periodic_dom);

                auto tiling_expr = isl::multi_expression::zero(sched_space, n_dim+1);
                for (int i = 0; i < n_dim; ++i)
                    tiling_expr.set(i+1, sched_space.var(i));
                auto tile_index_expr =
                        isl::floor((sched_space.var(full_stream_dim) - tiling.offset)
                                   / tiling.size) + 1;
                tiling_expr.set(0, tile_index_expr);

                isl::map tiling_map(isl_map_from_multi_aff(tiling_expr.copy()));
                if (false && verbose<scheduler>::enabled())
                {
                    cout << "Periodic tiling: " << endl;
                    m_printer.print(tiling_expr);  cout << endl;
                    m_printer.print(tiling_map);  cout << endl;
                }

                tiled.map_range_through(tiling_map);

                if (false && verbose<scheduler>::enabled())
                {
                    cout << "Periodic tiles: " << endl;
                    m_printer.print_each_in(tiled);
                }
            }

            // Add the prelude tile

            {
                auto tiling_expr = isl::multi_expression::zero(sched_space, n_dim+1);
                for (int i = 0; i < n_dim; ++i)
                    tiling_expr.set(i+1, sched_space.var(i));
                tiling_expr.set(0, sched_space.val(0));

                isl::map tiling_map(isl_map_from_multi_aff(tiling_expr.copy()));
                auto prelude_tile = sched.prelude;
                prelude_tile.map_range_through(tiling_map);

                if (false && verbose<scheduler>::enabled())
                {
                    cout << "Prelude tile: " << endl;
                    m_printer.print_each_in(prelude_tile);
                }

                tiled = tiled | prelude_tile;
            }

            sched.tiled = tiled;
        }
    }
    else
    {
        sched.prelude_tree = sched.tree;
        sched.prelude = sched.tiled = sched.full;
    }

    infinite_band = isl_schedule_node_free(infinite_band);
    root = isl_schedule_node_free(root);
    domain_node = isl_schedule_node_free(domain_node);
}

scheduler::tiling
scheduler::find_periodic_tiling(const isl::union_map & schedule)
{
    if (verbose<scheduler>::enabled())
        cout << endl << "Finding periodic tiling." << endl;

    tiling periodic_tiling;
    periodic_tiling.dim = -1;
    periodic_tiling.offset = 0;
    periodic_tiling.size = 1;

    auto accesses = m_model_summary.write_relations | m_model_summary.read_relations;
    accesses = accesses.in_domain(m_model_summary.domains);
    auto access_schedules = accesses;
    access_schedules.map_domain_through(schedule);

    vector<access_info> access_infos = analyze_access_schedules(schedule);

    // Find common tiling dimension
    for (auto & access : access_infos)
    {
        if (access.time_period.empty())
            continue;

        // Find schedule tiling dimension (first dim where ray is not 0),
        // and time period.
        for (int dim = 0; dim < access.time_period.size(); ++dim)
        {
            if(access.time_period[dim] != 0)
            {
                // Make sure the dimension is identical for all access schedule
                if (periodic_tiling.dim == -1)
                    periodic_tiling.dim = dim;
                else if (periodic_tiling.dim != dim)
                    throw error("Inconsistent infinite directions of access schedules.");
                break;
            }
        }
    }

    // Find common tiling period
    for (auto & access : access_infos)
    {
        if (access.time_period.empty())
            continue;

        int access_period = access.time_period[periodic_tiling.dim];
        periodic_tiling.size = lcm(periodic_tiling.size, access_period);
    }

    // Find common data offsets
    for (auto & access : access_infos)
    {
        if (access.time_period.empty())
            continue;

        // Make sure infinite direction is parallel to first dimension of array.
        for (int dim = 1; dim < access.data_offset.size(); ++dim)
        {
            if (access.data_offset[dim] != 0)
                throw error("Access has unexpected infinite direction in data space.");
        }

        int periods_per_tile = periodic_tiling.size / access.time_period[periodic_tiling.dim];
        int offset = access.data_offset[0] * periods_per_tile;

        auto array = access.array;
        if (array->period == 0)
            array->period = offset;
        else if (array->period != offset)
            throw error("Accesses have inconsistent tile offsets in array space.");
    }

    // Find common period onset
    for (auto & access : access_infos)
    {
        int onset = find_period_onset(access, periodic_tiling.dim);
        periodic_tiling.offset = max(periodic_tiling.offset, onset);
    }

    if (verbose<scheduler>::enabled())
    {
        cout << "Periodic tiling:" << endl;
        cout << "Dimension = " << periodic_tiling.dim << endl;
        cout << "Offset = " << periodic_tiling.offset << endl;
        cout << "Size = " << periodic_tiling.size << endl;
    }

    return periodic_tiling;
}

vector<scheduler::access_info>
scheduler::analyze_access_schedules(const isl::union_map & schedule)
{
    auto accesses = m_model_summary.write_relations | m_model_summary.read_relations;
    accesses = accesses.in_domain(m_model_summary.domains);
    auto access_schedules = accesses;
    access_schedules.map_domain_through(schedule);

    vector<access_info> access_infos;

    for (auto & stmt : m_model.statements)
    {
        vector<array_relation> accesses;
        if (stmt->write_relation.array)
            accesses.push_back(stmt->write_relation);
        accesses.insert(accesses.end(), stmt->read_relations.begin(), stmt->read_relations.end());
        for (auto & access : accesses)
        {
            auto array = access.array;
            auto a = access.map.in_domain(stmt->domain).in_range(array->domain);

            if (verbose<scheduler>::enabled()) {
                cout << ".. Access: "; m_printer.print(a); cout << endl;
            }

            isl::union_map s = a;
            s.map_domain_through(schedule);
            s.for_each([&](const isl::map & m)
            {
                m.for_each([&](const isl::basic_map & access_schedule)
                {
                    if (verbose<scheduler>::enabled())
                    {
                        cout << "... Access schedule: "; m_printer.print(access_schedule); cout << endl;
                    }

                    access_info info;
                    info.array = array.get();
                    info.schedule = access_schedule;

                    // Find smallest unique ray of access schedule

                    vector<arrp::ivector> rays;
                    arrp::find_rays(access_schedule.wrapped().lifted().flattened(), rays);

                    if (verbose<scheduler>::enabled())
                    {
                        for (auto & r : rays)
                        {
                            cout << "ray: ";
                            for (auto & i : r)
                                cout << i << " ";
                            cout << endl;
                        }
                    }

                    if (rays.empty())
                    {
                        if (verbose<scheduler>::enabled())
                            cout << "No ray. Skipping." << endl;

                        access_infos.push_back(info);
                        return true;
                    }

                    if (rays.size() != 1)
                    {
                        throw error("Multiple rays.");
                    }


                    auto & ray = rays.front();

                    int n_time_dim = access_schedule.get_space().dimension(isl::space::input);
                    int n_array_dim = access_schedule.get_space().dimension(isl::space::output);

                    info.time_period = vector<int>(ray.begin(),
                                                   ray.begin() + n_time_dim);
                    info.data_offset = vector<int>(ray.begin() + n_time_dim,
                                                   ray.begin() + n_time_dim + n_array_dim);

                    access_infos.push_back(info);

                    return true;
                });

                return true;
            });
        }
    }

    return access_infos;
}

int scheduler::find_period_onset(const access_info & info, int tiling_dim)
{
    if (info.time_period.empty())
    {
        auto times = info.schedule.domain();
        auto time_space = times.get_space();
        // NOTE: Assuming infinite direction of schedule is positive in all dimensions
        int last_time = times.maximum(time_space.var(tiling_dim)).integer();

        if (verbose<scheduler>::enabled())
        {
            cout << "Finite Domain: "; m_printer.print(times); cout << endl;
            cout << "Latest: " << last_time << endl;
        }

        return last_time + 1;
    }
    else
    {
        auto times = info.schedule.domain();
        auto time_space = times.get_space();
        auto offset_func = isl::multi_expression::identity(time_space);
        int n_dim = time_space.dimension(isl::space::variable);
        for (int i = 0; i < n_dim; ++i)
        {
            offset_func.set(i, time_space.var(i) + info.time_period[i]);
        }
#if 0
        if (verbose<scheduler>::enabled())
        {
            cout << "Domain offset function: "; m_printer.print(offset_func); cout << endl;
        }
#endif
        isl::basic_set times_offset =
                isl_basic_set_preimage_multi_aff(times.copy(), offset_func.copy());

        if (verbose<scheduler>::enabled())
        {
            cout << "Infinite Domain: "; m_printer.print(times); cout << endl;
            cout << "Offset domain: "; m_printer.print(times_offset); cout << endl;
        }

        auto aliens = isl::set(times_offset) - isl::set(times);

        if (verbose<scheduler>::enabled())
        {
            cout << "Outliers: "; m_printer.print(aliens); cout << endl;
        }

        int time_of_latest_alien = aliens.maximum(time_space.var(tiling_dim)).integer();

        if (verbose<scheduler>::enabled())
        {
            cout << "Latest outlier: " << time_of_latest_alien << endl;
        }

        return time_of_latest_alien + 1;
    }
}

bool scheduler::validate_schedule(isl::union_map & schedule)
{
    auto deps = m_model_summary.dependencies;
    deps.map_domain_through(schedule);
    deps.map_range_through(schedule);

    isl::space sched_space(nullptr);
    schedule.for_each([&](const isl::map & m){
        sched_space = m.get_space().range();
        return false;
    });

    auto after = order_greater_than(sched_space);

    auto invalid_deps = deps & after;

    bool is_valid = invalid_deps.is_empty();

    if (!is_valid && verbose<scheduler>::enabled())
    {
        cout << "Invalid deps:" << endl;
        m_printer.print_each_in(invalid_deps);
    }

    return is_valid;
}

}
}
