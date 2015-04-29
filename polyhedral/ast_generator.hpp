/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
#define STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED

#include "../common/polyhedral_model.hpp"
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

class ast_generator
{
public:

    struct debug : public stream::debug::topic<debug, polyhedral::debug>
    { static string id() { return "ast"; } };

    struct debug_buffer_size :
            public stream::debug::topic
            <debug_buffer_size, debug, stream::debug::disabled>
    { static string id() { return "buffer-size"; } };

    class error : public std::runtime_error
    {
    public:
        error(const string & msg):
            runtime_error(msg)
        {}
    };

    ast_generator( const vector<statement*> &, const vector<array_ptr> & arrays );
    ~ast_generator();

    pair<struct clast_stmt*,struct clast_stmt*> generate();

    void set_print_ast_enabled(bool flag)
    {
        m_print_ast = flag;
    }

private:

    struct data
    {
        data(isl::context & ctx):
            finite_domains(ctx),
            infinite_domains(ctx),
            write_relations(ctx),
            read_relations(ctx),
            dependencies(ctx)
        {}
        isl::union_set finite_domains;
        isl::union_set infinite_domains;
        isl::union_map write_relations;
        isl::union_map read_relations;
        isl::union_map dependencies;
    };

    // Translation to ISL representation:

    void polyhedral_model(data &);
    void polyhedral_model(statement *, data &);
#if 0
    void polyhedral_model(isl::union_set & finite_domains,
                          isl::union_set & infinite_domains,
                          isl::union_map & data_iter_map,
                          isl::union_map & dependencies);

    pair<isl::basic_set, isl::basic_map> polyhedral_domain( statement * );
    isl::union_map polyhedral_dependencies( statement * );
    #endif

    isl::matrix constraint_matrix( const mapping & );
#if 0
    void periodic_model( const isl::union_set & domains,
                         const isl::union_map & dependencies,
                         isl::union_map & domain_map,
                         isl::union_set & init_domains,
                         isl::union_set & steady_domains,
                         isl::union_map & periodic_dependencies );
#endif
    // Scheduling

    isl::union_map make_schedule(const isl::union_set & domains,
                                 const isl::union_map & dependencies);

    isl::union_map make_init_schedule(isl::union_set & domains,
                                      isl::union_map & dependencies);

    isl::union_map make_steady_schedule(isl::union_set & domains,
                                        isl::union_map & dependencies);

    isl::union_map schedule_finite_domains(const isl::union_set & finite_domains,
                                           const isl::union_map & dependencies);

    pair<isl::union_map, isl::union_map>
    schedule_infinite_domains(const isl::union_set & infinite_domains,
                              const isl::union_map & dependencies,
                              isl::union_map & infinite_schedule);

    void combine_schedules(const isl::union_map & finite_schedule,
                           const isl::union_map & infinite_schedule,
                           isl::union_map & combined_schedule);

    int compute_period(const isl::union_map & schedule,
                       int & flow_dim, int & n_dims_out);

    int common_offset(isl::union_map & schedule, int flow_dim);

    void print_each_in( const isl::union_set & );
    void print_each_in( const isl::union_map & );

    // Buffer size computation

    void compute_buffer_sizes( const isl::union_map & schedule,
                               const data & );

    void compute_buffer_size
    ( const isl::union_map & schedule,
      const data &,
      const array_ptr & array,
      const isl::space & time_space );

    // AST generation

    struct clast_stmt *make_ast( const isl::union_map & schedule );

    bool m_print_ast;

    isl::context m_ctx;
    isl::printer m_printer;

    vector<statement*> m_statements;
    vector<array_ptr> m_arrays;
};

}
}


#endif // STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
