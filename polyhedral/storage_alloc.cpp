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

    isl::space *time_space = nullptr;

    schedule.full.for_each([&time_space]( const map & m )
    {
        time_space = new space( m.range().get_space() );
        return false;
    });

    for (auto & array : m_model.arrays)
    {
        compute_buffer_size(schedule, array, *time_space);
    }

    delete time_space;
}

void storage_allocator::compute_buffer_size
( const polyhedral::schedule & schedule,
  const array_ptr & array,
  const isl::space & time_space )
{
    if (debug::is_enabled())
    {
        cout << "Computing buffer size for array: " << array->name << endl;
    }

    using namespace isl;
    using isl::expression;

    auto array_space = array->domain.get_space();

    auto array_sched_space = isl::space::from(array_space, time_space);

    // Filter and map writers and readers

    auto all_write_sched = schedule.full;
    all_write_sched.map_domain_through(m_model_summary.write_relations);
    auto write_sched = all_write_sched.map_for(array_sched_space);

    auto all_read_sched = schedule.full;
    all_read_sched.map_domain_through(m_model_summary.read_relations);
    auto read_sched = all_read_sched.map_for(array_sched_space);

    if (read_sched.is_empty())
    {
        // No readers - no storage needed.
        return;
    }

    if (write_sched.is_empty())
    {
        throw std::runtime_error("Storage alloc: Array has readers but no writers.");
    }

    // Do the work

    map not_later = order_greater_than_or_equal(time_space);
    map later = order_less_than(time_space);

    // Find all source instances scheduled before t, for all t.
    // => Create map: t -> src
    //    such that: time(src) <= t

    auto written_not_later = write_sched.inverse()( not_later );

    // Find all instances of source consumed after time t, for each t;
    // => Create map: t -> src
    //    such that: time(sink(src)) <= t, for all sink

    auto read_later = read_sched.inverse()( later );

    // Find all src instances live at the same time.
    // = Create map: t -> src,
    //   Such that: time(src) <= t and time(sink(src)) <= t, for all sink
    auto buffered = written_not_later & read_later;

    if (debug::is_enabled())
    {
        cout << ".. Buffered: " << endl;
        m_printer.print(buffered);
        cout << endl;
    }

    vector<int> buffer_size;

    {
        auto buffered_reflection = (buffered * buffered);

        if (debug::is_enabled())
        {
            cout << ".. Buffer reflection: " << endl;
            m_printer.print(buffered_reflection); cout << endl;
        }

        isl::local_space space(buffered_reflection.get_space());
        int buf_dim_count = array_space.dimension(isl::space::variable);
        int time_dim_count = time_space.dimension(isl::space::variable);
        buffer_size.reserve(buf_dim_count);

        if (debug::is_enabled())
            cout << ".. Max reuse distance:" << endl;

        for (int dim = 0; dim < buf_dim_count; ++dim)
        {
            {
                auto buffered_set = buffered_reflection.wrapped();
                isl::local_space space(buffered_set.get_space());
                auto a = space(isl::space::variable, time_dim_count + dim);
                auto b = space(isl::space::variable, time_dim_count + buf_dim_count + dim);
                auto max_distance = buffered_reflection.wrapped().maximum(b - a);
                if (!max_distance.is_integer())
                {
                    ostringstream msg;
                    msg << "Infinite buffer size required for array "
                        << array->name << ", dimension " << dim << ".";
                    throw std::runtime_error(msg.str());
                }
                if (debug::is_enabled())
                    cout << max_distance.integer() << " ";
                buffer_size.push_back((int) max_distance.integer() + 1);
            }
            {
                isl::local_space space(buffered_reflection.get_space());
                auto a = space(isl::space::output, dim);
                auto b = space(isl::space::output, buf_dim_count + dim);
                buffered_reflection.add_constraint(a == b);
            }
        }

        if (debug::is_enabled())
            cout << endl;
    }

    array->buffer_size = buffer_size;
}

}
}
