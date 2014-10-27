#include "ast_generator.hpp"

#include <osl/osl.h>
#include <pluto/libpluto.h>
#include <cloog/isl/cloog.h>

#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/utility.hpp>
#include <isl-cpp/printer.hpp>

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
    m_printer(m_ctx)
{
    m_ctx.set_error_action(isl::context::abort_on_error);
}

ast_generator::~ast_generator()
{
}

clast_stmt *
ast_generator::generate( const vector<statement*> & statements )
{
    store_statements(statements);

    isl::union_set domains(m_ctx);
    isl::union_map dependencies(m_ctx);
    make_isl_representation(domains, dependencies);

    auto schedule = make_schedule(domains, dependencies);
    auto bounded_schedule = schedule.in_domain( domains );

    cout << endl << "Schedule:" << endl;
    m_printer.print(bounded_schedule);
    cout << endl;

    compute_buffer_sizes(bounded_schedule, dependencies);

    {
        CloogState *state = cloog_state_malloc();
        CloogOptions *options = cloog_options_malloc(state);
        CloogUnionDomain *schedule =
                cloog_union_domain_from_isl_union_map(bounded_schedule.copy());
        //CloogMatrix *dummy_matrix = cloog_matrix_alloc(0,0);

        CloogDomain *context_domain =
                cloog_domain_from_isl_set(isl_set_universe(bounded_schedule.get_space().copy()));
                //cloog_domain_from_cloog_matrix(state, dummy_matrix, 0);
        CloogInput *input =  cloog_input_alloc(context_domain, schedule);

        //cout << "--- Cloog input:" << endl;
        //cloog_input_dump_cloog(stdout, input, options);
        //cout << "--- End Cloog input ---" << endl;

        if (!input)
            cout << "Hmm no Cloog input..." << endl;

        struct clast_stmt *ast = cloog_clast_create_from_input(input, options);

        cout << endl << "--- Cloog AST:" << endl;
        clast_pprint(stdout, ast, 0, options);

        return ast;
    }

#if 0
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
#endif
}

void ast_generator::store_statements( const vector<statement*> & statements )
{
    int idx = 0;
    for (auto stmt : statements)
    {
        ostringstream name;
        name << "S_" << idx;
        statement_data &data = m_statements[name.str()];
        data.stmt = stmt;
        ++idx;
    }
}

void ast_generator::make_isl_representation
(isl::union_set & all_domains, isl::union_map & all_dependencies)
{
    for (const auto & entry : m_statements)
    {
        auto domain = isl_iteration_domain(entry);
        all_domains = all_domains | domain;
    }

    for (const auto & stmt_info : m_statements)
    {
        if (!stmt_info.second.stmt->expr)
            continue;

        auto dependency = isl_dependencies(stmt_info);
        all_dependencies = all_dependencies | dependency;
    }
}

isl::basic_set ast_generator::isl_iteration_domain( const statement_info & stmt_info )
{
    using isl::tuple;

    const string & name = stmt_info.first;
    statement *stmt = stmt_info.second.stmt;

    auto space = isl::space(m_ctx, tuple(), tuple(name, stmt->domain.size()));
    auto domain = isl::basic_set::universe(space);
    auto constraint_space = isl::local_space(space);

    for (int dim = 0; dim < stmt->domain.size(); ++dim)
    {
        int extent = stmt->domain[dim];

        auto dim_var = isl::expression::variable(constraint_space, isl::space::variable, dim);

        auto lower_bound = dim_var >= 0;
        domain.add_constraint(lower_bound);

        if (extent >= 0)
        {
            auto upper_bound = dim_var < extent;
            domain.add_constraint(upper_bound);
        }
    }

    cout << endl << "Iteration domain:" << endl;
    isl::printer p(m_ctx);
    p.print(domain);
    cout << endl;

    return domain;
}

isl::union_map ast_generator::isl_dependencies( const statement_info & stmt_info )
{
    using isl::tuple;

    // We assume that a statement only writes one scalar value at a time.
    // Therefore, a dependency between two statements is exactly
    // the polyhedral::stream_access::pattern in the model.

    const string & source_name = stmt_info.first;
    statement *source = stmt_info.second.stmt;

    vector<stream_access*> deps;

    dependencies(source->expr, deps);

    isl::union_map all_dependencies_map(m_ctx);

    for (auto dep : deps)
    {
        auto stmt_is_dep_target = [&dep]( const statement_info & stmt_info )
        {
            return stmt_info.second.stmt == dep->target;
        };

        const auto & target_info =
                * std::find_if(m_statements.begin(), m_statements.end(),
                               stmt_is_dep_target);
        const string & target_name = target_info.first;

        // NOTE: "input" and "output" are swapped in the ISL model.
        // "input" = dependee
        // "output" = depender

        isl::space space(m_ctx,
                         tuple(),
                         tuple(target_name.c_str(), dep->target->domain.size()),
                         tuple(source_name.c_str(), source->domain.size()));

        auto equalities = isl_constraint_matrix(dep->pattern);
        auto inequalities = isl::matrix(m_ctx, 0, equalities.column_count());

        isl::basic_map dependency_map(space, equalities, inequalities);

        cout << endl << "Dependency:" << endl;
        m_printer.print(dependency_map);
        cout << endl;

        all_dependencies_map = all_dependencies_map | dependency_map;
    }

    return all_dependencies_map;
}

isl::matrix ast_generator::isl_constraint_matrix( const mapping & map )
{
    // one constraint for each output dimension
    int rows = map.output_dimension();
    // output dim + input dim + a constant
    int cols = map.output_dimension() + map.input_dimension() + 1;

    isl::matrix matrix(m_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            matrix(r,c) = 0;

    for (int out_dim = 0; out_dim < map.output_dimension(); ++out_dim)
    {
        // Put output index on the other side of the equality (negate):
        matrix(out_dim, out_dim) = -1;

        for (int in_dim = 0; in_dim < map.input_dimension(); ++in_dim)
        {
            int col = in_dim + map.output_dimension();
            int coef = map.coefficients(out_dim, in_dim);
            matrix(out_dim, col) = coef;
        }

        int offset = map.constants[out_dim];
        matrix(out_dim, cols-1) = offset;
    }

    return matrix;
}

isl::union_map ast_generator::make_schedule
(isl::union_set & domains, isl::union_map & dependencies)
{

    PlutoOptions *options = pluto_options_alloc();
    options->silent = 1;
    options->quiet = 1;
    options->debug = 0;
    options->moredebug = 0;
    //options->islsolve = 1;
    //options->fuse = MAXIMAL_FUSE;
    //options->unroll = 1;
    //options->polyunroll = 1;
    //options->ufactor = 2;
    //options->tile = 1;
    //options->parallel = 1;

    isl_union_map *schedule =
            pluto_schedule( domains.get(),
                            dependencies.get(),
                            options);

    pluto_options_free(options);

    return isl::union_map(schedule);
}

struct time_space_data
{
    isl_space *space = nullptr;
};

int get_time_space(isl_map *schedule, void *d)
{
    auto data = (time_space_data*) d;
    if (!data->space)
    {
        isl_set *schedule_range = isl_map_range(schedule);
        data->space = isl_set_get_space(schedule_range);
        isl_set_free(schedule_range);
    }
    else
        isl_map_free(schedule);
    return -1;
}

struct buffer_size_data
{
    ast_generator *generator;
    isl::union_map schedule;
    isl::space time_space;
};

int compute_buffer_size_helper(isl_map *dependence, void *d)
{
    auto data = reinterpret_cast<buffer_size_data*>(d);
    data->generator->compute_buffer_size(data->schedule,
                                         isl::map(dependence),
                                         data->time_space);
    return 0;
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const isl::union_map & dependencies )
{
    using namespace isl;
    using isl::tuple;

    cout << endl;

    time_space_data t;

    isl_union_map_foreach_map(schedule.get(), &get_time_space, &t);
    assert(t.space);

    buffer_size_data d
    { this, schedule, isl::space(t.space) };

    cout << "Each dependence:" << endl;

    isl_union_map_foreach_map(dependencies.get(), &compute_buffer_size_helper, &d);

    cout << endl;
}

void ast_generator::compute_buffer_size( const isl::union_map & schedule,
                                         const isl::map & dependence,
                                         const isl::space & time_space )
{
    using namespace isl;
    using isl::tuple;
    using isl::expression;

    space src_space = dependence.domain().get_space();
    space sink_space = dependence.range().get_space();
    space src_sched_space = space::from(src_space, time_space);
    space sink_sched_space = space::from(sink_space, time_space);

    map src_sched = schedule.map_for(src_sched_space);
    map sink_sched = schedule.map_for(sink_sched_space);
    m_printer.print(dependence); cout << endl;
    //p.print(src_sched); cout << endl;
    //p.print(sink_sched); cout << endl;

    map not_later = order_greater_than_or_equal(time_space);
    map not_earlier = order_less_than_or_equal(time_space);

    map src_not_later = src_sched.inverse()( not_later );
    map sink_not_earlier = sink_sched.inverse()( not_earlier );
    map src_consumed_not_earlier = dependence.inverse()( sink_not_earlier );

    m_printer.print(src_not_later); cout << endl;
    m_printer.print(sink_not_earlier); cout << endl;
    m_printer.print(src_consumed_not_earlier); cout << endl;

    auto distance_func = []( const space & s ) -> map
    {
        int dimensions = s.dimension(space::variable);
        local_space distance_space = local_space(range_product(s,s));
        auto a = isl::expression::variable(distance_space, space::variable, 0);
        auto b = isl::expression::variable(distance_space, space::variable, dimensions);
        return map(a - b);
    };

    map distance = distance_func(src_space);
    map delta = distance(src_not_later * src_consumed_not_earlier);
    cout << "delta := "; m_printer.print(delta); cout << endl;

    set range = delta.range();
    cout << "range := "; m_printer.print(range); cout << endl;

    set max_delta = range.lex_maximum();
    max_delta.coalesce();
    cout << "max delta := ";
    m_printer.print(max_delta);
    cout << endl;
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
        statement *stmt = info.second.stmt;
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
            isl_mat_alloc(m_ctx.get(), rows, cols);

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

    isl_mat *dummy_inequalities_matrix = isl_mat_alloc(m_ctx.get(), 0, cols);
    isl_space *space = isl_space_set_alloc(m_ctx.get(), 0, involved_stmts.size());
    isl_basic_set *flow_nullspace =
            isl_basic_set_from_constraint_matrices
            (space, flow_matrix, dummy_inequalities_matrix,
             isl_dim_set, isl_dim_cst, isl_dim_param, isl_dim_div);

    assert(flow_nullspace);
    cout << "Nullspace:" << endl;
    // FIXME:
    //m_printer.print(flow_nullspace); cout << endl;

    isl_basic_set * flow_nullspace_coeff =
            isl_basic_set_coefficients(flow_nullspace);

    assert(flow_nullspace_coeff);
    cout << "Nullspace coefficients:" << endl;
    // FIXME:
    //m_printer.print(flow_nullspace_coeff); cout << endl;

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
        statement *stmt = m_statements[stmt_name].stmt;
        assert(stmt);

        vector<int> infinite_dims = infinite_dimensions(stmt);
        assert(infinite_dims.size() == 1);
        int constrained_dim = infinite_dims.front();

        isl_space *space = isl_space_set_alloc(m_ctx.get(), 0, stmt->domain.size());
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
        // FIXME: m_printer.print(constrained_domain);
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
    // FIXME: m_printer.print(repetition_domains);
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
    statement *sink = info.second.stmt;

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
            { return info.second.stmt == source->target; };
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

}
}
