#include "ast_generator.hpp"

#include <osl/osl.h>
#include <pluto/libpluto.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <cstdlib>

using namespace std;

namespace stream {
namespace polyhedral {

ast_generator::ast_generator():
    m_ctx(isl_ctx_alloc()),
    m_printer(m_ctx)
{
    //isl_space *context_space = isl_space_params_alloc(m_ctx, 0);
    //isl_set *context_set = isl_set_empty(context_space);
    isl_set *context_set = isl_set_read_from_str(m_ctx, "{:}");
    m_ast_builder = isl_ast_build_from_context(context_set);
    assert(m_ast_builder);
}

ast_generator::~ast_generator()
{
    // TODO: free all other data...
    // hmm, defer deleting context until printer is deleted?

    isl_ast_build_free(m_ast_builder);
    isl_ctx_free(m_ctx);
}

pair<isl_ast_node*,isl_ast_node*>
ast_generator::generate( const vector<statement*> & statements )
{
    store_statements(statements);

    pair<isl_union_set*, isl_union_map*> isl_repr = isl_representation();

    isl_union_map *sched = schedule(isl_repr);

    cout << endl << "Schedule:" << endl;
    m_printer.print(sched);
    cout << endl;

    auto dataflow_domains = dataflow_iteration_domains(isl_repr.first);
    auto init_domain = dataflow_domains.first;
    auto rep_domain = dataflow_domains.second;

    auto rep_ast = generate_ast(sched, rep_domain);

    printf("# Repetition AST:\n");
    m_printer.print(rep_ast);
    printf("\n");

    isl_union_set_free(isl_repr.first);
    isl_union_map_free(isl_repr.second);
    isl_union_map_free(sched);

    return make_pair(nullptr, rep_ast);
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
    for (const auto & stmt_info : m_statements)
    {
        if (!stmt_info.second->expr)
            continue;

        isl_union_map *stmt_dep = isl_dependencies(stmt_info);
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
    // We assume that a statement only writes one scalar value at a time.
    // Therefore, a dependency between two statements is exactly
    // the polyhedral::stream_access::pattern in the model.

    const string & source_name = stmt_info.first;
    statement *source = stmt_info.second;

    vector<stream_access*> deps;

    dependencies(source->expr, deps);

    isl_union_map *united_map = nullptr;

    for (auto dep : deps)
    {
        auto stmt_is_dep_target = [&dep]( const statement_info & stmt_info )
        {
            return stmt_info.second == dep->target;
        };

        const auto & target_info =
                * std::find_if(m_statements.begin(), m_statements.end(),
                               stmt_is_dep_target);
        const string & target_name = target_info.first;

        // NOTE: "input" and "output" are swapped in the ISL model.
        // "input" = dependee
        // "output" = depender

        isl_space *space = isl_space_alloc(m_ctx, 0,
                                           dep->target->domain.size(),
                                           source->domain.size());

        space = isl_space_set_tuple_name(space, isl_dim_in, target_name.c_str());
        space = isl_space_set_tuple_name(space, isl_dim_out, source_name.c_str());

        isl_mat *eq_matrix = isl_constraint_matrix(dep->pattern);
        isl_mat *ineq_matrix_dummy = isl_mat_alloc(m_ctx, 0, isl_mat_cols(eq_matrix));
        isl_basic_map *map =
                isl_basic_map_from_constraint_matrices(space,
                                                       eq_matrix,
                                                       ineq_matrix_dummy,
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

pair<isl_union_set*, isl_union_set*>
ast_generator::dataflow_iteration_domains(isl_union_set* domains)
{
    vector<string> finite_statements;
    vector<string> infinite_statements;
    vector<string> invalid_statements;

    for(auto info : m_statements)
    {
        const string & stmt_name = info.first;
        statement *stmt = info.second;
        vector<int> infinite_dims = infinite_dimensions(stmt);
        if (infinite_dims.empty())
            finite_statements.push_back(stmt_name);
        else if (infinite_dims.size() == 1)
            infinite_statements.push_back(stmt_name);
        else
            invalid_statements.push_back(stmt_name);
    }

    cout << endl << "Statement types:" << endl;
    cout << "- finite: " << finite_statements.size() << endl;
    cout << "- infinite: " << infinite_statements.size() << endl;
    cout << "- invalid: " << invalid_statements.size() << endl;

    if (!invalid_statements.empty())
    {
        ostringstream msg;
        msg << "The following statements are infinite"
            << " in more than 1 dimension: " << endl;
        for (const auto & name: invalid_statements)
        {
            msg << "- " << name << endl;
        }
        throw error(msg.str());
    }

    vector<dataflow_dependency> deps;

    for(const string & name : infinite_statements)
    {
        dataflow_dependencies(*m_statements.find(name), deps);
    }

    vector<pair<string,int>> counts;
    dataflow_iteration_counts(deps, counts);

    isl_union_set *rep_domains = repetition_domains(domains, counts);

    // TODO: initialization domains
    // - each stmt must get its "peek" amount of elems available

    return make_pair((isl_union_set*) nullptr, rep_domains);
}

void ast_generator::dataflow_iteration_counts
( const vector<dataflow_dependency> &deps, vector<pair<string, int>> & counts )
{
    // FIXME: Multiple dependencies between same pair of statements

    unordered_set<string> involved_stmts;

    for (const auto & dep: deps)
    {
        involved_stmts.insert(dep.source);
        involved_stmts.insert(dep.sink);
    }

    int rows = deps.size();
    int cols = involved_stmts.size() + 1; // 1 dummy for constants
    isl_mat * flow_matrix =
            isl_mat_alloc(m_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for(int c = 0; c < cols; ++c)
            isl_mat_set_element_si(flow_matrix, r, c, 0);

    int row = 0;
    for (const auto & dep: deps)
    {
        auto source_loc = involved_stmts.find(dep.source);
        int source_index = std::distance(involved_stmts.begin(), source_loc);
        auto sink_loc = involved_stmts.find(dep.sink);
        int sink_index = std::distance(involved_stmts.begin(), sink_loc);

        cout << "source: " << dep.source << "@" << source_index << endl;
        cout << "sink: " << dep.sink << "@" << sink_index << endl;
        isl_mat_set_element_si(flow_matrix, row, source_index, dep.push);
        isl_mat_set_element_si(flow_matrix, row, sink_index, dep.pop);
        ++row;
    }

    isl_mat *dummy_inequalities_matrix = isl_mat_alloc(m_ctx, 0, cols);
    isl_space *space = isl_space_set_alloc(m_ctx, 0, involved_stmts.size());
    isl_basic_set *flow_nullspace =
            isl_basic_set_from_constraint_matrices
            (space, flow_matrix, dummy_inequalities_matrix,
             isl_dim_set, isl_dim_cst, isl_dim_param, isl_dim_div);

    assert(flow_nullspace);
    cout << "Nullspace:" << endl;
    m_printer.print(flow_nullspace); cout << endl;

    isl_basic_set * flow_nullspace_coeff =
            isl_basic_set_coefficients(flow_nullspace);

    assert(flow_nullspace_coeff);
    cout << "Nullspace coefficients:" << endl;
    m_printer.print(flow_nullspace_coeff); cout << endl;

    isl_mat *flow_nullspace_coeff_matrix =
            isl_basic_set_equalities_matrix
            (flow_nullspace_coeff,
             isl_dim_set,
             isl_dim_cst,
             isl_dim_param,
             isl_dim_div
             );

#if 0
    cout << endl << "Nullspace coefficient matrix:" << endl;
    {
        isl_mat * m = flow_nullspace_coeff_matrix;
        int r, c;
        for (r = 0; r < isl_mat_rows(m); ++r)
        {
            for (c = 0; c < isl_mat_cols(m); ++c)
            {
                double val = isl_val_get_d( isl_mat_get_element_val(m, r, c) );
                cout << val << " ";
            }
            cout << endl;
        }
    }
#endif
    if ( isl_mat_rows(flow_nullspace_coeff_matrix) != 1 ||
         isl_mat_cols(flow_nullspace_coeff_matrix) != involved_stmts.size() + 2)
    {
        throw error("Inconsistent dataflow.");
    }

    cout << endl << "Iteration counts:" << endl;
    auto stmt_loc = involved_stmts.begin();
    for (int i = 0; i < involved_stmts.size(); ++i)
    {
        //int count = isl_mat_get_element_si(flow_nullspace_coeff_matrix, 0, i+1);
        isl_val *count_val =
                isl_mat_get_element_val(flow_nullspace_coeff_matrix, 0, i+1);
        assert( isl_val_is_int(count_val) );
        int count = std::abs( isl_val_get_d(count_val) );
        cout << "- " << *stmt_loc << ": " << count << endl;
        counts.emplace_back(*stmt_loc, count);
        isl_val_free(count_val);
        ++stmt_loc;
    }
}

isl_union_set*
ast_generator::repetition_domains
(isl_union_set *domains, const vector<pair<string, int>> & counts)
{
    isl_union_set *repetition_domains = nullptr;

    for (auto item : counts)
    {
        const string & stmt_name = item.first;
        int stmt_count = item.second;
        statement *stmt = m_statements[stmt_name];
        assert(stmt);

        vector<int> infinite_dims = infinite_dimensions(stmt);
        assert(infinite_dims.size() == 1);
        int constrained_dim = infinite_dims.front();

        isl_space *space = isl_space_set_alloc(m_ctx, 0, stmt->domain.size());
        space = isl_space_set_tuple_name(space, isl_dim_set, stmt_name.c_str());

        isl_basic_set *constrained_domain = isl_basic_set_universe(isl_space_copy(space));
        isl_local_space *constraint_space = isl_local_space_from_space(space);

        isl_constraint *constraint;
        constraint = isl_inequality_alloc(constraint_space);
        constraint = isl_constraint_set_coefficient_si(constraint, isl_dim_set,
                                                       constrained_dim, -1);
        constraint = isl_constraint_set_constant_si(constraint, stmt_count-1);
        constrained_domain =
                isl_basic_set_add_constraint(constrained_domain, constraint);

        cout << endl << endl << "Constrained domain:" << endl;
        m_printer.print(constrained_domain);
        cout << endl;

        isl_union_set *to_intersect
                = isl_union_set_from_basic_set(constrained_domain);

        isl_union_set *repetition_domain =
                isl_union_set_intersect(isl_union_set_copy(domains),
                                        to_intersect);

        if (repetition_domains)
            repetition_domains =
                    isl_union_set_union(repetition_domains, repetition_domain);
        else
            repetition_domains = repetition_domain;
    }

    cout << endl << "Repetition domains:" << endl;
    m_printer.print(repetition_domains);
    cout << endl;

    return repetition_domains;
}

void ast_generator::dependencies( expression *expr,
                                  vector<stream_access*> & deps )
{
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto sub_expr : operation->operands)
            dependencies(sub_expr, deps);
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

void ast_generator::dataflow_dependencies
( const statement_info &info, vector<dataflow_dependency> & all_deps )
{
    const string &sink_name = info.first;
    statement *sink = info.second;

    if (!sink->expr)
        return;

    vector<int> infinite_dims = infinite_dimensions(sink);
    assert(infinite_dims.size() == 1);

    int sink_flow_dim = infinite_dims.front();

    cout << "-- sink flow dim = " << sink_flow_dim << endl;

    vector<stream_access*> sources;

    dependencies(sink->expr, sources);
    if (sources.empty())
        return;

    for(stream_access *source : sources)
    {
        int source_flow_dim = -1;
        for (int out_dim = 0;
             out_dim < source->pattern.output_dimension();
             ++out_dim)
        {
            cout << "-- coef: " << source->pattern.coefficients(out_dim, sink_flow_dim) << endl;
            if (source->pattern.coefficients(out_dim, sink_flow_dim) != 0)
            {
                source_flow_dim = out_dim;
                break;
            }
        }

        if (source_flow_dim < 0)
            throw std::runtime_error("Sink flow dimension does not map"
                                     " to any source dimension.");

        int flow_pop = source->pattern.coefficients(source_flow_dim, sink_flow_dim);

        vector<int> sink_index = sink->domain;
        sink_index[sink_flow_dim] = 0;
        vector<int> source_index = source->pattern * sink_index;
        int flow_peek = source_index[source_flow_dim] + 1;

        string source_name;
        {
            auto is_source = [&]( const statement_info &info )
            { return info.second == source->target; };
            auto it = std::find_if(m_statements.begin(), m_statements.end(), is_source);
            assert(it != m_statements.end());
            source_name = it->first;
        }

        dataflow_dependency dep;
        dep.source = source_name;
        dep.sink = sink_name;
        dep.source_dim = source_flow_dim;
        dep.sink_dim = sink_flow_dim;
        dep.push = 1;
        dep.peek = flow_peek;
        dep.pop = flow_pop;

        all_deps.push_back(dep);

        cout << endl << "Dependency:" << endl;
        cout << dep.source << "@" << dep.source_dim << " " << dep.push << " -> "
             << dep.peek << "/" << dep.pop << " " << dep.sink << "@" << dep.sink_dim
             << endl;
    }
}

vector<int> ast_generator::infinite_dimensions( statement *stmt )
{
    vector<int> infinite_dimensions;

    for (int dim = 0; dim < stmt->domain.size(); ++dim)
        if (stmt->domain[dim] == infinite)
            infinite_dimensions.push_back(dim);

    return std::move(infinite_dimensions);
}

isl_ast_node*
ast_generator::generate_ast
(isl_union_map *schedule, isl_union_set *domain)
{
    auto schedule_domain =
            isl_union_set_apply(domain, isl_union_map_copy(schedule));

    //printf("# Schedule domain:\n");
    //isl_printer_print_union_set(printer, schedule_ranges);
    //printf("\n");

    auto bounded_schedule =
            isl_union_map_intersect_range(isl_union_map_copy(schedule),
                                          schedule_domain);

    //printf("# Bounded schedule:\n");
    //isl_printer_print_union_map(printer, bounded_schedule);
    //printf("\n");

    isl_ast_node * ast = isl_ast_build_ast_from_schedule(m_ast_builder, bounded_schedule);
    assert(ast);

    return ast;
}

}
}
