#include "ast_generator.hpp"

#include <cloog/cloog.h>
#include <cloog/isl/cloog.h>

#include <isl/schedule.h>

#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/matrix.hpp>
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

statement *statement_for( const isl::identifier & id )
{
    return reinterpret_cast<statement*>(id.data);
}

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

    vector<dataflow_dependency> dataflow_deps;
    compute_dataflow_dependencies(dataflow_deps);

    if (!dataflow_deps.empty())
    {
        compute_dataflow_counts(dataflow_deps);
    }

    isl::union_set domains(m_ctx);
    isl::union_map dependencies(m_ctx);
    polyhedral_model(domains, dependencies);

    isl::union_set dataflow_domains(m_ctx);
    isl::union_map dataflow_dependencies(m_ctx);
    dataflow_model(domains, dependencies,
                   dataflow_domains, dataflow_dependencies);

    if (dataflow_domains.is_empty())
        throw error("No infinite statements.");

    isl::union_set first_steady_period(m_ctx);
    dataflow_domains.for_each( [&first_steady_period](isl::set & domain)
    {
        auto v = domain.get_space()(isl::space::variable, 0);
        domain.add_constraint(v == 0);
        first_steady_period = first_steady_period | domain;
        return true;
    });

    cout << endl;
    cout << "First steady period:" << endl;
    m_printer.print(first_steady_period); cout << endl;

    //return nullptr;

    auto schedule = make_schedule(first_steady_period, dataflow_dependencies);

    cout << endl << "Unbounded steady period schedule:" << endl;
    m_printer.print(schedule);
    cout << endl;

    auto bounded_schedule = schedule.in_domain(first_steady_period);

    cout << endl << "Bounded steady period schedule:" << endl;
    m_printer.print(bounded_schedule);
    cout << endl;


    auto all_steady_periods = entire_steady_schedule(bounded_schedule);

    cout << endl << "Steady schedule:" << endl;
    m_printer.print(all_steady_periods);
    cout << endl;

#if 1
    struct clast_stmt *ast;
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

        ast = cloog_clast_create_from_input(input, options);

        cout << endl << "--- Cloog AST:" << endl;
        clast_pprint(stdout, ast, 0, options);
    }
#endif

#if 1
    compute_buffer_sizes(all_steady_periods, dataflow_dependencies);

    // Convert buffer size to flat buffer size

    cout << endl << "Buffer sizes:" << endl;
    for (statement *stmt : m_statements)
    {
        int flow_dim_size = stmt->buffer_size ? stmt->buffer_size : stmt->steady_count;

        int flat_buf_size = 0;
        for (int d = 0; d < stmt->domain.size(); ++d)
        {
            int size = (d == stmt->dimension) ? flow_dim_size : stmt->domain[d];
            if (d == 0)
                flat_buf_size = size;
            else
                flat_buf_size *= size;
        }

        stmt->buffer_size = flat_buf_size;

        cout << stmt->name << ": buffer size = " << flat_buf_size << endl;
    }
#endif
    return ast;
}

void ast_generator::store_statements( const vector<statement*> & statements )
{
    m_statements = statements;
    int idx = 0;
    for (statement * stmt : m_statements)
    {
        ostringstream name;
        name << "S_" << idx;
        stmt->name = name.str();
        ++idx;
    }
}

void ast_generator::polyhedral_model
(isl::union_set & all_domains, isl::union_map & all_dependencies)
{
    cout << endl;
    cout << "### Polyhedral model ###" << endl;

    for (statement * stmt : m_statements)
    {
        auto domain = polyhedral_domain(stmt);
        all_domains = all_domains | domain;
    }

    for (statement * stmt : m_statements)
    {
        auto dependency = polyhedral_dependencies(stmt);
        all_dependencies = all_dependencies | dependency;
    }
}

isl::basic_set ast_generator::polyhedral_domain( statement *stmt )
{
    using isl::tuple;

    const string & name = stmt->name;

    auto space = isl::space( m_ctx,
                             isl::set_tuple( isl::identifier(stmt->name, stmt),
                                             stmt->domain.size() ) );
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

    cout << "Iteration domain: "; m_printer.print(domain); cout << endl;

    return domain;
}

isl::union_map ast_generator::polyhedral_dependencies( statement * dependent )
{
    using isl::tuple;

    // We assume that a statement only writes one scalar value at a time.
    // Therefore, a dependency between two statements is exactly
    // the polyhedral::stream_access::pattern in the model.

    isl::union_map all_dependencies_map(m_ctx);

    vector<stream_access*> stream_accesses;
    find_nodes<stream_access>(dependent->expr, stream_accesses);

    for (auto access : stream_accesses)
    {
        statement *target = access->target;

        // NOTE: "input" and "output" are swapped in the ISL model.
        // "input" = source
        // "output" = sink

        isl::input_tuple target_tuple(isl::identifier(target->name, target),
                                      target->domain.size());
        isl::output_tuple dependent_tuple(isl::identifier(dependent->name, dependent),
                                       dependent->domain.size());
        isl::space space(m_ctx, target_tuple, dependent_tuple);

        auto equalities = constraint_matrix(access->pattern);
        auto inequalities = isl::matrix(m_ctx, 0, equalities.column_count());

        isl::basic_map dependency_map(space, equalities, inequalities);

        cout << "Dependency: ";
        m_printer.print(dependency_map); cout << endl;

        all_dependencies_map = all_dependencies_map | dependency_map;
    }

    vector<reduction_access*> reduction_accesses;
    find_nodes<reduction_access>(dependent->expr, reduction_accesses);

    for (auto access : reduction_accesses)
    {
#if 1
        isl::output_tuple dependent_space
                (isl::identifier(dependent->name, dependent),
                 dependent->domain.size());

        // Initialization dependence
        {
            statement *initializer = access->initializer;

            assert(dependent->domain.size() >= initializer->domain.size());

            isl::input_tuple initializer_space
                    (isl::identifier(initializer->name, initializer),
                     initializer->domain.size());

            isl::space space(m_ctx, initializer_space, dependent_space);

            int coef_count = initializer->domain.size() + dependent->domain.size() + 1;
            int dep_coef = initializer->domain.size();
            int const_coef = coef_count - 1;
            int reduction_dim = initializer->domain.size();
            if (reduction_dim > dependent->domain.size() - 1)
                --reduction_dim;

            // Equalities
            int eq_count = reduction_dim + 1;
            auto equalities = isl::matrix(m_ctx, eq_count, coef_count, 0);
            // Constraints: Initial dimensions are equal
            for (int dim = 0; dim < reduction_dim; ++dim)
            {
                equalities(dim, dim) = 1;
                equalities(dim, dep_coef + dim) = -1;
            }
            // Constraint: Dependent reduction dimension is 0:
            equalities(reduction_dim, dep_coef + reduction_dim) = 1;

            // No inequalities
            auto inequalities = isl::matrix(m_ctx, 0, coef_count);

            isl::basic_map dependency_map(space, equalities, inequalities);
            all_dependencies_map = all_dependencies_map | dependency_map;

            cout << "Reduction initializer dependency: ";
            m_printer.print(dependency_map); cout << endl;
        }

        // Reduction dependence
        {
            statement *reductor = access->reductor;

            assert(dependent->domain.size() >= reductor->domain.size());

            isl::input_tuple reductor_space
                    (isl::identifier(reductor->name, reductor),
                     reductor->domain.size());

            isl::space space(m_ctx, reductor_space, dependent_space);

            int coef_count = reductor->domain.size() + dependent->domain.size() + 1;
            int dep_coef = reductor->domain.size();
            int const_coef = coef_count - 1;
            int reduction_dim = reductor->domain.size() - 1;

            // Equalities
            int eq_count = reduction_dim + 1;
            auto equalities = isl::matrix(m_ctx, eq_count, coef_count, 0);
            // Constraints: Initial dimensions are equal
            for (int dim = 0; dim < reduction_dim; ++dim)
            {
                equalities(dim, dim) = 1;
                equalities(dim, dep_coef + dim) = -1;
            }
            // Constraint:
            // r = reduction dimension; reduction(r) - dependent(r) + 1 = 0;
            equalities(reduction_dim, reduction_dim) = 1;
            equalities(reduction_dim, dep_coef + reduction_dim) = -1;
            equalities(reduction_dim, const_coef) = 1;

            // Inequalities
            auto inequalities = isl::matrix(m_ctx, 1, coef_count);
            // Constraint: dependent(reduction_dim) >= 1
            inequalities(0, dep_coef + reduction_dim) = 1;
            inequalities(0, const_coef) = -1;

            isl::basic_map dependency_map(space, equalities, inequalities);
            all_dependencies_map = all_dependencies_map | dependency_map;

            cout << "Reduction self-dependency: ";
            m_printer.print(dependency_map); cout << endl;
        }
#endif
    }

    return all_dependencies_map;
}

isl::matrix ast_generator::constraint_matrix( const mapping & map )
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

void ast_generator::compute_dataflow_dependencies
( vector<dataflow_dependency> & result )
{
    vector<statement*> finite_statements;
    vector<statement*> infinite_statements;
    vector<statement*> invalid_statements;

    for(statement *stmt : m_statements)
    {
        vector<int> infinite_dims = infinite_dimensions(stmt);
        if (infinite_dims.empty())
            finite_statements.push_back(stmt);
        else if (infinite_dims.size() == 1)
        {
            stmt->dimension = infinite_dims.front();
            infinite_statements.push_back(stmt);
        }
        else
            invalid_statements.push_back(stmt);
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
        for (statement *stmt: invalid_statements)
        {
            msg << "- " << stmt->name << endl;
        }
        throw error(msg.str());
    }

    for(statement *stmt : infinite_statements)
    {
        compute_dataflow_dependencies(stmt, result);
    }
}


void ast_generator::compute_dataflow_dependencies
( statement *sink, vector<dataflow_dependency> & all_deps )
{
    cout << "-- sink flow dim = " << sink->dimension << endl;

    vector<stream_access*> sources;

    find_nodes<stream_access>(sink->expr, sources);
    if (sources.empty())
        return;

    for(stream_access *source : sources)
    {
        int source_flow_dim = -1;
        for (int out_dim = 0;
             out_dim < source->pattern.output_dimension();
             ++out_dim)
        {
            cout << "-- coef: " << source->pattern.coefficients(out_dim, sink->dimension) << endl;
            if (source->pattern.coefficients(out_dim, sink->dimension) != 0)
            {
                source_flow_dim = out_dim;
                break;
            }
        }

        if (source_flow_dim < 0)
            throw std::runtime_error("Sink flow dimension does not map"
                                     " to any source dimension.");
        if (source_flow_dim != source->target->dimension)
            throw std::runtime_error("Sink flow dimension does not map"
                                     " to source flow dimension.");

        int flow_pop = source->pattern.coefficients(source_flow_dim, sink->dimension);

        vector<int> sink_index = sink->domain;
        sink_index[sink->dimension] = 0;
        vector<int> source_index = source->pattern * sink_index;
        int flow_peek = std::max(1, source_index[source_flow_dim]);

        dataflow_dependency dep;
        dep.source = source->target;
        dep.sink = sink;
        //dep.source_dim = source_flow_dim;
        //dep.sink_dim = sink_flow_dim;
        dep.push = 1;
        dep.peek = flow_peek;
        dep.pop = flow_pop;

        all_deps.push_back(dep);

        cout << endl << "Dependency:" << endl;
        cout << dep.source << "@" << source->target->dimension << " " << dep.push << " -> "
             << dep.peek << "/" << dep.pop << " " << dep.sink << "@" << sink->dimension
             << endl;
    }
}

void ast_generator::compute_dataflow_counts
( const vector<dataflow_dependency> & deps )
{
    using namespace isl;

    // FIXME: Multiple dependencies between same pair of statements

    unordered_set<statement*> involved_stmts;

    for (const auto & dep: deps)
    {
        involved_stmts.insert(dep.source);
        involved_stmts.insert(dep.sink);
    }

    int rows = deps.size();
    int cols = involved_stmts.size();

    isl::matrix flow_matrix(m_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for(int c = 0; c < cols; ++c)
            flow_matrix(r,c) = 0;

    int row = 0;
    for (const auto & dep: deps)
    {
        auto source_loc = involved_stmts.find(dep.source);
        int source_index = std::distance(involved_stmts.begin(), source_loc);
        auto sink_loc = involved_stmts.find(dep.sink);
        int sink_index = std::distance(involved_stmts.begin(), sink_loc);

        cout << "source: " << dep.source->name << "@" << source_index << endl;
        cout << "sink: " << dep.sink->name << "@" << sink_index << endl;
        flow_matrix(row, source_index) = dep.push;
        flow_matrix(row, sink_index) = - dep.pop;
        ++row;
    }

    cout << "Flow:" << endl;
    isl::print(flow_matrix);

    isl::matrix steady_counts = flow_matrix.nullspace();

    cout << "Steady Counts:" << endl;
    isl::print(steady_counts);

    // Initialization counts:

    // Number of tokens produced should be at least number of tokens consumed
    // after the initial epoch + one steady period.

    isl::space statement_space(m_ctx, isl::set_tuple(involved_stmts.size()));
    auto init_counts = isl::set::universe(statement_space);
    auto init_cost = isl::expression::value(statement_space, 0);
    cout << "Init count cost expr:" << endl;
    m_printer.print(init_cost); cout << endl;
    for (int i = 0; i < involved_stmts.size(); ++i)
    {
        auto stmt = isl::expression::variable(statement_space, space::variable, i);
        init_counts.add_constraint( stmt >= 0 );
        init_cost = stmt + init_cost;
        cout << "Init count cost expr:" << endl;
        m_printer.print(init_cost); cout << endl;
    }
    for (const auto & dep: deps)
    {
        auto source_ref = involved_stmts.find(dep.source);
        int source_index = std::distance(involved_stmts.begin(), source_ref);
        auto sink_ref = involved_stmts.find(dep.sink);
        int sink_index = std::distance(involved_stmts.begin(), sink_ref);

        auto source = isl::expression::variable(statement_space,
                                                space::variable,
                                                source_index);
        auto sink = isl::expression::variable(statement_space,
                                              space::variable,
                                              sink_index);
        int source_steady = steady_counts(source_index,0).value().numerator();
        int sink_steady = steady_counts(sink_index,0).value().numerator();

        // p(a)*i(a) - o(b)*i(b) + [p(a)*s(a) - o(b)*s(b) - e(b) + o(b)] >= 0

        auto constraint =
                dep.push * source - dep.pop * sink
                + (dep.push * source_steady - dep.pop * sink_steady - dep.peek + dep.pop)
                >= 0;

        init_counts.add_constraint(constraint);
    }

    cout << "Viable initialization counts:" << endl;
    m_printer.print(init_counts); cout << endl;

    auto init_optimum = init_counts.minimum(init_cost);
    init_counts.add_constraint( init_cost == init_optimum );
    auto init_optimum_point = init_counts.single_point();

    cout << "Initialization Counts:" << endl;
    m_printer.print(init_optimum_point); cout << endl;

    assert(steady_counts.column_count() == 1);
    assert(steady_counts.row_count() == involved_stmts.size());
    auto stmt_ref = involved_stmts.begin();
    for (int stmt_idx = 0; stmt_idx < involved_stmts.size(); ++stmt_idx, ++stmt_ref)
    {
        (*stmt_ref)->init_count =
                (int) init_optimum_point(isl::space::variable, stmt_idx).numerator();
        (*stmt_ref)->steady_count =
                steady_counts(stmt_idx,0).value().numerator();
    }
}

void ast_generator::dataflow_model
( const isl::union_set & domains,
  const isl::union_map & dependencies,
  isl::union_set & dataflow_domains,
  isl::union_map & dataflow_dependencies)
{
    isl::union_map domain_maps(m_ctx);

    domains.for_each( [&](const isl::set & d)
    {
        isl::identifier id = d.id();
        statement *stmt = statement_for(id);
        int inf_dim = stmt->dimension;
        if (inf_dim == -1)
            return true;

        auto d_space = d.get_space();

        // Output domain space

        auto dd_space = d_space;
        dd_space.insert_dimensions(isl::space::variable,0);
        dd_space.set_id(isl::space::variable, id);

        // Compute input->output mapping

        int d_dims = d.dimensions();
        int dd_dims = d_dims + 1;
        int column_count = d_dims + dd_dims + 1;

        isl::matrix eq_mtx(m_ctx, d_dims, column_count, 0 );

        // Compute relation between period and intra-period (iteration) domains
        // in = (out_period * steady) + out + init
        eq_mtx(inf_dim, inf_dim) = -1; // input iteration
        eq_mtx(inf_dim, d_dims) = stmt->steady_count; // output period
        eq_mtx(inf_dim, d_dims + inf_dim + 1) = 1; // output iteration
        eq_mtx(inf_dim, d_dims + dd_dims) = stmt->init_count; // constant

        // Make all other dimensions equal
        for (int dim = 0; dim < d_dims; ++dim)
        {
            if (dim == inf_dim)
                continue;
            eq_mtx(dim, dim) = -1;
            eq_mtx(dim, d_dims + dim + 1) = 1;
        }

        // No inequalities
        isl::matrix ineq_mtx(m_ctx, 0, column_count);

        isl::map map = isl::basic_map(isl::space::from(d_space, dd_space), eq_mtx, ineq_mtx );

        // Output domain is a mapping of input domain
        // plus an additional constraint.

        auto dd = map(d);
        {
            auto v = dd_space(isl::space::variable, inf_dim + 1);
            dd.add_constraint(v >= 0);
            dd.add_constraint(v < stmt->steady_count);
        }

        // Store results

        dataflow_domains = dataflow_domains | dd;
        domain_maps = domain_maps | map;

        return true;
    });

    cout << endl;

    cout << "Dataflow domains:" << endl;
    m_printer.print(dataflow_domains); cout << endl;

    cout << "Domain mappings:" << endl;
    m_printer.print(domain_maps); cout << endl;

    dataflow_dependencies = dependencies;
    dataflow_dependencies.map_domain_through(domain_maps);
    dataflow_dependencies.map_range_through(domain_maps);

    cout << "Dataflow dependencies:" << endl;
    m_printer.print(dataflow_dependencies); cout << endl;

    cout << "Bounded dataflow dependencies:" << endl;
    m_printer.print( dataflow_dependencies
                     .in_domain(dataflow_domains)
                     .in_range(dataflow_domains) );
    cout << endl;
}


isl::union_map ast_generator::make_schedule
(isl::union_set & domains, isl::union_map & dependencies)
{
    isl::union_map proximities( dependencies.get_space() );

    isl_union_set *dom = domains.copy();
    isl_union_map *dep = dependencies.copy();
    isl_union_map *prox = isl_union_map_empty( isl_union_map_get_space(dep) );

    isl_schedule *sched = isl_union_set_compute_schedule(dom,dep,prox);
    assert(sched);


    isl_union_map *sched_map = isl_schedule_get_map(sched);

    isl_schedule_free(sched);

    return sched_map;

#if 0
    PlutoOptions *options = pluto_options_alloc();
    options->silent = 1;
    options->quiet = 1;
    options->debug = 0;
    options->moredebug = 0;
    options->islsolve = 1;
    options->fuse = MAXIMAL_FUSE;
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

    // Re-set lost IDs:

    isl::union_map original_schedule(schedule);
    isl::union_map corrected_schedule(m_ctx);

    original_schedule.for_each( [&](isl::map & m)
    {
        string name = m.name(isl::space::input);
        auto name_matches = [&name](statement *stmt){return stmt->name == name;};
        auto stmt_ref =
                std::find_if(m_statements.begin(), m_statements.end(),
                             name_matches);
        assert(stmt_ref != m_statements.end());
        m.set_id(isl::space::input, isl::identifier(name, *stmt_ref));
        corrected_schedule = corrected_schedule | m;
        return true;
    });

    return corrected_schedule;
#endif
}

isl::union_map ast_generator::entire_steady_schedule
( const isl::union_map &period_schedule )
{
    isl::union_map entire_schedule(m_ctx);
    period_schedule.for_each( [&]( isl::map & m )
    {
        m.drop_constraints_with(isl::space::input, 0);

        m.insert_dimensions(isl::space::output, 0, 1);

        auto cnstr = isl::constraint::equality(isl::local_space(m.get_space()));
        cnstr.set_coefficient(isl::space::input, 0, 1);
        cnstr.set_coefficient(isl::space::output, 0, -1);
        m.add_constraint(cnstr);

        entire_schedule = entire_schedule | m;

        return true;
    });

    return entire_schedule;
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const isl::union_map & dependencies )
{
    using namespace isl;

    cout << endl;

    isl::space *time_space = nullptr;

    schedule.for_each([&time_space]( const map & m )
    {
        time_space = new space( m.range().get_space() );
        return false;
    });

    dependencies.for_each([&]( const map & dependency )
    {
        compute_buffer_size(schedule, dependency, *time_space);
        return true;
    });
    cout << endl;

    delete time_space;
}

void ast_generator::compute_buffer_size( const isl::union_map & schedule,
                                         const isl::map & dependency,
                                         const isl::space & time_space )
{
    cout << "Buffer size for dependency: ";
    m_printer.print(dependency); cout << endl;

    using namespace isl;
    using isl::expression;

    // Get info

    space src_space = dependency.domain().get_space();
    space sink_space = dependency.range().get_space();

    space src_sched_space = space::from(src_space, time_space);
    space sink_sched_space = space::from(sink_space, time_space);

    map src_sched = schedule.map_for(src_sched_space);
    map sink_sched = schedule.map_for(sink_sched_space);

    statement *source_stmt = statement_for(src_space.id(isl::space::variable));

    // Do the work

    map not_later = order_greater_than_or_equal(time_space);
    map not_earlier = order_less_than_or_equal(time_space);

    map src_not_later = src_sched.inverse()( not_later );
    map sink_not_earlier = sink_sched.inverse()( not_earlier );
    map src_consumed_not_earlier =
            dependency.inverse()( sink_not_earlier ).in_range(src_sched.domain());

    //src_not_later.coalesce();
    //src_consumed_not_earlier.coalesce();

    //cout << "--produced:" << endl;
    //m_printer.print(src_not_later); cout << endl;
    //m_printer.print(sink_not_earlier); cout << endl;
    //cout << "--consumed:" << endl;
    //m_printer.print(src_consumed_not_earlier); cout << endl;

    auto combined = (src_not_later * src_consumed_not_earlier).range();

    //cout << "Combined:" << endl;
    //m_printer.print(combined); cout << endl;

    int dim_count = src_space.dimension(isl::space::variable);
    int dim = source_stmt->dimension + 1;
    assert(dim >= 0);

    isl::local_space opt_space(combined.get_space());
    auto x0 = expression::variable(opt_space, isl::space::variable, 0);
    auto x1 = expression::variable(opt_space, isl::space::variable, dim);
    auto y0 = expression::variable(opt_space, isl::space::variable, dim_count);
    auto y1 = expression::variable(opt_space, isl::space::variable, dim_count+dim);
    auto cost =
            (x0 * source_stmt->steady_count + x1) -
            (y0 * source_stmt->steady_count + y1);

    auto maximum = combined.maximum(cost);

    cout << "Max delay = "; m_printer.print(maximum); cout << endl;

    // Store result

    assert(maximum.denominator() == 1);
    // "maximum" is index difference, so add 1
    int buf_size = maximum.numerator() + 1;
    if (source_stmt->buffer_size < buf_size)
        source_stmt->buffer_size = buf_size;
}

template<typename T>
void ast_generator::find_nodes( expression * expr, vector<T*> & container )
{
    if (auto node = dynamic_cast<T*>(expr))
    {
        container.push_back(node);
    }

    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto sub_expr : operation->operands)
            find_nodes<T>(sub_expr, container);
        return;
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

int ast_generator::first_infinite_dimension( statement *stmt )
{
    for (int dim = 0; dim < stmt->domain.size(); ++dim)
        if (stmt->domain[dim] == infinite)
            return dim;
    return -1;
}

}
}
