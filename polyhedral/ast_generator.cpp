#include "ast_generator.hpp"
#include "dataflow_model.hpp"

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

ast_generator::ast_generator( const vector<statement*> & statements,
                              const dataflow::model * dataflow ):
    m_printer(m_ctx),
    m_dataflow(dataflow)
{
    store_statements(statements);
    m_ctx.set_error_action(isl::context::abort_on_error);
}

ast_generator::~ast_generator()
{
}

clast_stmt *
ast_generator::generate()
{
    isl::union_set domains(m_ctx);
    isl::union_map dependencies(m_ctx);
    polyhedral_model(domains, dependencies);

    isl::union_set init_domains(m_ctx);
    isl::union_set steady_domains(m_ctx);
    isl::union_map periodic_dependencies(m_ctx);
    isl::union_map domain_map(m_ctx);
    periodic_model(domains, dependencies,
                   domain_map, init_domains, steady_domains,
                   periodic_dependencies);

    auto init_schedule = make_init_schedule(init_domains, periodic_dependencies);
    auto period_schedule = make_steady_schedule(steady_domains, periodic_dependencies);

    auto combined_schedule =
            combine_schedule(init_domains, steady_domains,
                             init_schedule, period_schedule);

    compute_buffer_sizes(combined_schedule, periodic_dependencies, domain_map);

#if 0
    struct clast_stmt *ast;
    {
        CloogState *state = cloog_state_malloc();
        CloogOptions *options = cloog_options_malloc(state);
        CloogUnionDomain *schedule =
                cloog_union_domain_from_isl_union_map(combined_schedule.copy());
        //CloogMatrix *dummy_matrix = cloog_matrix_alloc(0,0);

        CloogDomain *context_domain =
                cloog_domain_from_isl_set(isl_set_universe(combined_schedule.get_space().copy()));
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

#if 0
    compute_buffer_sizes(steady_period_schedule, periodic_dependencies);

    // Convert buffer size to flat buffer size

    cout << endl << "Buffer sizes:" << endl;
    for (statement *stmt : m_statements)
    {
        const dataflow::actor & actor = m_dataflow->actor_for(stmt);

        int flow_dim_size = stmt->buffer_size ? stmt->buffer_size : actor.steady_count;

        int flat_buf_size = 0;
        for (int d = 0; d < stmt->domain.size(); ++d)
        {
            int size = (d == actor.flow_dimension) ? flow_dim_size : stmt->domain[d];
            if (d == 0)
                flat_buf_size = size;
            else
                flat_buf_size *= size;
        }

        stmt->buffer_size = flat_buf_size;

        cout << stmt->name << ": buffer size = " << flat_buf_size << endl;
    }
#endif

    return nullptr;
    //return ast;
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

    assert(stmt->domain.size());

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
    dependent->expr->find<stream_access>(stream_accesses);

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
    dependent->expr->find<reduction_access>(reduction_accesses);

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

void ast_generator::periodic_model
( const isl::union_set & domains,
  const isl::union_map & dependencies,
  isl::union_map & domain_map,
  isl::union_set & init_domains,
  isl::union_set & steady_domains,
  isl::union_map & periodic_dependencies)
{
    isl::union_map init_domain_maps(m_ctx);
    isl::union_map steady_domain_maps(m_ctx);

    domains.for_each( [&](const isl::set & domain)
    {
        isl::identifier id = domain.id();
        auto in_space = domain.get_space();
        auto out_space = in_space;
        out_space.insert_dimensions(isl::space::variable,0);
        out_space.set_id(isl::space::variable, id);
        auto map_space = isl::space::from(in_space, out_space);
        auto constraint_space = isl::local_space(map_space);
        int in_dims = domain.dimensions();

        statement *stmt = statement_for(id);
        auto actor_ptr = m_dataflow->find_actor_for(stmt);
        if (actor_ptr)
        {
            const dataflow::actor & actor = *actor_ptr;

            // init part

            isl::basic_map init_map = isl::basic_map::universe(map_space);

            for (int in_dim = 0; in_dim < in_dims; ++in_dim)
            {
                auto in_var = constraint_space(isl::space::input, in_dim);
                auto out_var = constraint_space(isl::space::output, in_dim+1);
                init_map.add_constraint(out_var == in_var);
            }

            {
                auto out_flow_var = constraint_space(isl::space::output, actor.flow_dimension+1);
                init_map.add_constraint(out_flow_var >= 0);
                init_map.add_constraint(out_flow_var < actor.init_count);
            }
            {
                auto out0_var = constraint_space(isl::space::output, 0);
                init_map.add_constraint(out0_var == -1);
            }

            init_domain_maps = init_domain_maps | init_map;

            // steady part

            isl::basic_map steady_map = isl::basic_map::universe(map_space);

            for (int in_dim = 0; in_dim < in_dims; ++in_dim)
            {
                auto in_var = constraint_space(isl::space::input, in_dim);
                auto out_var = constraint_space(isl::space::output, in_dim+1);

                if (in_dim == actor.flow_dimension)
                {
                    // in[flow] = (out[0] * steady) + out[flow] + init
                    auto out0_var = constraint_space(isl::space::output, 0);
                    auto constraint = in_var == out0_var * actor.steady_count
                            + out_var + actor.init_count;
                    steady_map.add_constraint(constraint);
                }
                else
                {
                    steady_map.add_constraint(out_var == in_var);
                }
            }

            {
                auto out_flow_var = constraint_space(isl::space::output, actor.flow_dimension+1);
                steady_map.add_constraint(out_flow_var >= 0);
                steady_map.add_constraint(out_flow_var < actor.steady_count);
            }
            {
                auto out0_var = constraint_space(isl::space::output, 0);
                steady_map.add_constraint(out0_var >= 0);
            }

            steady_domain_maps = steady_domain_maps | steady_map;
        }
        else
        {
            isl::basic_map init_map = isl::basic_map::universe(map_space);

            for (int in_dim = 0; in_dim < in_dims; ++in_dim)
            {
                auto in_var = constraint_space(isl::space::input, in_dim);
                auto out_var = constraint_space(isl::space::output, in_dim+1);
                init_map.add_constraint(out_var == in_var);
            }
            {
                auto out0_var = constraint_space(isl::space::output, 0);
                init_map.add_constraint(out0_var == -1);
            }

            init_domain_maps = init_domain_maps | init_map;
        }

        return true;
    });

    cout << endl;

    init_domains = init_domain_maps(domains);
    steady_domains = steady_domain_maps(domains);

    cout << "Init domains:" << endl;
    m_printer.print(init_domains); cout << endl;
    cout << "Steady domains:" << endl;
    m_printer.print(steady_domains); cout << endl;

    domain_map = init_domain_maps | steady_domain_maps;

    periodic_dependencies = dependencies;
    periodic_dependencies.map_domain_through(domain_map);
    periodic_dependencies.map_range_through(domain_map);

    cout << "Periodic dependencies:" << endl;
    m_printer.print(periodic_dependencies); cout << endl;
#if 0
    cout << "Bounded periodic dependencies:" << endl;
    m_printer.print( periodic_dependencies
                     .in_domain(periodic_domains)
                     .in_range(periodic_domains) );
#endif
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

isl::union_map
ast_generator::make_init_schedule(isl::union_set & domains,
                                  isl::union_map & dependencies)
{
    isl::union_map init_schedule = make_schedule(domains, dependencies);

    cout << endl << "Init schedule:" << endl;
    //m_printer.print(init_schedule.in_domain(init_domains));
    m_printer.print(init_schedule);
    cout << endl;

    return init_schedule;
}

isl::union_map
ast_generator::make_steady_schedule(isl::union_set & domains,
                                    isl::union_map & dependencies)
{
    //cout << endl << "Steady domains:" << endl;
    isl::union_set period_domains(m_ctx);
    domains.for_each( [&](isl::set & domain)
    {
        isl::identifier id = domain.id();
        statement *stmt = statement_for(id);
        auto actor_ptr = m_dataflow->find_actor_for(stmt);
        assert(actor_ptr);
        const dataflow::actor & actor = *actor_ptr;
        auto period = domain.get_space()(isl::space::variable, 0);
        domain.add_constraint(period == 0);
        period_domains = period_domains | domain;
        //m_printer.print(domain); cout << endl;
        return true;
    });

    isl::union_map steady_schedule = make_schedule(period_domains, dependencies);

    cout << endl << "Steady schedule:" << endl;
    //m_printer.print(steady_schedule.in_domain(steady_domains));
    m_printer.print(steady_schedule);
    cout << endl;

    return steady_schedule;
}

isl::union_map
ast_generator::combine_schedule
( const isl::union_set & init_domains,
  const isl::union_set & steady_domains,
  const isl::union_map & init_schedule,
  const isl::union_map & period_schedule )
{
    isl::union_map canonical_init_schedule(m_ctx);

    init_schedule.for_each( [&]( isl::map & sched )
    {
        sched.insert_dimensions(isl::space::output, 0, 1);

        auto cnstr = isl::constraint::equality(isl::local_space(sched.get_space()));
        cnstr.set_coefficient(isl::space::output, 0, 1);
        cnstr.set_constant(1);
        sched.add_constraint(cnstr);

        canonical_init_schedule = canonical_init_schedule | sched;

        return true;
    });

    isl::union_map canonical_steady_schedule(m_ctx);

    period_schedule.for_each( [&]( isl::map & sched )
    {
        sched.insert_dimensions(isl::space::output, 0, 1);

        auto cnstr = isl::constraint::equality(isl::local_space(sched.get_space()));
        cnstr.set_coefficient(isl::space::input, 0, 1);
        cnstr.set_coefficient(isl::space::output, 0, -1);
        sched.add_constraint(cnstr);

        canonical_steady_schedule = canonical_steady_schedule | sched;

        return true;
    });

    isl::union_map combined_schedule =
            canonical_init_schedule.in_domain(init_domains) |
            canonical_steady_schedule.in_domain(steady_domains);

    cout << endl << "Combined schedule:" << endl;
    m_printer.print(combined_schedule);
    cout << endl;

    return combined_schedule;
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const isl::union_map & dependencies,
                                          const isl::union_map & domain_maps )
{
    using namespace isl;

    cout << endl;

    isl::space *time_space = nullptr;

    isl::union_map domain_unmap = domain_maps.inverse();

    schedule.for_each([&time_space]( const map & m )
    {
        time_space = new space( m.range().get_space() );
        return false;
    });

    dependencies.for_each([&]( const map & dependency )
    {
        compute_buffer_size(schedule, domain_unmap, dependency, *time_space);
        return true;
    });
    cout << endl;

    delete time_space;
}

#define DEBUG_BUFFER_SIZE 1

void ast_generator::compute_buffer_size( const isl::union_map & schedule,
                                         const isl::union_map & domain_unmap,
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

    statement *source_stmt = statement_for(src_space.id(isl::space::variable));
    auto source_actor_ptr = m_dataflow->find_actor_for(source_stmt);
    if (!source_actor_ptr)
    {
        cout << ".. Source not an actor; skipping." << endl;
        return;
    }

    const dataflow::actor & source_actor = *source_actor_ptr;

    // Extract schedule

    space src_sched_space = space::from(src_space, time_space);
    space sink_sched_space = space::from(sink_space, time_space);

    map src_sched = schedule.map_for(src_sched_space);
    map sink_sched = schedule.map_for(sink_sched_space);

    // Extract inverse domain map

    space src_mapped_space = src_space;
    src_mapped_space.drop_dimensions(isl::space::variable, 0);
    src_mapped_space.set_id(isl::space::variable, src_space.id(isl::space::variable));

    space src_unmap_space = isl::space::from(src_space, src_mapped_space);

    map src_unmap = domain_unmap.map_for(src_unmap_space);

    // Do the work

    map not_later = order_greater_than_or_equal(time_space);
    map not_earlier = order_less_than_or_equal(time_space);

    map src_not_later = src_sched.inverse()( not_later );
    map sink_not_earlier = sink_sched.inverse()( not_earlier );
    map src_consumed_not_earlier =
            dependency.inverse()( sink_not_earlier ).in_range(src_sched.domain());

#if DEBUG_BUFFER_SIZE == 1
    cout << ".. produced:" << endl;
    m_printer.print(src_not_later); cout << endl;
    m_printer.print(sink_not_earlier); cout << endl;
    cout << ".. consumed:" << endl;
    m_printer.print(src_consumed_not_earlier); cout << endl;
#endif
    auto reuse =
            (src_unmap(src_not_later) *
             src_unmap(src_consumed_not_earlier))
            .range();

#if DEBUG_BUFFER_SIZE == 1
    cout << ".. reuse:" << endl;
    m_printer.print(reuse); cout << endl;
#endif


    isl::local_space opt_space(reuse.get_space());
    int dim = source_actor.flow_dimension;
    assert(dim >= 0);

    auto x = expression::variable(opt_space, isl::space::variable, dim);
    auto y = expression::variable(opt_space, isl::space::variable,
                                  source_stmt->domain.size()+dim);
    auto cost = x - y;

    auto maximum = reuse.maximum(cost);

    cout << ".. Max delay = "; m_printer.print(maximum); cout << endl;

    // Store result

    assert(maximum.denominator() == 1);
    // "maximum" is index difference, so add 1
    int buf_size = maximum.numerator() + 1;
    if (source_stmt->buffer_size < buf_size)
        source_stmt->buffer_size = buf_size;
}

}
}
