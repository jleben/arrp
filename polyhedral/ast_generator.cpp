#include "ast_generator.hpp"

#include <osl/osl.h>
#include <pluto/libpluto.h>

#include <cloog/cloog.h>
#include <cloog/isl/cloog.h>

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
        compute_dataflow_counts(dataflow_deps, m_dataflow_counts);
    }

    /*
      Compute schedule with domains
      offset by init counts and
      spanning steady counts.

      For the purpose of buffer calculation,
      Construct a larger infinite schedule involving
      entire execution (init phase and all steady periods).
      Should be able to use original dependencies, cuz iteration
      indexes (domains) don't change.

      For buffer access index, use index in schedule minus init count.
    */

    isl::union_set domains(m_ctx);
    isl::union_map dependencies(m_ctx);
    make_isl_representation(domains, dependencies);

    isl::union_set dataflow_domains(m_ctx);
    isl::union_map dataflow_dependencies(m_ctx);
    dataflow_model(domains, dependencies, m_dataflow_counts,
                   dataflow_domains, dataflow_dependencies);

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

    compute_buffer_sizes(all_steady_periods, dataflow_dependencies);

    //return nullptr;


#if 1
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
#endif

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

    return isl::union_map(schedule);
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
    using isl::tuple;

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
    using isl::tuple;
    using isl::expression;

    space src_space = dependency.domain().get_space();
    space sink_space = dependency.range().get_space();

    const dataflow_count & src_count =
            m_dataflow_counts.at(src_space.name(space::variable));

    space src_sched_space = space::from(src_space, time_space);
    space sink_sched_space = space::from(sink_space, time_space);

    map src_sched = schedule.map_for(src_sched_space);
    map sink_sched = schedule.map_for(sink_sched_space);

    map not_later = order_greater_than_or_equal(time_space);
    map not_earlier = order_less_than_or_equal(time_space);

    map src_not_later = src_sched.inverse()( not_later );
    map sink_not_earlier = sink_sched.inverse()( not_earlier );
    map src_consumed_not_earlier =
            dependency.inverse()( sink_not_earlier ).in_range(src_sched.domain());

    src_not_later.coalesce();
    src_consumed_not_earlier.coalesce();

    cout << "--produced:" << endl;
    m_printer.print(src_not_later); cout << endl;
    //m_printer.print(sink_not_earlier); cout << endl;
    cout << "--consumed:" << endl;
    m_printer.print(src_consumed_not_earlier); cout << endl;

    auto combined = (src_not_later * src_consumed_not_earlier).range();

    //cout << "Combined:" << endl;
    //m_printer.print(combined); cout << endl;

    string source_name = src_space.name(isl::space::variable);
    statement *source_stmt = m_statements.at(source_name).stmt;
    int dim_count = src_space.dimension(isl::space::variable);
    int dim = first_infinite_dimension(source_stmt) + 1;
    assert(dim >= 0);

    isl::local_space opt_space(combined.get_space());
    auto x0 = expression::variable(opt_space, isl::space::variable, 0);
    auto x1 = expression::variable(opt_space, isl::space::variable, dim);
    auto y0 = expression::variable(opt_space, isl::space::variable, dim_count);
    auto y1 = expression::variable(opt_space, isl::space::variable, dim_count+dim);
    auto cost =
            (x0 * src_count.steady + x1) -
            (y0 * src_count.steady + y1);

    auto maximum = combined.maximum(cost);

    cout << "Maximum = "; m_printer.print(maximum); cout << endl;
}

void ast_generator::compute_dataflow_dependencies
( vector<dataflow_dependency> & result )
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

    for(const string & name : infinite_statements)
    {
        compute_dataflow_dependencies(*m_statements.find(name), result);
    }
}


void ast_generator::compute_dataflow_dependencies
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
        int flow_peek = std::max(1, source_index[source_flow_dim]);

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

void ast_generator::compute_dataflow_counts
( const vector<dataflow_dependency> & deps,
  std::unordered_map<string,dataflow_count> & result )
{
    using namespace isl;

    // FIXME: Multiple dependencies between same pair of statements

    unordered_set<string> involved_stmts;

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

        cout << "source: " << dep.source << "@" << source_index << endl;
        cout << "sink: " << dep.sink << "@" << sink_index << endl;
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

    isl::space statement_space(m_ctx, isl::tuple(), isl::tuple("",involved_stmts.size()));
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
    auto stmt_name_ref = involved_stmts.begin();
    for (int stmt_idx = 0; stmt_idx < involved_stmts.size(); ++stmt_idx, ++stmt_name_ref)
    {
        dataflow_count counts =
        {
            (int) init_optimum_point(isl::space::variable, stmt_idx).numerator(),
            (int) steady_counts(stmt_idx,0).value().numerator()
        };
        result.emplace(*stmt_name_ref, counts);
    }
}

void ast_generator::dataflow_model
( const isl::union_set & domains,
  const isl::union_map & dependencies,
  const unordered_map<string,dataflow_count> & counts,
  isl::union_set & dataflow_domains,
  isl::union_map & dataflow_dependencies)
{
    isl::union_map domain_maps(m_ctx);

    domains.for_each( [&](const isl::set & d)
    {
        string stmt_name = d.name();
        statement *stmt = m_statements.at(stmt_name).stmt;
        int inf_dim = first_infinite_dimension(stmt);
        const auto & count = counts.at(stmt_name);

        auto d_space = d.get_space();

        // Output domain space

        auto dd_space = d_space;
        dd_space.insert_dimensions(isl::space::variable,0,1);
        dd_space.set_name(isl::space::variable,  stmt_name);

        // Compute input->output mapping

        int d_dims = d.dimensions();
        int dd_dims = d_dims + 1;
        int column_count = d_dims + dd_dims + 1;

        isl::matrix eq_mtx(m_ctx, d_dims, column_count, 0 );

        // Compute relation between period and intra-period (iteration) domains
        // in = (out_period * steady) + out + init
        eq_mtx(inf_dim, inf_dim) = -1; // input iteration
        eq_mtx(inf_dim, d_dims) = count.steady; // output period
        eq_mtx(inf_dim, d_dims + inf_dim + 1) = 1; // output iteration
        eq_mtx(inf_dim, d_dims + dd_dims) = count.init; // constant

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
            dd.add_constraint(v < count.steady);
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

isl::union_set ast_generator::steady_period
( const isl::union_set & domains,
  const unordered_map<string,dataflow_count> & counts )
{
    isl::union_set period(m_ctx);

    //cout << "All steady periods: "; m_printer.print(periods); cout << endl;

    for( const auto & elem : counts )
    {
        const string & name = elem.first;
        const dataflow_count & count = elem.second;
        statement *stmt = m_statements[name].stmt;
        int dim = first_infinite_dimension(stmt);
        assert(dim >= 0);

        isl::space space(m_ctx, isl::tuple(), isl::tuple(name,stmt->domain.size()));
        auto domain = domains.set_for(space);
        //auto period = isl::set::universe(space);
        auto iter = space(isl::space::variable, dim);
        domain.add_constraint( iter >= count.init );
        domain.add_constraint( iter < (count.init + count.steady) );

        //cout << "Steady domain: "; m_printer.print(domain); cout << endl;

        period = period | domain;
    }

    return period;
}

#if 0
pair<isl_union_set*, isl_union_set*>
ast_generator::dataflow_iteration_domains(isl_union_set* domains)
{
    isl_union_set *rep_domains = repetition_domains(domains, counts);

    // TODO: initialization domains
    // - each stmt must get its "peek" amount of elems available

    return make_pair((isl_union_set*) nullptr, rep_domains);
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
#endif

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
