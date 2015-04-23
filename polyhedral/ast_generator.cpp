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
#include <limits>
#include <cmath>
#include <cstdlib>

using namespace std;

namespace stream {

static int gcd(int a, int b)
{
    for (;;)
    {
        if (a == 0) return b;
        b %= a;
        if (b == 0) return a;
        a %= b;
    }
}

static int lcm(int a, int b)
{
    int div = gcd(a, b);
    return div ? (a / div * b) : 0;
}

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

    // FIXME: belongs somewhere else...
    for (statement * stmt : m_statements)
    {
        const dataflow::actor *actor = m_dataflow->find_actor_for(stmt);
        if ( actor && dynamic_cast<input_access*>(stmt->expr) )
        {
            stmt->iteration_domain = { polyhedral::infinite };
            stmt->data_to_iteration = mapping(stmt->domain.size(), 1);
            stmt->data_to_iteration.coefficient(actor->flow_dimension, 0) = 1;
        }
        else
        {
            stmt->iteration_domain = stmt->domain;
            stmt->data_to_iteration = mapping::identity(stmt->domain.size());
        }
    }
}

ast_generator::~ast_generator()
{
}

pair<clast_stmt*,clast_stmt*>
ast_generator::generate()
{
    if (debug::is_enabled())
        cout << endl << "### AST Generation ###" << endl;

    isl::union_set finite_domains(m_ctx);
    isl::union_set infinite_domains(m_ctx);
    isl::union_map data_iter_map(m_ctx);
    isl::union_map data_dependencies(m_ctx);
    polyhedral_model(finite_domains, infinite_domains, data_iter_map, data_dependencies);

    isl::union_map iter_dependencies = data_dependencies;
    iter_dependencies.map_domain_through(data_iter_map);

    // FIXME: belongs somewhere else...
    // Add additional constraints for infinite inputs:
    // each input iteration must be after the previous one.
    for (statement * stmt : m_statements)
    {
        const dataflow::actor *actor = m_dataflow->find_actor_for(stmt);
        if ( actor && dynamic_cast<input_access*>(stmt->expr) )
        {
            assert(stmt->iteration_domain.size() == 1);
            auto iter_space = isl::space( m_ctx,
                                          isl::set_tuple(isl::identifier(stmt->name, stmt), 1) );
            auto iter_dep_space = isl::space::from(iter_space, iter_space);
            auto dep = isl::basic_map::universe(iter_dep_space);
            isl::local_space cnstr_space(iter_dep_space);
            auto in = cnstr_space(isl::space::input, 0);
            auto out = cnstr_space(isl::space::output, 0);
            dep.add_constraint(out == in + 1);

            if (debug::is_enabled())
            {
                cout << "Input sequence constraint: ";
                m_printer.print(dep);
                cout << endl;
            }

            iter_dependencies = iter_dependencies | dep;
        }
    }


    auto finite_schedule =
            schedule_finite_domains(finite_domains, iter_dependencies);

    isl::union_map infinite_schedule(m_ctx);
    auto periodic_schedules =
            schedule_infinite_domains(infinite_domains, iter_dependencies,
                                      infinite_schedule);

    isl::union_map combined_schedule(m_ctx);
    combine_schedules(finite_schedule, infinite_schedule, combined_schedule);

    compute_buffer_sizes(combined_schedule, data_dependencies, data_iter_map);

    if(m_print_ast)
        cout << endl << "== Output AST ==" << endl;

    if(m_print_ast)
        cout << endl << "-- Finite --" << endl;
    struct clast_stmt *finite_ast
            = make_ast( finite_schedule );

    if(m_print_ast)
        cout << endl << "-- Init --" << endl;
    struct clast_stmt *init_ast
            = make_ast( periodic_schedules.first );

    // Join finite and infinite initialization schedules:
    {
        if (!finite_ast)
        {
            finite_ast = init_ast;
        }
        else
        {
            clast_stmt *s = finite_ast;
            while(s->next)
                s = s->next;
            s->next = init_ast;
        }
    }

    if(m_print_ast)
        cout << endl << "-- Period --" << endl;
    struct clast_stmt *period_ast
            = make_ast( periodic_schedules.second );

    //return pair<clast_stmt*,clast_stmt*>(nullptr, nullptr);
    return make_pair(finite_ast, period_ast);

#if 0
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
#endif
}

void ast_generator::polyhedral_model
(isl::union_set & finite_domains, isl::union_set &infinite_domains,
 isl::union_map & data_iter_map,
 isl::union_map & all_dependencies)
{    
    for (statement * stmt : m_statements)
    {
        auto domain = polyhedral_domain(stmt);
        if (m_dataflow->find_actor_for(stmt))
            infinite_domains = infinite_domains | domain.first;
        else
            finite_domains = finite_domains | domain.first;

        data_iter_map = data_iter_map | domain.second;
    }

    for (statement * stmt : m_statements)
    {
        auto dependency = polyhedral_dependencies(stmt);
        all_dependencies = all_dependencies | dependency;
    }
}

pair<isl::basic_set, isl::basic_map>
ast_generator::polyhedral_domain( statement *stmt )
{
    using isl::tuple;

    assert(stmt->iteration_domain.size());

    auto iter_space = isl::space( m_ctx,
                                  isl::set_tuple( isl::identifier(stmt->name, stmt),
                                                  stmt->iteration_domain.size() ) );
    auto iter_domain = isl::basic_set::universe(iter_space);
    {
        auto constraint_space = isl::local_space(iter_space);
        for (int dim = 0; dim < stmt->iteration_domain.size(); ++dim)
        {
            int extent = stmt->iteration_domain[dim];

            auto dim_var = isl::expression::variable(constraint_space, isl::space::variable, dim);

            if (extent >= 0)
            {
                auto lower_bound = dim_var >= 0;
                iter_domain.add_constraint(lower_bound);

                auto upper_bound = dim_var < extent;
                iter_domain.add_constraint(upper_bound);
            }
        }
    }

    auto data_space = isl::space( m_ctx,
                                  isl::set_tuple( isl::identifier(stmt->name, stmt),
                                                  stmt->domain.size() ) );

    // FIXME: constraint_matrix places output dim before input dim
    auto iter_data_space = isl::space::from(iter_space, data_space);

    auto equalities = constraint_matrix(stmt->data_to_iteration);
    auto inequalities = isl::matrix(m_ctx, 0, equalities.column_count());

    auto data_iter_map =
            isl::basic_map(iter_data_space, equalities, inequalities).inverse();

    if(debug::is_enabled())
    {
        cout << "Iteration domain: "; m_printer.print(iter_domain); cout << endl;
        cout << "Data to iteration map: "; m_printer.print(data_iter_map); cout << endl;
    }

    return make_pair(iter_domain, data_iter_map);
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
                                       dependent->iteration_domain.size());
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
        // FIXME: Required data to iteration mapping?
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

/*
  Notation:
    c_i = number of init iterations
    c_s = number of steady iterations
    df = flow dimension

  Create domain mappings:
    M_i = D[d1,d2,df,...] -> D[-1,d1,d2,df...] : 0 <= df < c_i
    M_s = D[d1,d2,df,...] -> D[d0',d1,d2,df'...] : df = d0' * c_s + df' + c_i
  Map domains:
    D_i = M_i(D)
    D_s = M_s(D)
  Map dependencies:
    Dep_i+s = (M_i U M_s)(Dep)
*/

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
(const isl::union_set & domains, const isl::union_map & dependencies)
{
    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    //isl_options_set_schedule_fuse(m_ctx.get(), ISL_SCHEDULE_FUSE_MAX);

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());

    constr = isl_schedule_constraints_set_validity(constr, dependencies.copy());
    constr = isl_schedule_constraints_set_proximity(constr, dependencies.copy());

    isl_schedule * sched =
            isl_schedule_constraints_compute_schedule(constr);
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
        print_schedule(init_schedule);
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
        print_schedule(steady_schedule);
        cout << endl;
    }

    return steady_schedule;
}

isl::union_map
ast_generator::schedule_finite_domains
(const isl::union_set & finite_domains, const isl::union_map & dependencies)
{
    auto schedule = make_schedule(finite_domains, dependencies).in_domain(finite_domains);
    cout << endl << "Finite schedule:" << endl;
    print_schedule(schedule);
    cout << endl;

    return schedule;
}

pair<isl::union_map, isl::union_map>
ast_generator::schedule_infinite_domains
(const isl::union_set & infinite_domains, const isl::union_map & dependencies,
 isl::union_map & infinite_sched)
{
    using namespace isl;

    if (infinite_domains.is_empty())
        return make_pair(isl::union_map(m_ctx), isl::union_map(m_ctx));

    infinite_sched = make_schedule(infinite_domains, dependencies).in_domain(infinite_domains);

    if (debug::is_enabled())
    {
        cout << "Infinite schedule:" << endl;
        print_schedule(infinite_sched);
        cout << endl;
    }

    if (infinite_sched.is_empty())
        throw error("Could not compute infinite schedule.");

    //auto infinite_sched_in_domain = infinite_sched.in_domain(infinite_domains);

    int flow_dim = -1;
    int n_dims = 0;

    int least_common_period = compute_period(infinite_sched, flow_dim, n_dims);

    int least_common_offset =
            common_offset(infinite_sched, flow_dim);

    auto period_range = set::universe(isl::space(m_ctx, set_tuple(n_dims)));
    {
        local_space cnstr_space(period_range.get_space());
        auto flow_var = cnstr_space(isl::space::variable, flow_dim);
        period_range.add_constraint(flow_var >= least_common_offset);
        period_range.add_constraint(flow_var < (least_common_offset + least_common_period));
    }

    auto period_sched_part = infinite_sched.in_range(period_range);

    isl::union_map period_sched(m_ctx);

    period_sched_part.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        if (!dynamic_cast<input_access*>(stmt->expr))
        {
            period_sched = period_sched | m;
            return true;
        }

        auto domain = m.domain();
        local_space domain_space(domain.get_space());
        auto i = domain_space(space::variable, 0);
        auto min_i = domain.minimum(i);

        auto translation = map::universe(space::from(domain.get_space(), domain.get_space()));
        local_space xl_space(translation.get_space());
        auto i0 = xl_space(space::input, 0);
        auto i1 = xl_space(space::output, 0);
        translation.add_constraint(i1 == i0 - min_i);

        m.map_domain_through(translation);

        period_sched = period_sched | m;

        stmt->buffer_period_offset = min_i.integer();

        return true;
    });

    if (debug::is_enabled())
    {
        cout << "Period schedule:" << endl;
        print_schedule(period_sched);
        cout << endl;
    }

    isl::union_map init_sched(m_ctx);

    infinite_sched.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        auto actor = m_dataflow->find_actor_for(stmt);
        assert(actor);

        local_space cnstr_space(m.get_space());

        auto stmt_flow_var = cnstr_space(isl::space::input, actor->flow_dimension);
        m.add_constraint(stmt_flow_var >= 0);

        auto sched_flow_var = cnstr_space(isl::space::output, flow_dim);
        m.add_constraint(sched_flow_var < least_common_offset);

        init_sched = init_sched | m;

        return true;
    });

    if (debug::is_enabled())
    {
        cout << "Init schedule:" << endl;
        print_schedule(init_sched);
        cout << endl;
    }

    return make_pair(init_sched, period_sched);
}

int ast_generator::compute_period
(const isl::union_map & schedule, int & flow_dim_out, int & n_dims_out)
{
    vector<pair<statement*,int>> ks;

    int sched_flow_dim = -1;

    schedule.for_each( [&] (const isl::map & m)
    {
        auto id = m.id(isl::space::input);
        statement *stmt = statement_for(id);
        auto actor_ptr = m_dataflow->find_actor_for(stmt);
        assert(actor_ptr);
        int flow_dim = actor_ptr->flow_dimension;

        m.for_each( [&] (const isl::basic_map & bm)
        {
            // Find first output dimension which iterates
            // the infinite input dimension.

            auto space = bm.get_space();
            int in_dims = space.dimension(isl::space::input);
            int out_dims = space.dimension(isl::space::output);

            n_dims_out = out_dims;

            auto eq = bm.equalities_matrix();
            int rows = eq.row_count();

            int first_out_dim = out_dims;
            int first_flow_k = 0;
            int first_flow_c = 0;

            for (int r = 0; r < rows; ++r)
            {
                int flow_k = eq(r, flow_dim).value().integer();
                if (flow_k)
                {
                    for (int out = 0; out < out_dims; ++out)
                    {
                        int out_k = eq(r, out + in_dims).value().integer();
                        if (out_k)
                        {
                            if (out < first_out_dim)
                            {
                                first_out_dim = out;
                                first_flow_k = std::abs(flow_k);
                                first_flow_c = eq(r, in_dims + out_dims).value().integer();
                                if (out_k > 0)
                                    first_flow_c *= -1;
                            }
                            break;
                        }
                    }
                }
            }

            ks.push_back(make_pair(stmt,first_flow_k));

            //cout << id.name << " flow coef @ " << first_out_dim << " = " << first_flow_k << endl;

            if (sched_flow_dim == -1)
                sched_flow_dim = first_out_dim;
            else if (sched_flow_dim != first_out_dim)
                cerr << "WARNING: different schedule flow dimensions" << endl;

            return true;
        });
        return true;
    });

    flow_dim_out = sched_flow_dim;

    int least_common_period = 1;
    for (const auto & k : ks)
    {
        int period = k.second;
        least_common_period = lcm(least_common_period, period);
    }

    // Compute span of period in statement flow dimension.
    // This equals the offset in buffer indexes added at each period.
    for (const auto & k : ks)
    {
        statement *stmt = k.first;
        int period = k.second;
        int span = least_common_period / period;
        //cout << "Period advances " << stmt->name << " by " << span << endl;
        if (!stmt->buffer_period)
            stmt->buffer_period = span;
        if (stmt->buffer_period != span)
            cerr << "WARNING: different period counts for the same statement!" << endl;
    }

    return least_common_period;
}

int ast_generator::common_offset(isl::union_map & schedule, int flow_dim)
{
    using namespace isl;

    // Assumption: schedule is bounded to iteration domains.

    int common_offset = std::numeric_limits<int>::min();

    schedule.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        auto actor = m_dataflow->find_actor_for(stmt);
        if (!actor)
            return true;

        auto space = m.get_space();
        int in_dims = space.dimension(space::input);

        // add constraint: iter[flow] < 0
        local_space cnstr_space(space);
        auto dim0_idx = cnstr_space(space::input, actor->flow_dimension);
        m.add_constraint(dim0_idx < 0);
        assert(!m.is_empty());

        set s(m.wrapped());
        auto flow_idx = s.get_space()(space::variable, in_dims + flow_dim);
        int max_flow_idx = s.maximum(flow_idx).integer();
        int offset = max_flow_idx + 1;

        if (debug::is_enabled())
        {
            //cout << id.name << " offset = " << offset << endl;
        }

        common_offset = std::max(common_offset, offset);

        return true;
    });

    return common_offset;
}

void
ast_generator::combine_schedules
(const isl::union_map & finite_schedule,
 const isl::union_map & infinite_schedule,
 isl::union_map & combined_schedule)
{
    int finite_sched_dim = 0, infinite_sched_dim = 0;

    finite_schedule.for_each( [&]( isl::map & sched )
    {
        finite_sched_dim = sched.get_space().dimension(isl::space::output);
        return false;
    });
    infinite_schedule.for_each( [&]( isl::map & sched )
    {
        infinite_sched_dim = sched.get_space().dimension(isl::space::output);
        return false;
    });

    //isl::union_map finite_schedule_part(m_ctx);

    finite_schedule.for_each( [&]( isl::map & sched )
    {
        {
            // Add first dimension for period, equal to -1
            sched.insert_dimensions(isl::space::output, 0, 1);
            auto d0 = sched.get_space()(isl::space::output, 0);
            sched.add_constraint(d0 == -1);
        }

        if (infinite_sched_dim > finite_sched_dim)
        {
            // Add dimensions to match period schedule, equal to 0
            int location = finite_sched_dim+1;
            int count = infinite_sched_dim - finite_sched_dim;
            sched.insert_dimensions(isl::space::output, location, count);
            isl::local_space space( sched.get_space() );
            for (int dim = location; dim < location+count; ++dim)
            {
                auto d = space(isl::space::output, dim);
                sched.add_constraint(d == 0);
            }
        }

        //finite_schedule_part = finite_schedule_part | sched;
        combined_schedule = combined_schedule | sched;

        return true;
    });

    //isl::union_map infinite_schedule_part(m_ctx);

    infinite_schedule.for_each( [&]( isl::map & sched )
    {
        {
            // Lower-bound domain flow dim to 0;
            auto id = sched.id(isl::space::input);
            auto stmt = statement_for(id);
            auto actor = m_dataflow->find_actor_for(stmt);
            assert(actor);

            isl::local_space space( sched.get_space() );
            auto in_flow = space(isl::space::input, actor->flow_dimension);
            sched.add_constraint(in_flow >= 0);
        }

        {
            // Add first dimension, equal to 0;
            sched.insert_dimensions(isl::space::output, 0, 1);
            isl::local_space space( sched.get_space() );
            auto d0 = space(isl::space::output, 0);
            sched.add_constraint(d0 == 0);
        }

        if (finite_sched_dim > infinite_sched_dim)
        {
            // Add dimensions to match init schedule, equal to 0
            int location = infinite_sched_dim+1;
            int count = finite_sched_dim - infinite_sched_dim;
            sched.insert_dimensions(isl::space::output, location, count);
            isl::local_space space( sched.get_space() );
            for (int dim = location; dim < location + count; ++dim)
            {
                auto d = space(isl::space::output, dim);
                sched.add_constraint(d == 0);
            }
        }

        //infinite_schedule_part = infinite_schedule_part | sched;
        combined_schedule = combined_schedule | sched;

        return true;
    });

    if (debug::is_enabled())
    {
        cout << endl << "Combined schedule:" << endl;
        print_schedule(combined_schedule);
        cout << endl;
    }
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const isl::union_map & data_dependencies,
                                          const isl::union_map & data_iter_map )
{
    using namespace isl;

    isl::space *time_space = nullptr;

    schedule.for_each([&time_space]( const map & m )
    {
        time_space = new space( m.range().get_space() );
        return false;
    });

    for (statement *stmt : m_statements)
    {
        compute_buffer_size(schedule, data_dependencies, data_iter_map,
                            stmt, *time_space);
    }

    delete time_space;

    for (statement *stmt : m_statements)
    {
        if (stmt->buffer.empty())
        {
            const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
            for (int dim = 0; dim < stmt->domain.size(); ++dim)
            {
                if (actor && dim == actor->flow_dimension)
                    // FIXME!
                    stmt->buffer.push_back(std::max(actor->init_count, actor->steady_count));
                else
                    stmt->buffer.push_back(stmt->domain[dim]);
                assert(stmt->buffer.back() >= 0);
            }
        }
    }
}

void ast_generator::compute_buffer_size
( const isl::union_map & schedule,
  const isl::union_map & data_dependencies,
  const isl::union_map & data_iter_map,
  statement *stmt,
  const isl::space & time_space )
{
    if (debug_buffer_size::is_enabled())
    {
        cout << "Buffer size for statement: " << stmt->name << endl;
    }

    using namespace isl;
    using isl::expression;


    auto src_iter_space = isl::space( m_ctx,
                                      isl::set_tuple( isl::identifier(stmt->name, stmt),
                                                 stmt->iteration_domain.size() ) );
    auto src_data_space = isl::space( m_ctx,
                                      isl::set_tuple( isl::identifier(stmt->name, stmt),
                                                      stmt->domain.size() ) );
    auto src_data_universe = isl::basic_set::universe(src_data_space);

    auto src_data_deps = data_dependencies.in_domain(src_data_universe);

    if (src_data_deps.is_empty())
        return;

    // Extract schedule

    space src_sched_space = space::from(src_iter_space, time_space);
    map src_sched = schedule.map_for(src_sched_space);

    // Do the work

    map not_later = order_greater_than_or_equal(time_space);
    map later = order_less_than(time_space);

    // Find all source instances scheduled before t, for all t.
    // => Create map: t -> src
    //    such that: time(src) <= t

    map src_not_later = src_sched.inverse()( not_later );
    {
        auto src_iter_data_map =
                data_iter_map.map_for(space::from(src_data_space, src_iter_space)).inverse();
        src_not_later.map_range_through(src_iter_data_map);
    }

    // Find all instances of source consumed after time t, for each t;
    // => Create map: t -> src
    //    such that: time(sink(src)) <= t, for all sink

    map src_consumed_later(space::from(time_space, src_data_space));

    src_data_deps.for_each( [&]( const map & dep ){

        space sink_space = dep.range().get_space();
        space sink_sched_space = space::from(sink_space, time_space);
        map sink_sched = schedule.map_for(sink_sched_space);
        map sink_later = sink_sched.inverse()( later );
        map src_consumed_by_sink_later =
                dep.inverse()( sink_later ); // FIXME? .in_range(src_sched.domain());

        src_consumed_later = src_consumed_later | src_consumed_by_sink_later;

        return true;
    });

    // Find all src instances live at the same time.
    // = Create map: t -> src,
    //   Such that: time(src) <= t and time(sink(src)) <= t, for all sink
    auto buffered = src_not_later & src_consumed_later;

    if (debug_buffer_size::is_enabled())
    {
        cout << ".. Buffered: " << endl;
        m_printer.print(buffered);
        cout << endl;
    }

    vector<int> buffer_size;

    {
        auto buffered_reflection = (buffered * buffered);

        if (debug_buffer_size::is_enabled())
        {
            cout << ".. Buffer reflection: " << endl;
            m_printer.print(buffered_reflection); cout << endl;
        }

        isl::local_space space(buffered_reflection.get_space());
        int buf_dim_count = stmt->domain.size();
        int time_dim_count = time_space.dimension(isl::space::variable);
        buffer_size.reserve(buf_dim_count);

        if (debug_buffer_size::is_enabled())
            cout << ".. Max reuse distance:" << endl;

        for (int dim = 0; dim < buf_dim_count; ++dim)
        {
            {
                auto buffered_set = buffered_reflection.wrapped();
                isl::local_space space(buffered_set.get_space());
                auto a = space(isl::space::variable, time_dim_count + dim);
                auto b = space(isl::space::variable, time_dim_count + buf_dim_count + dim);
                auto max_distance = buffered_reflection.wrapped().maximum(b - a).integer();
                if (debug_buffer_size::is_enabled())
                    cout << max_distance << " ";
                buffer_size.push_back((int) max_distance + 1);
            }
            {
                isl::local_space space(buffered_reflection.get_space());
                auto a = space(isl::space::output, dim);
                auto b = space(isl::space::output, buf_dim_count + dim);
                buffered_reflection.add_constraint(a == b);
            }
        }

        if (debug_buffer_size::is_enabled())
            cout << endl;
    }

    stmt->buffer = buffer_size;

    // TODO: Clear source_stmt->inter_period_dependency as appropriate.

#if 0
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
#endif
}

void ast_generator::print_schedule( const isl::union_map & sched )
{
    sched.for_each ( [&](const isl::map & m){
       m_printer.print(m);
       cout << endl;
       return true;
    });
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
