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
        if (verbose<storage_allocator>::enabled())
        {
            cout << endl << "== Array " << array->name << endl;
        }

        compute_buffer_size(schedule, array, *time_space);

        find_inter_period_dependency(schedule, array);
    }

    delete time_space;
}

void storage_allocator::compute_buffer_size
( const polyhedral::schedule & schedule,
  const array_ptr & array,
  const isl::space & time_space )
{
    if (verbose<storage_allocator>::enabled())
    {
        cout << ".. Buffer size:" << endl;
    }

    using namespace isl;
    using isl::expression;

    auto array_space = array->domain.get_space();

    auto array_sched_space = isl::space::from(array_space, time_space);

    // Filter and map writers and readers

    auto write_relations = m_model_summary.write_relations.in_domain(m_model_summary.domains);
    auto read_relations = m_model_summary.read_relations.in_domain(m_model_summary.domains);

    auto all_write_sched = schedule.full;
    all_write_sched.map_domain_through(write_relations);
    auto write_sched = all_write_sched.map_for(array_sched_space);

    auto all_read_sched = schedule.full;
    all_read_sched.map_domain_through(read_relations);
    auto read_sched = all_read_sched.map_for(array_sched_space);

    if (verbose<storage_allocator>::enabled())
    {
        cout << "  Write schedule: ";
        m_printer.print(write_sched);
        cout << endl;
        cout << "  Read schedule: ";
        m_printer.print(read_sched);
        cout << endl;
    }

    if (read_sched.is_empty())
    {
        // No readers - no storage needed.
        return;
    }

    if (write_sched.is_empty())
    {
        throw std::runtime_error("Storage alloc: Array has readers but no writers.");
    }

    /*
    A pair of array elements is live at the same time if
    mutually one is written before the other one is read.

    ISCC:
    w_before_r := unwrap(domain((aw cross ar) ->* wrap (t <<= t)));
    live_together := w_before_r * (w_before_r^-1);
    */

    auto before = order_less_than(time_space);
    auto written_before_read =
            write_sched.cross(read_sched).in_range(before.wrapped()).domain().unwrapped();

    auto live_together =
            written_before_read & (written_before_read.inverse());
#if 0
    if (verbose<storage_allocator>::enabled())
    {
        cout << ".. Written before read:" << endl;
        m_printer.print_each_in(written_before_read);

        cout << ".. Live together:" << endl;
        m_printer.print_each_in(live_together);
    }
#endif
    vector<int> buffer_size;

    {
        int buf_dim_count = array_space.dimension(isl::space::variable);
        buffer_size.reserve(buf_dim_count);

        if (verbose<storage_allocator>::enabled())
            cout << "  Max reuse distance:" << endl;

        for (int dim = 0; dim < buf_dim_count; ++dim)
        {
            {
                auto live_together_set = live_together.wrapped();
                isl::local_space space(live_together_set.get_space());
                auto a = space(isl::space::variable, dim);
                auto b = space(isl::space::variable, buf_dim_count + dim);
                auto max_distance = live_together_set.maximum(b - a);
                if (!max_distance.is_integer())
                {
                    ostringstream msg;
                    msg << "Infinite buffer size required for array "
                        << array->name << ", dimension " << dim << ".";
                    throw std::runtime_error(msg.str());
                }
                if (verbose<storage_allocator>::enabled())
                    cout << max_distance.integer() << " ";
                buffer_size.push_back((int) max_distance.integer() + 1);
            }
            {
                isl::local_space space(live_together.get_space());
                auto a = space(isl::space::input, dim);
                auto b = space(isl::space::output, dim);
                live_together.add_constraint(a == b);
            }
        }

        if (verbose<storage_allocator>::enabled())
            cout << endl;
    }

    array->buffer_size = buffer_size;
}

void storage_allocator::find_inter_period_dependency
( const polyhedral::schedule & schedule,
  const array_ptr & array )
{

    auto written_in_period =
            m_model_summary.write_relations( schedule.period.domain() )
            .set_for(array->domain.get_space());

    auto read_in_period =
            m_model_summary.read_relations( schedule.period.domain() )
            .set_for(array->domain.get_space());

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
