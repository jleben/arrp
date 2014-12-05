#ifndef STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
#define STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED

#include "model.hpp"
#include "dataflow_model.hpp"
#include "../utility/isl_printer.hpp"
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

//#include <isl-cpp/set.hpp>
//#include <isl-cpp/map.hpp>
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

    ast_generator( const vector<statement*> &, const dataflow::model * );
    ~ast_generator();

    pair<struct clast_stmt*,struct clast_stmt*> generate();

private:

    // Translation to ISL representation:

    void polyhedral_model(isl::union_set & domains,
                          isl::union_map & dependencies);

    isl::basic_set polyhedral_domain( statement * );
    isl::union_map polyhedral_dependencies( statement * );
    isl::matrix constraint_matrix( const mapping & );

    void periodic_model( const isl::union_set & domains,
                         const isl::union_map & dependencies,
                         isl::union_map & domain_map,
                         isl::union_set & init_domains,
                         isl::union_set & steady_domains,
                         isl::union_map & periodic_dependencies );

    // Scheduling

    isl::union_map make_schedule(isl::union_set & domains,
                                 isl::union_map & dependencies);

    isl::union_map make_init_schedule(isl::union_set & domains,
                                      isl::union_map & dependencies);

    isl::union_map make_steady_schedule(isl::union_set & domains,
                                        isl::union_map & dependencies);

    isl::union_map combine_schedule(const isl::union_set & init_domains,
                                    const isl::union_set & steady_domains,
                                    const isl::union_map & init_schedule,
                                    const isl::union_map & period_schedule);

    // Buffer size computation

    void compute_buffer_sizes( const isl::union_map & schedule,
                               const isl::union_map & dependencies,
                               const isl::union_map & domain_maps );

    void compute_buffer_size( const isl::union_map & schedule,
                              const isl::union_map & domain_unmap,
                              const isl::map & dependence,
                              const isl::space & time_space );

    // AST generation

    struct clast_stmt *make_ast( const isl::union_map & schedule );

    isl::context m_ctx;
    isl::printer m_printer;

    vector<statement*> m_statements;
    const dataflow::model * m_dataflow;
};

}
}


#endif // STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
