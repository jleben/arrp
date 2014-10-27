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

#include <cloog/cloog.h>

//Including these crashes pluto_schedule. Why??
//#include <isl-cpp/set.hpp>
//#include <isl-cpp/map.hpp>
#include <isl-cpp/context.hpp>
#include <isl-cpp/printer.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <stdexcept>

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

    clast_stmt *
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

    // Buffer size computation

    void compute_buffer_sizes( const isl::union_map & schedule,
                               const isl::union_map & dependencies );

    void compute_buffer_size( const isl::union_map & schedule,
                              const isl::map & dependence,
                              const isl::space & time_space );

    friend int compute_buffer_size_helper(isl_map *dependence, void *d);

    // Dataflow

    // returns two domains:
    // 1. initialization
    // 2. repetition
    pair<isl_union_set*, isl_union_set*>
    dataflow_iteration_domains(isl_union_set* domains);

    void dataflow_dependencies( const statement_info &,
                                vector<dataflow_dependency> & );

    void dataflow_iteration_counts( const vector<dataflow_dependency> &,
                                    vector<pair<string, int>> & );

    isl_union_set *repetition_domains(isl_union_set *domains,
                                      const vector<pair<string, int>> & counts);

    // General helpers:

    void dependencies( expression *, vector<stream_access*> & );
    vector<int> infinite_dimensions( statement *stmt );


    statement_store m_statements;
    isl::context m_ctx;
    isl::printer m_printer;
};

}
}


#endif // STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
