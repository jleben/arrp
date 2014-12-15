/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

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
    m_print_ast(false),
    m_printer(m_ctx),
    m_statements(statements),
    m_dataflow(dataflow)
{
    m_ctx.set_error_action(isl::context::abort_on_error);
}

ast_generator::~ast_generator()
{
}

pair<clast_stmt*,clast_stmt*>
ast_generator::generate()
{
    if (debug::is_enabled())
        cout << endl << "### AST Generation ###" << endl;

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

    isl::union_set period_domains(m_ctx);
    steady_domains.for_each( [&](isl::set & domain)
    {
        auto period = domain.get_space()(isl::space::variable, 0);
        domain.add_constraint(period == 0);
        period_domains = period_domains | domain;
        return true;
    });

    auto init_schedule = make_init_schedule(init_domains, periodic_dependencies);
    auto period_schedule = make_steady_schedule(period_domains, periodic_dependencies);

    auto combined_schedule =
            combine_schedule(init_domains, steady_domains,
                             init_schedule, period_schedule);

    compute_buffer_sizes(combined_schedule, periodic_dependencies, domain_map);

    if(m_print_ast)
        cout << endl << "== Output AST ==" << endl;

    if(m_print_ast)
        cout << endl << "-- Finite --" << endl;
    struct clast_stmt *init_ast
            = make_ast( init_schedule.in_domain(init_domains) );

    if(m_print_ast)
        cout << endl << "-- Periodic --" << endl;
    struct clast_stmt *period_ast
            = make_ast( period_schedule.in_domain(period_domains) );

    return std::make_pair(init_ast, period_ast);
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

    if(debug::is_enabled())
    {
        cout << "Iteration domain: "; m_printer.print(domain); cout << endl;
    }

    return domain;
}

isl::union_map ast_generator::polyhedral_dependencies( statement * dependent )
{
    using isl::tuple;

    // We assume that a statement only writes one scalar value at a time.
    // Therefore, a dependency between two statements is exactly
    // the polyhedral::stmt_access::pattern in the model.

    isl::union_map all_dependencies_map(m_ctx);

    vector<stmt_access*> stmt_accesses;
    dependent->expr->find<stmt_access>(stmt_accesses);

    for (auto access : stmt_accesses)
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

        if (debug::is_enabled())
        {
            cout << "Dependency: ";
            m_printer.print(dependency_map); cout << endl;
        }

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

            if (debug::is_enabled())
            {
                cout << "Reduction initializer dependency: ";
                m_printer.print(dependency_map); cout << endl;
            }
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

            if (debug::is_enabled())
            {
                cout << "Reduction self-dependency: ";
                m_printer.print(dependency_map); cout << endl;
            }
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

    if (debug::is_enabled())
        cout << endl;

    init_domains = init_domain_maps(domains);
    steady_domains = steady_domain_maps(domains);

    if (debug::is_enabled())
    {
        cout << "Init domains:" << endl;
        m_printer.print(init_domains); cout << endl;
        cout << "Steady domains:" << endl;
        m_printer.print(steady_domains); cout << endl;
    }

    domain_map = init_domain_maps | steady_domain_maps;

    periodic_dependencies = dependencies;
    periodic_dependencies.map_domain_through(domain_map);
    periodic_dependencies.map_range_through(domain_map);

    if (debug::is_enabled())
    {
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
}


isl::union_map ast_generator::make_schedule
(isl::union_set & domains, isl::union_map & dependencies)
{
    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    //isl_options_set_schedule_fuse(m_ctx.get(), ISL_SCHEDULE_FUSE_MAX);

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

    if (debug::is_enabled())
    {
        cout << endl << "Init schedule:" << endl;
        //m_printer.print(init_schedule.in_domain(init_domains));
        m_printer.print(init_schedule);
        cout << endl;
    }

    return init_schedule;
}

isl::union_map
ast_generator::make_steady_schedule(isl::union_set & period_domains,
                                    isl::union_map & dependencies)
{
    isl::union_map steady_schedule = make_schedule(period_domains, dependencies);

    if (debug::is_enabled())
    {
        cout << endl << "Steady schedule:" << endl;
        //m_printer.print(steady_schedule.in_domain(steady_domains));
        m_printer.print(steady_schedule);
        cout << endl;
    }

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

    int init_sched_dim = 0, period_sched_dim = 0;

    init_schedule.for_each( [&]( isl::map & sched )
    {
        init_sched_dim = sched.get_space().dimension(isl::space::output);
        return false;
    });
    period_schedule.for_each( [&]( isl::map & sched )
    {
        period_sched_dim = sched.get_space().dimension(isl::space::output);
        return false;
    });

    init_schedule.for_each( [&]( isl::map & sched )
    {
        {
            // Add first dimension for period, equal to -1
            sched.insert_dimensions(isl::space::output, 0, 1);
            auto t0 = sched.get_space()(isl::space::output, 0);
            sched.add_constraint(t0 == -1);
        }

        if (period_sched_dim > init_sched_dim)
        {
            // Add dimensions to match period schedule, equal to 0
            int location = init_sched_dim+1;
            int count = period_sched_dim - init_sched_dim;
            sched.insert_dimensions(isl::space::output, location, count);
            isl::local_space space( sched.get_space() );
            for (int dim = location; dim < location+count; ++dim)
            {
                auto t = space(isl::space::output, dim);
                sched.add_constraint(t == 0);
            }
        }

        canonical_init_schedule = canonical_init_schedule | sched;

        return true;
    });

    isl::union_map canonical_steady_schedule(m_ctx);

    period_schedule.for_each( [&]( isl::map & sched )
    {
        {
            // Add first dimension for period, equal to period index
            sched.insert_dimensions(isl::space::output, 0, 1);
            isl::local_space space( sched.get_space() );
            auto p = space(isl::space::input, 0);
            auto t0 = space(isl::space::output, 0);
            sched.add_constraint(p == t0);
        }

        if (init_sched_dim > period_sched_dim)
        {
            // Add dimensions to match init schedule, equal to 0
            int location = period_sched_dim+1;
            int count = init_sched_dim - period_sched_dim;
            sched.insert_dimensions(isl::space::output, location, count);
            isl::local_space space( sched.get_space() );
            for (int dim = location; dim < location + count; ++dim)
            {
                auto t = space(isl::space::output, dim);
                sched.add_constraint(t == 0);
            }
        }

        canonical_steady_schedule = canonical_steady_schedule | sched;

        return true;
    });

    isl::union_map combined_schedule =
            canonical_init_schedule.in_domain(init_domains) |
            canonical_steady_schedule.in_domain(steady_domains);

    if (debug::is_enabled())
    {
        cout << endl << "Combined schedule:" << endl;
        m_printer.print(combined_schedule);
        cout << endl;
    }

    return combined_schedule;
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const isl::union_map & dependencies,
                                          const isl::union_map & domain_maps )
{
    using namespace isl;

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

    delete time_space;

    if (debug_buffer_size::is_enabled())
        cout << endl << "Buffer sizes:" << endl;

    for (statement *stmt : m_statements)
    {
        if (stmt->buffer.empty())
        {
            const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
            for (int dim = 0; dim < stmt->domain.size(); ++dim)
            {
                if (actor && dim == actor->flow_dimension)
                    stmt->buffer.push_back(std::max(actor->init_count, actor->steady_count));
                else
                    stmt->buffer.push_back(stmt->domain[dim]);
                assert(stmt->buffer.back() >= 0);
            }
        }

        if (debug_buffer_size::is_enabled())
        {
            int flat_size = 1;
            cout << stmt->name << ": ";
            for (auto b : stmt->buffer)
            {
                cout << b << " ";
                flat_size *= b;
            }
            cout << "[" << flat_size << "]";
            cout << endl;
        }
    }
}

void ast_generator::compute_buffer_size( const isl::union_map & schedule,
                                         const isl::union_map & domain_unmap,
                                         const isl::map & dependency,
                                         const isl::space & time_space )
{
    if (debug_buffer_size::is_enabled())
    {
        cout << "Buffer size for dependency: ";
        m_printer.print(dependency); cout << endl;
    }

    using namespace isl;
    using isl::expression;

    // Get info

    space src_space = dependency.domain().get_space();
    space sink_space = dependency.range().get_space();

    statement *source_stmt = statement_for(src_space.id(isl::space::variable));

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
    map later = order_less_than(time_space);

    map src_not_later = src_sched.inverse()( not_later );
    map sink_later = sink_sched.inverse()( later );
    map src_consumed_later =
            dependency.inverse()( sink_later ).in_range(src_sched.domain());

    if (debug_buffer_size::is_enabled())
    {
        cout << ".. produced:" << endl;
        m_printer.print(src_not_later); cout << endl;
        m_printer.print(sink_later); cout << endl;
        cout << ".. consumed:" << endl;
        m_printer.print(src_consumed_later); cout << endl;
    }

    auto buffered = src_unmap(src_not_later) & src_unmap(src_consumed_later);

    vector<long> buffer_size;

    {
        auto buffered_reflection = (buffered * buffered).wrapped();

        if (debug_buffer_size::is_enabled())
        {
            cout << ".. buffer reflection: " << endl;
            m_printer.print(buffered_reflection); cout << endl;
        }

        isl::local_space space(buffered_reflection.get_space());
        int buf_dim_count = source_stmt->domain.size();
        int time_dim_count = time_space.dimension(isl::space::variable);
        buffer_size.reserve(buf_dim_count);

        if (debug_buffer_size::is_enabled())
            cout << ".. Max reuse distance:" << endl;

        for (int dim = 0; dim < buf_dim_count; ++dim)
        {
            auto a = space(isl::space::variable, time_dim_count + dim);
            auto b = space(isl::space::variable, time_dim_count + buf_dim_count + dim);
            auto distance = b - a;
            auto max_distance = buffered_reflection.maximum(distance).integer();
            if (debug_buffer_size::is_enabled())
                cout << max_distance << " ";
            buffer_size.push_back(max_distance + 1);
        }

        if (debug_buffer_size::is_enabled())
            cout << endl;
    }

    auto & max_buffer_size = source_stmt->buffer;
    for (unsigned int dim = 0; dim < source_stmt->domain.size(); ++dim)
    {
        if (dim >= max_buffer_size.size())
            max_buffer_size.push_back(buffer_size[dim]);
        else if (buffer_size[dim] > max_buffer_size[dim])
            max_buffer_size[dim] = buffer_size[dim];
    }

    // Compute largest re-use distance in number of periods

    {
        int time_dim_count = time_space.dimension(isl::space::variable);
        map time_dep = dependency;
        time_dep.map_domain_through(src_sched);
        time_dep.map_range_through(sink_sched);
        auto time_dep_set = time_dep.wrapped();
        auto expr_space = isl::local_space(time_dep_set.get_space());
        auto src_period = expr_space(isl::space::variable, 0);
        auto sink_period = expr_space(isl::space::variable, time_dim_count);
        auto distance = sink_period - src_period;
        auto max_distance = time_dep_set.maximum(distance).integer();
        if (debug_buffer_size::is_enabled())
            cout << ".. max period distance = " << max_distance << endl;
        assert(max_distance >= 0);
        source_stmt->inter_period_dependency = max_distance > 0;
    }
}

struct clast_stmt *ast_generator::make_ast( const isl::union_map & isl_schedule )
{
    if (isl_schedule.is_empty())
        return nullptr;

    CloogState *state = cloog_state_malloc();
    CloogOptions *options = cloog_options_malloc(state);
    CloogUnionDomain *schedule =
            cloog_union_domain_from_isl_union_map(
                isl_schedule.copy());
    //CloogMatrix *dummy_matrix = cloog_matrix_alloc(0,0);

    CloogDomain *context_domain =
            cloog_domain_from_isl_set(
                isl_set_universe(isl_schedule.get_space().copy()));
            //cloog_domain_from_cloog_matrix(state, dummy_matrix, 0);
    CloogInput *input =  cloog_input_alloc(context_domain, schedule);

    //cout << "--- Cloog input:" << endl;
    //cloog_input_dump_cloog(stdout, input, options);
    //cout << "--- End Cloog input ---" << endl;

    assert(input);

    clast_stmt *ast = cloog_clast_create_from_input(input, options);

    if(m_print_ast)
        clast_pprint(stdout, ast, 0, options);

    return ast;
}

}
}
