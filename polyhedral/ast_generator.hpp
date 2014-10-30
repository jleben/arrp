#ifndef STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
#define STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED

#include "model.hpp"
#include "../utility/isl_printer.hpp"

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
    typedef std::unordered_map<string, statement_data> statement_store;
    typedef statement_store::value_type statement_info;

    class error : public std::runtime_error
    {
    public:
        error(const string & msg):
            runtime_error(msg)
        {}
    };

    ast_generator();
    ~ast_generator();

    struct clast_stmt *
    generate( const vector<statement*> & statements );

    const statement_store &statements()
    {
        return m_statements;
    }

private:

    struct dataflow_dependency
    {
        string source;
        string sink;
        int source_dim;
        int sink_dim;
        int push;
        int peek;
        int pop;
    };

    struct dataflow_count
    {
        int init;
        int steady;
    };

    void store_statements( const vector<statement*> & );

    // Translation to ISL representation:

    void make_isl_representation(isl::union_set & domains,
                                 isl::union_map & dependencies);

    isl::basic_set isl_iteration_domain( const statement_info & );
    isl::union_map isl_dependencies( const statement_info & );
    isl::matrix isl_constraint_matrix( const mapping & );

    // Scheduling

    isl::union_map make_schedule(isl::union_set & domains,
                                 isl::union_map & dependencies);

    isl::union_map entire_steady_schedule( const isl::union_map &
                                           period_schedule );

    // Buffer size computation

    void compute_buffer_sizes( const isl::union_map & schedule,
                               const isl::union_map & dependencies );

    void compute_buffer_size( const isl::union_map & schedule,
                              const isl::map & dependence,
                              const isl::space & time_space );

    friend int compute_buffer_size_helper(isl_map *dependence, void *d);

    // Dataflow

    void compute_dataflow_dependencies( vector<dataflow_dependency> & result );

    void compute_dataflow_dependencies( const statement_info &info,
                                        vector<dataflow_dependency> & result );

    void compute_dataflow_counts ( const vector<dataflow_dependency> &,
                                   unordered_map<string,dataflow_count> & result );

    void dataflow_model( const isl::union_set & domains,
                         const isl::union_map & dependencies,
                         const unordered_map<string,dataflow_count> & counts,
                         isl::union_set & result_domains,
                         isl::union_map & result_dependencies);

    isl::union_set steady_period( const isl::union_set & domains,
                                  const unordered_map<string,dataflow_count> & counts );
#if 0
    // returns two domains:
    // 1. initialization
    // 2. repetition
    pair<isl_union_set*, isl_union_set*>
    dataflow_iteration_domains(isl_union_set* domains);

    isl_union_set *repetition_domains(isl_union_set *domains,
                                      const vector<pair<string, int>> & counts);
#endif


    // General helpers:

    void dependencies( expression *, vector<stream_access*> & );
    int first_infinite_dimension( statement *stmt );
    vector<int> infinite_dimensions( statement *stmt );


    statement_store m_statements;
    isl::context m_ctx;
    isl::printer m_printer;
    unordered_map<string,dataflow_count> m_dataflow_counts;
};

}
}


#endif // STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
