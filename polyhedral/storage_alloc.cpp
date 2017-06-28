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

storage_allocator::storage_allocator( model & m):
    m_model(m),
    m_model_summary(m),
    m_printer(m.context)
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

    auto conflicts = compute_conflicts(write_sched, read_sched, order_less_than_or_equal(sched_space));

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
                conflicts |= conflict.inverse();
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
        if (verbose<storage_allocator>::enabled())
            cout << "  Computing max reuse distance..." << endl;

        if (verbose<storage_allocator>::enabled())
            cout << "  Max reuse distance:";

        isl::map active_conflicts = conflicts;

        for (int dim = 0; dim < buffer_size.size(); ++dim)
        {
            {
                auto conflict_set = active_conflicts.wrapped();
                isl::local_space space(conflict_set.get_space());
                auto a = space(isl::space::variable, dim);
                auto b = space(isl::space::variable, buffer_size.size() + dim);
                auto max_distance = conflict_set.maximum(b - a);
                if (!max_distance.is_integer())
                {
                    ostringstream msg;
                    msg << "Infinite buffer size required for array "
                        //<< array->name
                        << conflicts.name(isl::space::input)
                        << ", dimension " << dim << ".";
                    throw std::runtime_error(msg.str());
                }
                if (verbose<storage_allocator>::enabled())
                    cout << max_distance.integer() << " ";
                buffer_size[dim] = (int) max_distance.integer() + 1;
            }
            {
                isl::local_space space(active_conflicts.get_space());
                auto a = space(isl::space::input, dim);
                auto b = space(isl::space::output, dim);
                active_conflicts.add_constraint(a == b);
            }
        }

        if (verbose<storage_allocator>::enabled())
            cout << endl;
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
}

}
}
