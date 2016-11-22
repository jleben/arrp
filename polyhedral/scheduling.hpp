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

#ifndef STREAM_POLYHEDRAL_SCHEDULER_INCLUDED
#define STREAM_POLYHEDRAL_SCHEDULER_INCLUDED

#include "../common/ph_model.hpp"
#include "../utility/debug.hpp"

#include <isl/set.h>
#include <isl/union_set.h>
#include <isl/map.h>
#include <isl/union_map.h>
#include <isl/ast_build.h>
#include <isl/printer.h>

// Including these crashes pluto_schedule because
// pluto is linked both to libpiplibMP (needed by ISL)
// and libpiplib64, and it should use 64 code version,
// but including these ISL headers makes it call into MP version instead.

#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/context.hpp>
#include <isl-cpp/printer.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <stdexcept>

struct clast_stmt;

namespace isl {
class space;
class basic_set;
class set;
class union_set;
class basic_map;
class map;
class union_map;
}

namespace stream {
namespace polyhedral {

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;

class scheduler
{
public:

    class error : public std::runtime_error
    {
    public:
        error(const string & msg):
            runtime_error(msg)
        {}
    };

    struct reversal
    {
        string stmt_name;
        int dim;
    };

    scheduler( model & m );

    void set_schedule_whole_program(bool flag)
    {
        m_schedule_whole = flag;
    }

    polyhedral::schedule schedule
    (bool optimize, const vector<reversal> & reversals);

private:

    struct data
    {
        data(isl::context & ctx):
            finite_domains(ctx),
            infinite_domains(ctx),
            write_relations(ctx),
            read_relations(ctx),
            dependencies(ctx),
            schedule(ctx),
            prelude_schedule(ctx),
            period_schedule(ctx)
        {}
        isl::union_set finite_domains;
        isl::union_set infinite_domains;
        isl::union_map write_relations;
        isl::union_map read_relations;
        isl::union_map dependencies;
        isl::union_map schedule;

        isl::union_map prelude_schedule;
        isl::union_map period_schedule;
    };

    void prepare_data(data &);

    isl_bool add_schedule_constraints(struct isl_scheduler *sched);

    static
    isl_bool add_schedule_constraints_helper(struct isl_scheduler * sched, void * data)
    {
        auto me = reinterpret_cast<scheduler*>(data);
        return me->add_schedule_constraints(sched);
    }

    // Scheduling

    isl::schedule make_schedule(const isl::union_set & domains,
                                 const isl::union_map & dependencies,
                                 bool optimize);

    isl::union_map make_proximity_dependencies(const isl::union_map & dependencies);

    void make_periodic_schedule(polyhedral::schedule &);

    struct tiling
    {
        int dim;
        int offset;
        int size;
    };

    tiling find_periodic_tiling(const isl::union_map & schedule);

    struct access_info
    {
        polyhedral::array * array { nullptr };
        isl::basic_map schedule { nullptr };
        vector<int> time_period;
        vector<int> data_offset;
    };

    vector<access_info> analyze_access_schedules(const isl::union_map & schedule);

    int find_period_onset(const access_info & info, int dim);

    void find_stream_dim_and_period(const isl::union_map & schedule,
                                    int & dim, int & period);
    int find_prelude_duration(const isl::union_map & schedule,
                              int time_dim);
    void find_array_periods(const isl::union_map & schedule,
                            int time_dim, int prelude, int period);

    isl::union_map prelude_schedule
    (const isl::union_map & schedule, int prelude);
    isl::union_map periodic_schedule
    (const isl::union_map & schedule, int prelude, int period);

    bool validate_schedule(isl::union_map & schedule);

    int common_offset(isl::union_map & schedule, int flow_dim);

    void print_each_in( const isl::union_set & );
    void print_each_in( const isl::union_map & );

    isl::printer m_printer;

    model & m_model;
    model_summary m_model_summary;

    bool m_schedule_whole = false;
};

}
}


#endif // STREAM_POLYHEDRAL_SCHEDULER_INCLUDED
