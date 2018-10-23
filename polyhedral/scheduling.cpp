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
{
  m_printer.set_yaml_style(isl::printer::yaml_block_style);
}

polyhedral::schedule
scheduler::schedule(const scheduler::options & options)
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
        cout << "Order:" << endl;
        m_printer.print_each_in(m_model_summary.order_relations);
    }

    if (verbose<polyhedral::model>::enabled())
    {
        cout << endl << "### Domains: ###" << endl;
        m_printer.print(m_model_summary.domains);
        cout << endl;

        cout << endl << "### Dependencies:" << endl;
        m_printer.print(m_model_summary.dependencies);
        cout << endl;

        cout << endl << "### Order:" << endl;
        m_printer.print(m_model_summary.order_relations);
        cout << endl;
    }

    polyhedral::schedule schedule(m_model.context);

    schedule.tree = make_schedule(m_model_summary.domains,
                                  m_model_summary.dependencies,
                                  m_model_summary.order_relations,
                                  options);

    schedule.full = schedule.tree.map().in_domain(m_model_summary.domains);

    if (verbose<scheduler>::enabled())
    {
        cout << endl << "Schedule:" << endl;
        m_printer.print(schedule.tree);

        cout << endl << "Schedule map:" << endl;
        m_printer.print_each_in(schedule.full);
        cout << endl;

        cout << endl << "Scheduled dependencies:" << endl;
        m_model_summary.dependencies.for_each([&](isl::map m){
            auto space = m.get_space();
            cout << space.id(isl::space::input).name << " -> "
                 << space.id(isl::space::output).name << ": " << endl;
            isl::union_map um(m);
            um.map_domain_through(schedule.full);
            um.map_range_through(schedule.full);
            isl::union_set us = isl_union_map_deltas(um.copy());
            us.for_each([&](const isl::set & s){
                s.for_each([&](isl::basic_set bs){
                    m_printer.print(bs);
                    cout << endl;
                    return true;
                });
                return true;
            });
            return true;
        });
        cout << endl;
    }

    if (verbose<polyhedral::model>::enabled())
    {
        cout << endl << "### Schedule: ###" << endl;
        m_printer.print(schedule.full);
        cout << endl;
    }

    make_periodic_schedule(schedule, options);

    if (verbose<scheduler>::enabled())
    {
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
 const isl::union_map & order,
 const scheduler::options & options)
{
    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    isl_options_set_schedule_whole_component(domains.ctx().get(), !options.cluster);

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());
#if 0
    constr = isl_schedule_constraints_set_constraint_filter
            (constr, &add_schedule_constraints_helper, this);
#endif
    auto validity = dependencies | order;
    constr = isl_schedule_constraints_set_validity(constr, validity.copy());
    constr = isl_schedule_constraints_set_coincidence(constr, validity.copy());

    if (options.optimize)
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
scheduler::make_periodic_schedule(polyhedral::schedule & sched, const options & opt)
{
    isl_schedule_node * domain_node = isl_schedule_get_root(sched.tree.get());
    assert_or_throw(isl_schedule_node_get_type(domain_node) == isl_schedule_node_domain);

    isl_schedule_node * root = isl_schedule_node_get_child(domain_node, 0);
    assert_or_throw(root != nullptr);

    isl_schedule_node * infinite_node = nullptr;
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
            infinite_node = isl_schedule_node_copy(root);
    }
    else
    {
        assert_or_throw(root_type == isl_schedule_node_sequence);
        int elem_count = root_seq_elems = isl_schedule_node_n_children(root);
        for (int i = 0; i < elem_count; ++i)
        {
            if (infinite_node)
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
                        infinite_node = isl_schedule_node_copy(child);
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

    if (infinite_node)
    {
        if (verbose<scheduler>::enabled())
        {
            isl::union_map infinite_sched =
                    isl_schedule_node_get_subtree_schedule_union_map(infinite_node);
            cout << "Infinite part of schedule:" << endl;
            m_printer.print_each_in(infinite_sched);
            cout << endl;
        }

        auto original_schedule_func = isl_schedule_node_band_get_partial_schedule(infinite_node);

        // Tile

        if (!opt.tile_size.empty())
        {
            // NOTE: Tile entire schedule, because different schedules for prologue
            // and period can mess up with storage allocation
            // (storage required in prologue is redundant in period).

            infinite_node = tile(infinite_node, opt);

            if (verbose<scheduler>::enabled())
            {
                cout << "Tiled schedule:" << endl;
                isl_printer_print_schedule_node(m_printer.get(), infinite_node);
                cout << endl;
            }

            // Ensure tile parallelism

            if (opt.tile_parallelism)
            {
                infinite_node = ensure_tile_parallelism(infinite_node, opt);
            }
        }

        // Permute schedule dimensions (intra-tile schedule if tiling)

        if (!opt.intra_tile_permutation.empty())
        {
            bool is_tiling = !opt.tile_size.empty();

            auto node = infinite_node;
            if (is_tiling)
                node = isl_schedule_node_child(node, 0);

            node = permute_dimensions(node, opt.intra_tile_permutation);

            if (is_tiling)
                node = isl_schedule_node_parent(node);

            infinite_node = node;

            if (verbose<scheduler>::enabled())
            {
                cout << "Permuted schedule:" << endl;
                isl_printer_print_schedule_node(m_printer.get(), infinite_node);
                cout << endl;
            }
        }

        // Add periodic tiling dimension

        {
            infinite_node = add_periodic_tiling_dimension
                    (infinite_node, original_schedule_func, opt);

            if (verbose<scheduler>::enabled())
            {
                cout << "Schedule with period dimension:" << endl;
                isl_printer_print_schedule_node(m_printer.get(), infinite_node);
                cout << endl;
            }
        }

        original_schedule_func = isl_multi_union_pw_aff_free(original_schedule_func);

        // Find period size and offset

        vector<access_info> access_analysis =
                analyze_access_schedules
                (isl_schedule_node_get_subtree_schedule_union_map(infinite_node));

        auto periodic_tiling = find_periodic_tiling(access_analysis, opt);

        assign_inter_tile_access_offsets(periodic_tiling, access_analysis);

        // Extract prologue and periodic domains

        isl::union_set prologue_dom(m_model.context);
        isl::union_set periodic_dom(m_model.context);

        {
          isl::union_set infinite_sched_dom = isl_schedule_node_get_domain(infinite_node);
          isl::union_map infinite_sched =
                  isl::union_map(isl_schedule_node_get_subtree_schedule_union_map(infinite_node))
                  .in_domain(infinite_sched_dom);

          infinite_sched.for_each([&](isl::map & m)
          {
            auto space = m.get_space();
            auto mp = m;
            mp.add_constraint(space.out(0) < periodic_tiling.offset);
            prologue_dom |= mp.domain();
            //cout << "Adding prologue domain: ";
            //m_printer.print(mp.domain()); cout << endl;
            return true;
          });

          periodic_dom = infinite_sched_dom - prologue_dom;
        }

        // Apply period size and offset

        {
            isl::space band_space = isl_schedule_node_band_get_space(infinite_node);

            auto offset_val = isl::value(m_model.context, -periodic_tiling.offset);
            auto scale_val = isl::value(m_model.context, periodic_tiling.size);

            auto offset_upa =
                    isl_union_pw_aff_val_on_domain
                    (m_model_summary.domains.universe().copy(), offset_val.copy());

            infinite_node = isl_schedule_node_band_shift
                    (infinite_node,
                     isl_multi_union_pw_aff_from_union_pw_aff(offset_upa));

            auto scale_multi_val = isl_multi_val_zero(band_space.copy());
            scale_multi_val = isl_multi_val_set_val(scale_multi_val, 0, scale_val.copy());
            infinite_node = isl_schedule_node_band_scale_down(infinite_node, scale_multi_val);
        }

        // Store entire schedule
        sched.tree = isl_schedule_node_get_schedule(infinite_node);

        // Extract prologue
        {
          auto entire_prologue_domain = m_model_summary.domains - periodic_dom;
          sched.prelude_tree = sched.tree;
          sched.prelude_tree.intersect_domain(entire_prologue_domain);
        }

        // Extract single period
        {
          isl::union_map um = isl_schedule_node_get_subtree_schedule_union_map(infinite_node);
          isl::union_set dom = isl_schedule_node_get_domain(infinite_node);
          um = um.in_domain(dom);

          isl::union_set period_dom(m_model.context);
          um.for_each([&](isl::map & m)
          {
            auto space = m.get_space();
            auto mp = m;
            mp.add_constraint(space.out(0) == 0);
            period_dom |= mp.domain();
            //cout << "Adding prologue domain: ";
            //m_printer.print(mp.domain()); cout << endl;
            return true;
          });

          sched.period_tree = sched.tree;
          sched.period_tree.intersect_domain(period_dom);
        }

        // Make map representations
        sched.full = sched.tiled = sched.tree.map_on_domain();
        sched.prelude = sched.prelude_tree.map_on_domain();
        sched.period = sched.period_tree.map_on_domain();

        if (verbose<scheduler>::enabled())
        {
            cout << endl << "Tiled schedule:" << endl;
            m_printer.print(sched.tree);
            cout << endl;
            m_printer.print_each_in(sched.tiled);

            cout << endl << "Prologue schedule:" << endl;
            m_printer.print(sched.prelude_tree);
            cout << endl;
            m_printer.print_each_in(sched.prelude);

            cout << endl << "Period schedule:" << endl;
            m_printer.print(sched.period_tree);
            cout << endl;
            m_printer.print_each_in(sched.period);
        }
    }
    else
    {
        sched.prelude_tree = sched.tree;
        sched.prelude = sched.tiled = sched.full;
    }

    infinite_node = isl_schedule_node_free(infinite_node);
    root = isl_schedule_node_free(root);
    domain_node = isl_schedule_node_free(domain_node);
}

// Modifies and returns 'node' to represent the schedule of tiles,
// while inserting a new child node representing the intra-tile schedule.
isl_schedule_node * scheduler::tile(isl_schedule_node * node, const options & opt)
{
    if (opt.tile_size.empty())
        return node;

    isl::space sched_space = isl_schedule_node_band_get_space(node);

    auto tile_size = isl_multi_val_zero(sched_space.copy());

    for (int i = 0; i < sched_space.dimension(isl::space::variable); ++i)
    {
        int size = i < opt.tile_size.size() ? opt.tile_size[i] : 1;
        if (size < 1)
            throw error("Invalid tile size: " + to_string(size));
        isl::value val(m_model.context, size);
        tile_size = isl_multi_val_set_val(tile_size, i, val.copy());
    }

    node = isl_schedule_node_band_tile(node, tile_size);

    return node;
}

// Replace original tile band [t0, t1, t2, ... tn]
// with [(t0 + t1 + ... + tn), t1, t2, ... tn]

// Assuming "node" is the tiling node, otherwise this transformation is illegal.

isl_schedule_node * scheduler::ensure_tile_parallelism(isl_schedule_node * node, const options & opt)
{
    if (verbose<scheduler>::enabled())
    {
        cout << "Ensuring tile parallelism..." << endl;
    }

    isl::space band_space = isl_schedule_node_band_get_space(node);
    int band_size = band_space.dimension(isl::space::variable);

    if (band_size < 2)
    {
        throw error("Can not ensure tile parallelism: the schedule band has fewer than 2 dimensions.");
    }

    // Create abstract function t0' = (t0 + t1 + ... + tn) as a function of tile indices

    isl::local_space local_band_space(band_space);

    auto tile0_func = isl::expression::value(local_band_space, 0);

    for (int i = 0; i < band_size; ++i)
    {
        int tile_size = i < opt.tile_size.size() ? opt.tile_size[i] : 1;
        tile0_func = tile0_func + local_band_space(isl::space::variable, i) / tile_size;
    }

    // Compose t0' with original tile functions, to get a function in statement indices

    auto band_func = isl_schedule_node_band_get_partial_schedule(node);

    auto tile0_proper_func = isl_multi_union_pw_aff_apply_aff(
                isl_multi_union_pw_aff_copy(band_func),
                tile0_func.copy());

    // Replace t0 with t0'

    band_func = isl_multi_union_pw_aff_set_union_pw_aff(band_func, 0, tile0_proper_func);

    // Insert replacement band

    node = isl_schedule_node_insert_partial_schedule(node, band_func);

    // Get original band node and delete it
    node = isl_schedule_node_child(node, 0);
    node = isl_schedule_node_delete(node);

    // Get the replacement band again

    node = isl_schedule_node_parent(node);

    return node;
}

// Permute dimensions of the given band node.
// Only permutations up to band size are used.
// If size of permutation is smaller than band size,
// the remaining band dimension are unchanged.

// Precisely: Given a permutation (i0, i1, ... i_n) and band (0, 1, ... m),
// the result is (k0, k1, ... k_m) where
// k_x = i_x if x < n and x < m,
// k_x = x otherwise.
isl_schedule_node * scheduler::permute_dimensions(isl_schedule_node * node, const vector<int> & permutation)
{
    assert_or_throw(isl_schedule_node_get_type(node) == isl_schedule_node_band);

    isl::space band_space = isl_schedule_node_band_get_space(node);
    int band_size = band_space.dimension(isl::space::variable);
    auto band_func = isl_schedule_node_band_get_partial_schedule(node);

    int permuted_dim_count = std::min(band_size, int(permutation.size()));

    // Extract dimensions used in permutation from the band
    vector<isl_union_pw_aff*> dim_funcs(permuted_dim_count);
    for (int i = 0; i < permuted_dim_count; ++i)
    {
        int j = permutation[i];
        assert_or_throw(j < band_size);
        dim_funcs[i] = isl_multi_union_pw_aff_get_union_pw_aff(band_func, j);
    }

    // Store permuted dimensions back into the band
    for (int i = 0; i < permuted_dim_count; ++i)
    {
        band_func = isl_multi_union_pw_aff_set_union_pw_aff(band_func, i, dim_funcs[i]);
    }

    // Insert permuted band
    node = isl_schedule_node_insert_partial_schedule(node, band_func);

    // Delete original band that has become a child of the permuted band
    node = isl_schedule_node_child(node, 0);
    node = isl_schedule_node_delete(node);

    // 'node' is now a child of the deleted node,
    // so move one level up to the permuted band.
    node = isl_schedule_node_parent(node);

    return node;
}

isl_schedule_node * scheduler::add_periodic_tiling_dimension
(isl_schedule_node * node,
 isl_multi_union_pw_aff * original_schedule_func,
 const options & opt)
{
    if (verbose<scheduler>::enabled())
    {
        cout << "Adding periodic tiling dimesion..." << endl;
    }

    isl::space band_space = isl_schedule_node_band_get_space(node);
    int band_size = band_space.dimension(isl::space::variable);

    // Find the infinite direction of the range of schedule.
    // Since the schedule is a union map, we find the ray of the
    // convex hull of the range of schedule, with equivalent result.

    isl::union_map schedule_map = isl_schedule_node_get_subtree_schedule_union_map(node);
    schedule_map = schedule_map
            .in_domain(m_model_summary.domains);

    isl::space range_space(nullptr);
    schedule_map.for_each([&](const isl::map & m){
        range_space = m.get_space().range();
        return false;
    });

    auto sched_range = schedule_map.range().set_for(range_space);

    auto simplified_range = sched_range.lifted().flattened();

    // NOTE: Finding convex hull takes too long in some cases, so we find simple hull:
    isl::basic_set schedule_range_hull = simplified_range.simple_hull();

    if (verbose<scheduler>::enabled())
    {
        cout << "Schedule hull:" << endl;
        m_printer.print(schedule_range_hull);
        cout << endl;
    }

    arrp::ivector ray = arrp::find_single_ray(schedule_range_hull);
    if (ray.empty())
        throw error("Schedule has multiple or no infinite directions.");

    if (verbose<scheduler>::enabled())
    {
        cout << "Infinite direction:";
        for (auto & v : ray)
            cout << ' ' << v;
        cout << endl;
    }

    // Get periodic tiling direction selected by user.

    arrp::ivector period_dir = opt.periodic_tile_direction;

    if (period_dir.empty())
    {
        // User has not requested any particular direction.
        // Find first scheduling direction with a non-zero projection
        // on the ray.
        // If the schedule was tiled, this only searches the inter-tile
        // dimensions.

        int dim = 0;
        for (; dim < band_size; ++dim)
        {
            if (ray[dim] != 0)
                break;
        }

        auto tile_func = isl_schedule_node_band_get_partial_schedule(node);

        auto tile0_func = isl_multi_union_pw_aff_get_union_pw_aff(tile_func, dim);

        node = isl_schedule_node_insert_partial_schedule
                (node,
                 isl_multi_union_pw_aff_from_union_pw_aff(tile0_func));

        tile_func = isl_multi_union_pw_aff_free(tile_func);
    }
    else
    {
        // User has requested a particular direction.
        // The direction is interpreted in the space of
        // the inter-tile schedule (or intra-tile schedule of not tiled),
        // but before the transformation by 'ensure_tile_parallelism'.

        // Validate selected direction.

        if (period_dir.size() > band_size)
            throw error("Selected periodic tiling direction has too many dimensions.");

        for (int i = 0 ; i < band_size && i < period_dir.size(); ++i)
            if (period_dir[i] < 0)
                throw error("Periodic tiling direction must be non-negative.");
#if 0
        // Map tile to schedule space

        int common_tile_size_multiple = 1;
        for (auto size : opt.tile_size)
            common_tile_size_multiple = lcm(common_tile_size_multiple, size);

        for (int i = 0; i < period_dir.size(); ++i)
        {
            auto & coef = period_dir[i];
            coef = common_tile_size_multiple * coef;
            if (i < opt.tile_size.size())
                coef /= opt.tile_size[i];
        }

        if (verbose<scheduler>::enabled())
        {
            cout << "Mapped period direction: ";
            for (auto coef : period_dir)
                cout << " " << coef;
            cout << endl;
        }

        int dot_product = 0;
        for (int i = 0 ; i < band_size && i < period_dir.size(); ++i)
        {
            auto coef = period_dir[i];
            if (coef < 0)
                throw error("Periodic tiling direction must be non-negative.");

            dot_product += ray[i] * coef;
        }

        if (dot_product == 0)
            throw error("Selected periodic tiling direction is perpendicular to infinite direction of schedule.");
#endif

        // Create abstract expression
        // p = k0 * t0 * floor(i0/t0) + ... + k_n * t_n * floor(i_n/t_n) where
        // <k0, k1, ...> is the user-specified period direction,
        // <t0, t1, ...> is the tile size, and
        // <i0, i1, ...> is an index in the original schedule space.

        // Note that t * floor(i/t) recreates an inter-tile index
        // before ensure_tile_parallelism was applied.

        // Note: if not tiling, the tile size will be 1, with no effect.

        isl::local_space local_band_space(band_space);

        auto period_func = isl::expression::value(local_band_space, 0);

        for (int i = 0; i < period_dir.size(); ++i)
        {
            auto coef = period_dir[i];
            int tile_size = i < opt.tile_size.size() ? opt.tile_size[i] : 1;
            auto tile_index = tile_size * isl::floor(local_band_space(isl::space::variable, i) / tile_size);
            period_func = period_func + coef * tile_index;
        }

        // Compose the abstract function with the original schedule functions

        auto full_period_func = isl_multi_union_pw_aff_apply_aff(
                    isl_multi_union_pw_aff_copy(original_schedule_func),
                    period_func.copy());

        // Insert a node with the given schedule function

        node = isl_schedule_node_insert_partial_schedule
                (node,
                 isl_multi_union_pw_aff_from_union_pw_aff(full_period_func));
    }

    return node;
}

scheduler::tiling
scheduler::find_periodic_tiling(const vector<access_info> & access_infos, const options & opt)
{
    if (verbose<scheduler>::enabled())
        cout << endl << "Finding periodic tiling." << endl;

    const int period_index_dimension = 0;

    tiling periodic_tiling;
    periodic_tiling.offset = 0;
    periodic_tiling.size = 1;

    // Find common tiling period
    for (auto & access : access_infos)
    {
        if (access.time_period.empty())
            continue;

        int access_period = access.time_period[period_index_dimension];
        periodic_tiling.size = lcm(periodic_tiling.size, access_period);
    }

    // Find common period onset
    for (auto & access : access_infos)
    {
        int onset = find_period_onset(access, period_index_dimension);
        periodic_tiling.offset = max(periodic_tiling.offset, onset);
    }

    // Apply tiling options

    if (opt.period_offset < 0)
      throw error("Invalid period offset.");
    if (opt.period_scale < 1)
      throw error("Invalid period scaling.");

    periodic_tiling.offset += opt.period_offset;
    periodic_tiling.size *= opt.period_scale;

    if (verbose<scheduler>::enabled())
    {
        cout << "Periodic tiling:" << endl;
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
        for (auto & access : stmt->array_accesses)
        {
            auto array = access->array;
            auto a = access->map.in_domain(stmt->domain).in_range(array->domain);

            if (verbose<scheduler>::enabled())
            {
                cout << ".. Access";
                if (access->reading) cout << ", read";
                if (access->writing) cout << ", write";
                cout << ": ";
                m_printer.print(a);
                cout << endl;
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
                        cout << "... Access schedule flat: ";
                        m_printer.print(access_schedule.wrapped().lifted().flattened());
                        cout << endl;
                    }

                    access_info info;
                    info.array = array.get();
                    info.schedule = access_schedule;

                    // Find smallest unique ray of access schedule

                    bool has_rays = false;
                    auto ray = arrp::find_single_ray(access_schedule.wrapped().lifted().flattened(), &has_rays);
                    if (ray.empty())
                    {
                        if (has_rays)
                            throw error("Multiple infinite directions.");

                        if (verbose<scheduler>::enabled())
                            cout << "No infinite direction. Skipping." << endl;

                        access_infos.push_back(info);
                        return true;
                    }

                    if (verbose<scheduler>::enabled())
                    {
                        cout << "ray: ";
                        for (auto & i : ray)
                            cout << i << " ";
                        cout << endl;
                    }

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

        if (verbose<scheduler>::enabled())
        {
            cout << "Finite Domain: "; m_printer.print(times); cout << endl;
        }

        auto time_space = times.get_space();
        // NOTE: Assuming infinite direction of schedule is positive in all dimensions
        int last_time = times.maximum(time_space.var(tiling_dim)).integer();

        if (verbose<scheduler>::enabled())
        {
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

void scheduler::assign_inter_tile_access_offsets
(const tiling & periodic_tiling, const vector<access_info> & analysis)
{
    const int period_index_dimension = 0;

    for (auto & access : analysis)
    {
        if (access.time_period.empty())
            continue;

        // Make sure:
        // if there is an infinite direction in data space
        // it is parallel to first dimension.
        for (int dim = 1; dim < access.data_offset.size(); ++dim)
        {
            if (access.data_offset[dim] != 0)
                throw error("Access has unexpected infinite direction in data space.");
        }

        int periods_per_tile = periodic_tiling.size / access.time_period[period_index_dimension];
        int offset = access.data_offset[0] * periods_per_tile;

        auto array = access.array;
        if (array->period == 0)
            array->period = offset;
        else if (array->period != offset)
            throw error("Accesses have inconsistent tile offsets in array space.");
    }
}

bool scheduler::validate_schedule(isl::union_map & schedule)
{
    auto deps = m_model_summary.dependencies | m_model_summary.order_relations;
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
