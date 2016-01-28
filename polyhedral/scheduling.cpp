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

#include <cloog/cloog.h>
#include <cloog/isl/cloog.h>

#include <isl/schedule.h>
#include <isl/schedule_node.h>

#include <isl-cpp/space.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/matrix.hpp>
#include <isl-cpp/utility.hpp>
#include <isl-cpp/printer.hpp>

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

scheduler::scheduler( model & m ):
    m_printer(m.context),
    m_model(m),
    m_model_summary(m)
{}

polyhedral::schedule
scheduler::schedule()
{
    if (debug::is_enabled())
        cout << endl << "### Scheduling ###" << endl;

    polyhedral::schedule schedule(m_model.context);
    schedule.full = make_schedule(m_model_summary.domains,
                                  m_model_summary.dependencies);

    auto periodic_schedule = make_periodic_schedule(schedule.full);
    schedule.prelude = periodic_schedule.first;
    schedule.period = periodic_schedule.second;

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

    {
        isl_schedule_node * node = isl_scheduler_get_current_node(sched);
        while(node)
        {
            auto type = isl_schedule_node_get_type(node);
            if (type == isl_schedule_node_sequence)
            {
                cout << "The schedule has a sequence node before a band node." << endl;
                isl_schedule_node_free(node);
                return isl_bool_error;
            }
            if (!isl_schedule_node_has_parent(node))
                break;
            node = isl_schedule_node_parent(node);
        }
    }

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
            cout << "Warning: coefficients not found for statement "
                 << stmt->name << endl;
        }

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

isl::union_map scheduler::make_schedule
(const isl::union_set & domains, const isl::union_map & dependencies)
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
                if (debug::is_enabled()) {
                    cout << "Accepting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }

                proximity_deps = proximity_deps | dep;
            }
            else
            {
                if (debug::is_enabled()) {
                    cout << "Rejecting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }
            }

            return true;
        });
        return true;
    });

    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());

    constr = isl_schedule_constraints_set_constraint_filter
            (constr, &add_schedule_constraints_helper, this);

    constr = isl_schedule_constraints_set_validity(constr, dependencies.copy());
    constr = isl_schedule_constraints_set_proximity(constr, proximity_deps.copy());

    isl_schedule * sched =
            isl_schedule_constraints_compute_schedule(constr);
    assert(sched);

    isl::union_map sched_map( isl_schedule_get_map(sched) );
    sched_map = sched_map.in_domain(domains);

    if (debug::is_enabled())
    {
        cout << "Schedule:" << endl;
        isl_printer_print_schedule(m_printer.get(), sched);
        cout << endl;

        cout << endl << "Schedule map:" << endl;
        print_each_in(sched_map);
        cout << endl;
    }

    isl_schedule_free(sched);

    return sched_map;
}


pair<isl::union_map, isl::union_map>
scheduler::make_periodic_schedule(const isl::union_map & schedule)
{
    int period_dur = compute_period_duration(schedule);
    int prelude_dur = compute_prelude_duration(schedule);
    auto sched_prelude = prelude_schedule(schedule, prelude_dur);
    auto sched_period = periodic_schedule(schedule, prelude_dur, period_dur);
    return make_pair(sched_prelude, sched_period);
}

#if 1
int scheduler::compute_period_duration
(const isl::union_map & schedule)
{
    // NOTE: Assuming that all the dependencies
    // are uniform in the infinite dimension of the schedule.

    // This implies that, for each statement S,
    // each p(S) instances of its infinite scheduling hyperplane
    // corresponding to 1 step in its infinite domain dimension
    // does exactly the same work and constitutes a repeating period.
    // p(S) = scheduling coefficient for that schedule and that domain
    // dimension.

    // To get to the common period for all statements, we need to
    // get the least common multiple of all p(S).


    // For each statement, get the scheduling coefficient
    // for its infinite dimension.

    vector<pair<statement*,int>> ks;

    schedule.for_each( [&](const isl::map & stmt_sched)
    {
        auto id = stmt_sched.id(isl::space::input);
        auto stmt = statement_for(id);
        assert(stmt);

        if (!stmt->is_infinite)
            return true;

        // Get scheduling coefficient for the infinite dimension

        // NOTE: Assuming that the schedule does not differ across
        // the infinite domain (but may differ across others).

        stmt_sched.for_each( [&] (const isl::basic_map & stmt_sched_part)
        {
            // NOTE: Assuming that the schedule is a bijection,
            // the coefficients of interest must be only in equalities.

            // - Get the equalities matrix
            // - Among the rows, find the one that defines the first
            // dimension of the schedule.
            // - From this row, get the coefficient for the first dimension
            // of the statement.
            auto equalities = stmt_sched_part.equalities_matrix();
            auto space = stmt_sched_part.get_space();
            int n_rows = equalities.row_count();
            int n_in = space.dimension(isl::space::input);
            int stmt_coef_col = 0;
            int sched_coef_col = n_in;
            for (int row = 0; row < n_rows; ++row)
            {
                int sched_coef = equalities(row, sched_coef_col).value().integer();
                if (sched_coef)
                {
                    int stmt_coef = equalities(row, stmt_coef_col).value().integer();
                    ks.push_back(make_pair(stmt, std::abs(stmt_coef)));
                    // continue to next partial schedule for this stmt
                    return true;
                }
            }
            throw error("The first dimension of the schedule does not involve"
                        " the infinite statement dimension.");
        });

        return true;
    });

    // Find least common multiple of the coefficients.
    // This is the common period of statements.

    int least_common_period = 1;
    for (const auto & k : ks)
    {
        int period = k.second;
        least_common_period = lcm(least_common_period, period);
    }

    if (debug::is_enabled())
    {
        cout << "Period duration = " << least_common_period << endl;
    }

    // FIXME: Check correctness of the following:

    // Compute how much of a statement's infinite dimension is covered
    // by the period.
    // This equals the offset in buffer indexes added at each period.
    for (const auto & k : ks)
    {
        auto stmt = k.first;
        int period = k.second;
        int span = least_common_period / period;

        if (debug::is_enabled())
        {
            cout << "Period spans " << span
                 << " iterations of " << stmt->name << endl;
        }

        // FIXME: Use the statement's write relation,
        // do not assume it is identity.

        if (!stmt->array->period)
            stmt->array->period = span;
        if (stmt->array->period != span)
            cerr << "WARNING: different buffer periods for the same array!" << endl;
    }

    return least_common_period;
}
#endif

int scheduler::compute_prelude_duration(const isl::union_map & schedule)
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

    // Assumption: schedule is bounded to iteration domains.

    int offset = std::numeric_limits<int>::min();

    schedule.for_each( [&](map & stmt_sched)
    {
        auto id = stmt_sched.id(isl::space::input);
        auto stmt = statement_for(id);
        auto sched_range = stmt_sched.range();
        isl::local_space space(sched_range.get_space());
        auto time = space(isl::space::variable, 0);
        if (stmt->is_infinite)
        {
            // Get domain of schedule.
            // Translate it by 1.
            // Subtract it from original domain.
            // Get latest time of the remaining domain.

            auto domain = stmt_sched.domain();

            auto domain_map_space =
                    space::from(domain.get_space(), domain.get_space());
            isl_multi_aff * translation =
                    isl_multi_aff_identity(domain_map_space.copy());
            isl_aff * i = isl_multi_aff_get_aff(translation, 0);
            i = isl_aff_set_constant_si(i, -1);
            isl_multi_aff_set_aff(translation, 0, i);

            auto translated_domain =
                    isl::set(isl_set_preimage_multi_aff(domain.copy(), translation));

            auto remaining = domain - translated_domain;
            auto sched = stmt_sched(remaining);
            auto time = sched.get_space()(isl::space::variable, 0);
            auto latest_time = sched.maximum(time);

            assert(latest_time.is_integer());
            offset = std::max(offset, (int) latest_time.integer());
        }
        else
        {
            auto latest_time = sched_range.maximum(time);
            assert(latest_time.is_integer());
            offset = std::max(offset, (int) latest_time.integer() + 1);
        }
        return true;
    });

    if (debug::is_enabled())
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

    if (debug::is_enabled())
    {
        cout << "Prelude schedule:" << endl;
        print_each_in(sched_prelude);
        cout << endl;
    }

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
        if (!stmt->array->period_offset)
            stmt->array->period_offset = offset;
        else if (stmt->array->period_offset != offset)
            cerr << "WARNING: different offsets for same buffer." << endl;

        sched_period = sched_period | stmt_sched;

        return true;
    });

    if (debug::is_enabled())
    {
        cout << "Periodic schedule:" << endl;
        print_each_in(sched_period);
        cout << endl;
    }

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
