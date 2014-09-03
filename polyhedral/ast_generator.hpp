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

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <stdexcept>

namespace stream {
namespace polyhedral {

using std::string;
using std::vector;
using std::unordered_map;
using std::pair;

class ast_generator
{
public:
    class error : public std::runtime_error
    {
    public:
        error(const string & msg):
            runtime_error(msg)
        {}
    };

    ast_generator();
    ~ast_generator();
    isl_ast_node *generate( const vector<statement*> & statements );

private:
    typedef std::unordered_map<string, statement*> statement_store;
    typedef statement_store::value_type statement_info;

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

    pair<isl_union_set*,isl_union_map*> isl_representation();

    isl_union_map *schedule(const pair<isl_union_set*,isl_union_map*> &);

    // returns two domains:
    // 1. initialization
    // 2. repetition
    pair<isl_union_set*, isl_union_set*>
    dataflow_iteration_domains(isl_union_set* domains);

    // Helpers for translation to ISL representation:
    isl_basic_set *isl_iteration_domain( const statement_info & );
    isl_union_map *isl_dependencies( const statement_info & );
    isl_mat *isl_constraint_matrix( const mapping & );

    // General helpers:
    void dependencies( expression *, vector<stream_access*> & );
    void dataflow_dependencies( const statement_info &,
                                     vector<dataflow_dependency> & );
    void dataflow_iteration_counts( const vector<dataflow_dependency> &,
                                    vector<pair<string, int>> & );
    isl_union_set *repetition_domains(isl_union_set *domains,
                                      const vector<pair<string, int>> & counts);
    vector<int> infinite_dimensions( statement *stmt );

    statement_store m_statements;
    isl_ctx *m_ctx;
    utility::isl::printer m_printer;
};

}
}


#endif // STREAM_POLYHEDRAL_AST_GENERATOR_INCLUDED
