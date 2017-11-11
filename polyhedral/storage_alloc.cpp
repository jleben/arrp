/*
Compiler for language for stream processing

Copyright (C) 2016  Jakob Leben <jakob.leben@gmail.com>

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

#include "storage_alloc.hpp"

#include <isl-cpp/space.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/utility.hpp>

#include <stdexcept>
#include <sstream>

using namespace std;

namespace stream {
namespace polyhedral {

storage_allocator::storage_allocator( model & m, bool classic):
    m_model(m),
    m_model_summary(m),
    m_printer(m.context),
    m_classic(classic)
{

}

void storage_allocator::allocate(const polyhedral::schedule & schedule)
{
    using namespace isl;


    for (auto & array : m_model.arrays)
    {
        if (verbose<storage_allocator>::enabled())
        {
            cout << endl << "== Array " << array->name << endl;
        }

        compute_buffer_size(schedule, array);

        find_inter_period_dependency(schedule, array);
    }
}

void storage_allocator::compute_buffer_size
( const polyhedral::schedule & schedule,
  const array_ptr & array )
{
    if (verbose<storage_allocator>::enabled())
    {
        cout << ".. Buffer size:" << endl;
    }

    using namespace isl;
    using isl::expression;

    isl::space sched_space(nullptr);
    schedule.tiled.for_each([&](const isl::map & m){
        sched_space = m.get_space().range();
        return false;
    });

    auto array_space = array->domain.get_space();

    auto array_sched_space = isl::space::from(array_space, sched_space);

    // Filter and map writers and readers

    auto write_relations = m_model_summary.write_relations.in_domain(m_model_summary.domains);
    auto read_relations = m_model_summary.read_relations.in_domain(m_model_summary.domains);

    auto all_write_sched = schedule.tiled;
    all_write_sched.map_domain_through(write_relations);
    auto write_sched = all_write_sched.map_for(array_sched_space).in_domain(array->domain);

    auto all_read_sched = schedule.tiled;
    all_read_sched.map_domain_through(read_relations);
    auto read_sched = all_read_sched.map_for(array_sched_space).in_domain(array->domain);

    if (verbose<storage_allocator>::enabled())
    {
        cout << "  Write schedule: " << endl;
        m_printer.print_each_in(write_sched);
        cout << endl;
        cout << "  Read schedule: " << endl;
        m_printer.print_each_in(read_sched);
        cout << endl;
    }

#if 0 // FIXME: The rest of the code assumes the array is always stored
    if (read_sched.is_empty())
    {
        // No readers - no storage needed.
        return;
    }
#endif

    if (write_sched.is_empty())
    {
        ostringstream msg;
        msg << "Storage alloc: Array " << array->name << " has readers but no writers." << endl;
        throw error(msg.str());
    }

    /*
    A pair of array elements is live at the same time if
    there exists an overlapping pair of periods between
    a write and a read of each of the elements -
    i.e the write of one is before the read of the other,
    as well as the opposite.

    We assume:
    There are no writes unrelated to any reads.
    If a write writes multiple elements, it also reads them.
    */

    auto access_sched = write_sched | read_sched;

    if (verbose<storage_allocator>::enabled())
    {
        cout << "  Access schedule: " << endl;
        m_printer.print_each_in(access_sched);
        cout << endl;
    }

    //cout << "Num access schedule sets: " << isl_map_n_basic_map(access_sched.get()) << endl;

    int buffer_dim_count = array_space.dimension(isl::space::variable);
    array->buffer_size = vector<int>(buffer_dim_count, 1);

    // Read-write conflicts
    auto conflicts = compute_conflicts(write_sched, read_sched, order_less_than(sched_space));

    // Read-read and write-write conflicts
    auto equal_time = isl::basic_map::identity(sched_space, sched_space);
    conflicts |= write_sched.cross(write_sched).in_range(equal_time.wrapped()).domain().unwrapped();
    conflicts |= read_sched.cross(read_sched).in_range(equal_time.wrapped()).domain().unwrapped();

    // Parallel conflicts
    auto parallel_conflicts = m_model.parallel_accesses.map_for(conflicts.get_space());

    if (verbose<storage_allocator>::enabled())
    {
        cout << "Num parallel conflicts: " << isl_map_n_basic_map(parallel_conflicts.get()) << endl;
    }

    conflicts |= parallel_conflicts;

    compute_buffer_size_from_conflicts(conflicts, array->buffer_size);
}

/*
Given a schedule of writes and reads, and an order relation R,
compute all pairs of array elements (x,y) such that
wx R ry and wy R rx where wx, wy, rx, ry are some write and read
times.
*/
isl::map storage_allocator::compute_conflicts
(const isl::map & write_schedule,
 const isl::map & read_schedule,
 const isl::map & order_relation)
{
    auto array_space = write_schedule.get_space().domain();

    isl::map precedence(isl::space::from(array_space, array_space));
    isl::map conflicts(precedence.get_space());

    auto access_schedule_pairs = write_schedule.cross(read_schedule);

    if (verbose<storage_allocator>::enabled())
    {
        cout << endl
             << "*** Num access schedule pairs: "
             << isl_map_n_basic_map(access_schedule_pairs.get())
             << endl;
    }

    order_relation.for_each([&](const isl::basic_map & order)
    {
        auto wrapped_order = order.wrapped();

        access_schedule_pairs.for_each([&](const isl::basic_map & a)
        {
            auto this_precedence = a.in_range(wrapped_order).domain().unwrapped();

            if (!this_precedence.is_empty())
                precedence |= this_precedence;

            return true;
        });

        return true;
    });

    if (verbose<storage_allocator>::enabled())
        cout << "Num precedence sets: " << isl_map_n_basic_map(precedence.get()) << endl;


    precedence = isl_map_coalesce(precedence.copy());
    //precedence = isl_map_remove_redundancies(precedence.copy());

    if (verbose<storage_allocator>::enabled())
        cout << "Num coalesced precedence sets: " << isl_map_n_basic_map(precedence.get()) << endl;

    int i = 0;
    precedence.for_each([&](const isl::basic_map & a){
        ++i;
        int j = 0;
        precedence.for_each([&](const isl::basic_map & b){
            ++j;
            if (j > i)
                return false;

            auto conflict = a & b.inverse();

            if (!conflict.is_empty())
            {
                // NOTE:
                // We need the inverse conflict too,
                // to ensure distance maximization works, since
                // some conflicts might have negative distance.
                conflicts |= conflict;
                //conflicts |= conflict.inverse();
            }

            return true;
        });
        return true;
    });

    if (verbose<storage_allocator>::enabled())
        cout << "Num conflicts: " << isl_map_n_basic_map(conflicts.get()) << endl;

    //conflicts = isl_map_coalesce(conflicts.copy());
    //cout << "Basic maps in conflicts optimized: " << isl_map_n_basic_map(conflicts.get()) << endl;

    return conflicts;
}

static
isl::set absolute_values(const isl::set & set)
{
    auto abs_set = isl::set(set.get_space());

    // Keep items that are already positive

    {
        auto positive = set;

        for(int dim = 0; dim < set.dimensions(); ++dim)
        {
            positive.limit_below(isl::space::variable, dim, 0);
        }

        abs_set |= positive;
    }

    // Map non-positive items into positive items

    auto abs_map = isl::map(isl::space::from(set.get_space(), set.get_space()));

    for(int dim = 0; dim < set.dimensions(); ++dim)
    {
        auto m = isl::basic_map::universe(abs_map.get_space());
        m.limit_above(isl::space::input, dim, 0);

        for(int dim2 = 0; dim2 < set.dimensions(); ++dim2)
        {
            if (dim2 == dim)
                m.add_constraint(m.get_space().out(dim2) == -m.get_space().in(dim2));
            else
                m.add_constraint(m.get_space().out(dim2) == m.get_space().in(dim2));
        }

        abs_map |= m;
    }

    abs_set |= abs_map(set);

    return abs_set;
}

void storage_allocator::compute_buffer_size_from_conflicts
( const isl::map & conflicts, vector<int> & buffer_size )
{
    if (conflicts.is_empty())
    {
        if (verbose<storage_allocator>::enabled())
            cout << "  No elements live together" << endl;

        for (auto & s : buffer_size)
            s = 1;
    }
    else
    {
        // Initialize buffer size, just in case.

        for (auto & s : buffer_size)
            s = 1;

        // Compute conflict distances

        auto deltas = conflicts.deltas();

        // Compute absolute values of distances

        deltas = absolute_values(deltas);

        if (!m_classic)
        {

        // For each dimension, find out the conflicts that can only be satisfied
        // by this dimension (their distance is zero in all other dimensions).
        // The maximum distance of such conflicts is the minimum buffer size in this dimension.

        for (int dim = 0; dim < deltas.dimensions(); ++dim)
        {
            auto exclusive_deltas = deltas;

            for (int dim2 = 0; dim2 < deltas.dimensions(); ++dim2)
            {
                if (dim2 != dim)
                    exclusive_deltas.add_constraint(exclusive_deltas.get_space().var(dim2) == 0);
            }

            auto max = exclusive_deltas.maximum(deltas.get_space().var(dim));
            if (!max.is_integer())
                throw error("Infinite storage conflict distance.");

            buffer_size[dim] = max.integer() + 1;
        }

        // Exclude conflicts already satisfied by the buffer size computed above.
        // A conflict is unsatisfied if its distance in each dimension is either 0
        // or larger than the buffer size.
        for (int dim = 0; dim < deltas.dimensions(); ++dim)
        {
            auto a = isl::set::universe(deltas.get_space());
            a.limit_below(isl::space::variable, dim, buffer_size[dim]);

            auto b = isl::set::universe(deltas.get_space());
            b.add_constraint(b.get_space().var(dim) == 0);

            deltas &= (a | b);
        }

        }

        // Expand buffer to satisfy remaining conflicts using the
        // classic successive modulo technique.

        for (int dim = 0; dim < buffer_size.size(); ++dim)
        {
            if (deltas.is_empty())
                break;

            {
                auto max_distance = deltas.maximum(deltas.get_space().var(dim));
                if (!max_distance.is_integer())
                    throw error("Infinite storage conflict distance.");

                int max_distance_int = (int) max_distance.integer() + 1;

                buffer_size[dim] = std::max(max_distance_int, buffer_size[dim]);
            }
            {
                deltas.add_constraint(deltas.get_space().var(dim) == 0);
            }
        }
    }
}

void storage_allocator::find_inter_period_dependency
( const polyhedral::schedule & schedule,
  const array_ptr & array )
{
    auto written_in_period =
            m_model_summary.write_relations( schedule.period.domain() )
            .set_for(array->domain.get_space())
            & array->domain;

    auto read_in_period =
            m_model_summary.read_relations( schedule.period.domain() )
            .set_for(array->domain.get_space())
            & array->domain;

    auto remaining = read_in_period - written_in_period;

    array->inter_period_dependency = !remaining.is_empty();

    if (verbose<storage_allocator>::enabled())
    {
        cout << ".. Inter-period dependency:" << endl;
        cout << "  written: "; m_printer.print(written_in_period); cout << endl;
        cout << "  read: "; m_printer.print(read_in_period); cout << endl;
        cout << "  remaining: "; m_printer.print(remaining); cout << endl;
        cout << "  inter-period dep = "
             << (array->inter_period_dependency ? "true" : "false")
             << endl;
    }

    if (array->is_infinite)
    {
        auto accessed_in_period = written_in_period | read_in_period;

        auto i0 = array->domain.get_space().var(0);
        auto min_i0 = accessed_in_period.minimum(i0);
        auto max_i0 = accessed_in_period.maximum(i0);
        assert_or_throw(min_i0.is_integer());
        assert_or_throw(max_i0.is_integer());

        array->first_period_access = min_i0.integer();
        array->last_period_access = max_i0.integer();

        if (verbose<storage_allocator>::enabled())
        {
            cout << ".. Period access range: ("
                 << array->first_period_access
                 << ", "
                 << array->last_period_access
                 << ")" << endl;
        }
    }
}

}
}
