#include "ast_generator.hpp"

#include <osl/osl.h>
#include <pluto/libpluto.h>

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace polyhedral {

ast_generator::ast_generator():
    m_ctx(isl_ctx_alloc()),
    m_printer(m_ctx)
{
}

ast_generator::~ast_generator()
{
    // TODO: free all other data...

    // hmm, defer deleting context until printer is deleted?
    isl_ctx_free(m_ctx);
}

isl_ast_node *ast_generator::generate( const vector<statement*> & statements )
{
    store_statements(statements);

    pair<isl_union_set*, isl_union_map*> isl_repr = isl_representation();

    isl_union_map *sched = schedule(isl_repr);

    cout << "Schedule:" << endl;
    m_printer.print(sched);
    cout << endl;

    return nullptr;
}

void ast_generator::store_statements( const vector<statement*> & statements )
{
    int idx = 0;
    for (auto stmt : statements)
    {
        ostringstream name;
        name << "S_" << idx;
        m_statements[name.str()] = stmt;
        ++idx;
    }
}

pair<isl_union_set*,isl_union_map*>
ast_generator::isl_representation()
{
    isl_union_set *all_domains = nullptr;
    for (const auto & entry : m_statements)
    {
        isl_basic_set *domain = isl_iteration_domain(entry);
        isl_union_set *to_unite = isl_union_set_from_basic_set(domain);
        if (all_domains)
            all_domains = isl_union_set_union(all_domains, to_unite);
        else
            all_domains = to_unite;
    }

    isl_union_map *all_dep = nullptr;
    for (const auto & entry : m_statements)
    {
        if (!entry.second->expr)
            continue;

        isl_union_map *stmt_dep = isl_dependencies(entry);
        if (all_dep)
            all_dep = isl_union_map_union(all_dep, stmt_dep);
        else
            all_dep = stmt_dep;
    }

    return std::make_pair(all_domains, all_dep);
}

isl_basic_set *ast_generator::isl_iteration_domain( const statement_info & stmt_info )
{
    const string & name = stmt_info.first;
    statement *stmt = stmt_info.second;

    isl_space *space = isl_space_set_alloc(m_ctx, 0, stmt->domain.size());
    space = isl_space_set_tuple_name(space, isl_dim_set, name.c_str());

    isl_basic_set *domain = isl_basic_set_universe(isl_space_copy(space));

    isl_local_space *constraint_space = isl_local_space_from_space(space);

    for (int dim = 0; dim < stmt->domain.size(); ++dim)
    {
        int extent = stmt->domain[dim];

        isl_constraint *lower_bound;
        lower_bound = isl_inequality_alloc(isl_local_space_copy(constraint_space));
        lower_bound = isl_constraint_set_coefficient_si(lower_bound, isl_dim_set, dim, 1);
        domain = isl_basic_set_add_constraint(domain, lower_bound);

        if (extent >= 0)
        {
            isl_constraint *upper_bound;
            upper_bound = isl_inequality_alloc(isl_local_space_copy(constraint_space));
            upper_bound = isl_constraint_set_coefficient_si(upper_bound, isl_dim_set, dim, -1);
            upper_bound = isl_constraint_set_constant_si(upper_bound, (extent-1));
            domain = isl_basic_set_add_constraint(domain, upper_bound);
        }
    }

    isl_local_space_free(constraint_space);

    cout << endl << "Iteration domain:" << endl;
    m_printer.print(domain);
    cout << endl;

    return domain;
}

isl_union_map *ast_generator::isl_dependencies( const statement_info & stmt_info )
{
    const string & source_name = stmt_info.first;
    statement *source = stmt_info.second;

    vector<stream_access*> deps;

    add_dependencies(source->expr, deps);

    isl_union_map *united_map = nullptr;

    for (auto dep : deps)
    {
        auto dep_info_criteria = [&]( const statement_info & info )
        {
            return info.second == dep->target;
        };

        const auto & dep_info =
                * std::find_if(m_statements.begin(), m_statements.end(),
                               dep_info_criteria);

        // NOTE: "input" and "output" are swapped in the ISL model.
        // "input" = dependee
        // "output" = depender

        isl_space *space = isl_space_alloc(m_ctx, 0,
                                           dep->target->domain.size(),
                                           source->domain.size());

        space = isl_space_set_tuple_name(space, isl_dim_in, dep_info.first.c_str());
        space = isl_space_set_tuple_name(space, isl_dim_out, source_name.c_str());

        isl_mat *dep_matrix = isl_constraint_matrix(dep->pattern);
        isl_mat *dummy_matrix = isl_mat_alloc(m_ctx, 0, isl_mat_cols(dep_matrix));
        isl_basic_map *map =
                isl_basic_map_from_constraint_matrices(space,
                                                       dep_matrix,
                                                       dummy_matrix,
                                                       isl_dim_in,
                                                       isl_dim_out,
                                                       isl_dim_cst,
                                                       isl_dim_param,
                                                       isl_dim_div);

        cout << endl << "Dependency:" << endl;
        m_printer.print(map);
        cout << endl;

        isl_union_map *map_to_unite = isl_union_map_from_basic_map(map);
        if (united_map)
            united_map = isl_union_map_union(united_map, map_to_unite);
        else
            united_map = map_to_unite;
    }

    return united_map;
}

isl_mat *ast_generator::isl_constraint_matrix( const mapping & map )
{
    // one constraint for each output dimension
    int rows = map.output_dimension();
    // output dim + input dim + a constant
    int cols = map.output_dimension() + map.input_dimension() + 1;

    isl_mat *matrix = isl_mat_alloc(m_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            matrix = isl_mat_set_element_si(matrix, r, c, 0);

    for (int out_dim = 0; out_dim < map.output_dimension(); ++out_dim)
    {
        // Put output index on the other side of the equality (negate):
        matrix = isl_mat_set_element_si(matrix, out_dim, out_dim, -1);

        for (int in_dim = 0; in_dim < map.input_dimension(); ++in_dim)
        {
            int col = in_dim + map.output_dimension();
            int coef = map.coefficients(out_dim, in_dim);
            matrix = isl_mat_set_element_si(matrix, out_dim, col, coef);
        }

        int offset = map.constants[out_dim];
        matrix = isl_mat_set_element_si(matrix, out_dim, cols-1, offset);
    }

    return matrix;
}

isl_union_map *ast_generator::schedule
(const pair<isl_union_set*,isl_union_map*> & representation)
{

    PlutoOptions *options = pluto_options_alloc();
    //options->debug = 1;
    //options->islsolve = 1;
    //options->fuse = MAXIMAL_FUSE;
    //options->unroll = 1;
    //options->polyunroll = 1;
    //options->ufactor = 2;
    //options->tile = 1;
    //options->parallel = 1;

    isl_union_set *stmts = representation.first;
    isl_union_map *dependencies = representation.second;

    isl_union_map *schedule = pluto_schedule(stmts, dependencies, options);

    pluto_options_free(options);

    return schedule;
}

void ast_generator::add_dependencies( expression *expr,
                                      vector<stream_access*> & deps )
{
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto sub_expr : operation->operands)
            add_dependencies(sub_expr, deps);
        return;
    }
    if (auto dependency = dynamic_cast<stream_access*>(expr))
    {
        deps.push_back(dependency);
        return;
    }
    if ( dynamic_cast<constant<int>*>(expr) ||
         dynamic_cast<constant<double>*>(expr) )
    {
        return;
    }
    throw std::runtime_error("Unexpected expression type.");
}

}
}
