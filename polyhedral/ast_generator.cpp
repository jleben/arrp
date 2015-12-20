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
#include <isl/schedule_node.h>

#include <isl-cpp/space.hpp>
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

ast_generator::ast_generator( const model & m ):
    m_print_ast(false),
    m_printer(m.context),
    m_model(m)
{}

ast_generator::~ast_generator()
{
}

pair<clast_stmt*,clast_stmt*>
ast_generator::generate()
{
    if (debug::is_enabled())
        cout << endl << "### AST Generation ###" << endl;

    data d(m_model.context);
    prepare_data(d);

#if 1
    if (debug::is_enabled())
    {
        cout << "Domains: " << endl;
        print_each_in(d.finite_domains);
        print_each_in(d.infinite_domains);
        cout << "Write relations:" << endl;
        print_each_in(d.write_relations);
        cout << "Read relations:" << endl;
        print_each_in(d.read_relations);
        cout << "Dependencies:" << endl;
        print_each_in(d.dependencies);
    }
#endif

    auto all_domains = d.finite_domains | d.infinite_domains;

#if 0
    if (debug::is_enabled())
    {
        cout << "All domains:" << endl;
        m_printer.print(all_domains); cout << endl;

        cout << "All deps:" << endl;
        m_printer.print(d.dependencies); cout << endl;
    }
#endif

    d.schedule = make_schedule(all_domains, d.dependencies).in_domain(all_domains);

    make_periodic_schedule(d.schedule);

#if 0
    d.finite_schedule =
            schedule_finite_domains(d.finite_domains, d.dependencies);

    auto periodic_schedules =
            schedule_infinite_domains(d.infinite_domains, d.dependencies,
                                      d.infinite_schedule);

#endif
    return pair<clast_stmt*,clast_stmt*>(nullptr, nullptr);
#if 0
    isl::union_map combined_schedule(m_model.context);
    combine_schedules(d.finite_schedule, d.infinite_schedule, combined_schedule);

    compute_buffer_sizes(combined_schedule, d);

    find_inter_period_deps(periodic_schedules.second, d);

    if(m_print_ast)
        cout << endl << "== Output AST ==" << endl;

    if(m_print_ast)
        cout << endl << "-- Finite --" << endl;
    struct clast_stmt *finite_ast
            = make_ast( d.finite_schedule );

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

    return make_pair(finite_ast, period_ast);
#endif
}

void ast_generator::prepare_data(data & d)
{

    for (const auto & stmt : m_model.statements)
    {
        {
            auto time_var = stmt->domain.get_space()(isl::space::variable, 0);
            try {
                auto result = stmt->domain.maximum(time_var);
                if (result.is_infinity())
                {
                    stmt->is_infinite = true;
                    d.infinite_domains = d.infinite_domains | stmt->domain;
                    cout << "Infinite domain: " << endl;
                    m_printer.print(stmt->domain); cout << endl;
                }
                else
                {
                    stmt->is_infinite = false;
                    d.finite_domains = d.finite_domains | stmt->domain;
                    cout << "Finite domain: " << endl;
                    m_printer.print(stmt->domain); cout << endl;
                }
            } catch (isl::error &) {
                throw error("Could not check whether statement domain is infinite.");
            }
        }
#if 1
        d.write_relations = d.write_relations |
                (isl::map(stmt->write_relation).in_domain(stmt->domain));

        d.read_relations = d.read_relations |
                (stmt->read_relations.in_domain(stmt->domain));
#else
        d.write_relations = d.write_relations |
                (stmt->write_relation);

        d.read_relations = d.read_relations |
                (stmt->read_relations);
#endif
    }

    d.dependencies = d.read_relations.inverse()(d.write_relations);
}

#if 0
void ast_generator::polyhedral_model(data & d)
{
    for (const auto & stmt : m_model.statements)
    {
        polyhedral_model(stmt, d);
    }

    auto deps = d.write_relations;
    deps.map_range_through(d.read_relations.inverse());
    d.dependencies = deps;

    // FIXME: belongs somewhere else...
    // Add additional constraints for infinite inputs and outputs:
    // each input iteration must be after the previous one.
    for (const auto & stmt : m_model.statements)
    {
        if ( stmt->flow_dim >= 0 &&
             (dynamic_cast<input_access*>(stmt->expr.get()) || !stmt->array) )
        {
            assert(stmt->domain.size() == 1);
            auto iter_space =
                    isl::space( m_model.context,
                                isl::set_tuple(isl::identifier(stmt->name, stmt.get()), 1) );
            auto iter_dep_space = isl::space::from(iter_space, iter_space);
            auto dep = isl::basic_map::universe(iter_dep_space);
            isl::local_space cnstr_space(iter_dep_space);
            auto in = cnstr_space(isl::space::input, 0);
            auto out = cnstr_space(isl::space::output, 0);
            dep.add_constraint(out == in + 1);

            d.dependencies = d.dependencies | dep;
        }
    }
}

void ast_generator::polyhedral_model(statement_ptr stmt, data & d)
{
    using namespace isl;
    using isl::tuple;

    // FIXME: Dirty hack
    if ( dynamic_cast<input_access*>(stmt->expr.get()) &&
         stmt->flow_dim >= 0 )
    {
        stmt->domain = { infinite };
        stmt->flow_dim = 0;
    }

    auto stmt_tuple = isl::set_tuple( isl::identifier(stmt->name, stmt.get()),
                                      stmt->domain.size() );
    auto iter_space = isl::space( m_model.context, stmt_tuple );
    auto iter_domain = isl::basic_set::universe(iter_space);
    {
        auto constraint_space = isl::local_space(iter_space);
        for (int dim = 0; dim < stmt->domain.size(); ++dim)
        {
            int extent = stmt->domain[dim];

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

    if (stmt->flow_dim >= 0)
        d.infinite_domains = d.infinite_domains | iter_domain;
    else
        d.finite_domains = d.finite_domains | iter_domain;

    // Write relation

    if (stmt->array)
    {
        auto a = stmt->array;
        auto array_tuple = isl::output_tuple( isl::identifier(a->name),
                                              a->size.size() );
        isl::input_tuple stmt_in_tuple;
        stmt_in_tuple.id = stmt_tuple.id;
        stmt_in_tuple.elements = stmt_tuple.elements;

        isl::space space(m_model.context, stmt_in_tuple, array_tuple);

        // FIXME: Dirty hack
        if ( dynamic_cast<input_access*>(stmt->expr.get()) &&
             stmt->flow_dim >= 0 )
        {
            isl::local_space cnstr_space(space);
            auto stmt_flow = cnstr_space(space::input, stmt->flow_dim);
            auto array_flow = cnstr_space(space::output, a->flow_dim);

            auto relation = isl::basic_map::universe(space);
            relation.add_constraint(stmt_flow == array_flow);

            d.write_relations = d.write_relations | relation;
        }
        else
        {
            auto equalities = constraint_matrix(stmt->write_relation);
            auto inequalities = isl::matrix(m_model.context, 0, equalities.column_count());
            auto relation = isl::basic_map(space, equalities, inequalities);

            d.write_relations = d.write_relations | relation;
        }
    }

    // Read relations

    {
        vector<array_access*> array_accesses;
        stmt->expr->find<array_access>(array_accesses);

        for (auto access : array_accesses)
        {
            auto a = access->target;
            auto array_tuple = isl::output_tuple( isl::identifier(a->name),
                                                  a->size.size() );
            isl::input_tuple stmt_in_tuple;
            stmt_in_tuple.id = stmt_tuple.id;
            stmt_in_tuple.elements = stmt_tuple.elements;

            isl::space space(m_model.context, stmt_in_tuple, array_tuple);

            // FIXME: Dirty hack
            if ( !stmt->array )
            {
                isl::local_space cnstr_space(space);
                auto stmt_flow = cnstr_space(space::input, stmt->flow_dim);
                auto array_flow = cnstr_space(space::output, a->flow_dim);

                auto relation = isl::basic_map::universe(space);
                relation.add_constraint(stmt_flow == array_flow);

                d.read_relations = d.read_relations | relation;
            }
            else
            {
                auto equalities = constraint_matrix(access->pattern);
                auto inequalities = isl::matrix(m_model.context, 0, equalities.column_count());
                auto relation = isl::basic_map(space, equalities, inequalities);

                d.read_relations = d.read_relations | relation;
            }
        }
    }
}

isl::matrix ast_generator::constraint_matrix( const mapping & map )
{
    // one constraint for each output dimension
    int rows = map.output_dimension();
    // output dim + input dim + a constant
    int cols = map.output_dimension() + map.input_dimension() + 1;

    isl::matrix matrix(m_model.context, rows, cols);

    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            matrix(r,c) = 0;

    for (int out_dim = 0; out_dim < map.output_dimension(); ++out_dim)
    {
        int out_col = out_dim + map.input_dimension();

        // Put output index on the other side of the equality (negate):
        matrix(out_dim, out_col) = -1;

        for (int in_dim = 0; in_dim < map.input_dimension(); ++in_dim)
        {
            int in_col = in_dim;
            int coef = map.coefficients(out_dim, in_dim);
            matrix(out_dim, in_col) = coef;
        }

        int offset = map.constants[out_dim];
        matrix(out_dim, cols-1) = offset;
    }

    return matrix;
}
#endif

isl_bool print_schedule_node_info(isl_schedule_node *node, void *user)
{
    auto type = isl_schedule_node_get_type(node);
    if (type == isl_schedule_node_band)
    {
        cout << "band is permutable = "
             << isl_schedule_node_band_get_permutable(node) << endl;
    }
    else
    {
        cout << "not a band" << endl;
    }

    return isl_bool_true;
}


isl_bool
ast_generator::add_schedule_constraints
(struct isl_scheduler * sched)
{
    int sched_dim = isl_scheduler_get_current_dim(sched);
    if (sched_dim != 0)
        return isl_bool_false;

    {
        isl_schedule_node * node = isl_scheduler_get_current_node(sched);
        while(node)
        {
            auto type = isl_schedule_node_get_type(node);
            if (type == isl_schedule_node_sequence)
            {
                cout << "The schedule has a sequence node before a band node." << endl;
                isl_schedule_node_free(node);
                return isl_bool_error;
            }
            if (!isl_schedule_node_has_parent(node))
                break;
            node = isl_schedule_node_parent(node);
        }
    }

    isl::basic_set coefs(isl_scheduler_get_user_coefs(sched));
    auto cnstr_space = isl::local_space(coefs.get_space());

    bool new_constraints = false;

    for (auto & stmt : m_model.statements)
    {
        if (!stmt->is_infinite)
            continue;

        int coef_pos = isl_scheduler_get_user_coef_pos
                (sched, stmt->domain.get_space().get());
        if (coef_pos < 0)
        {
            cout << "Warning: coefficients not found for statement "
                 << stmt->name << endl;
        }

        int n_params = stmt->domain.get_space().dimension(isl::space::parameter);
        int var_coef_pos = coef_pos + n_params;
        auto coef = cnstr_space(isl::space::variable, var_coef_pos);
        coefs.add_constraint(coef > 0);
        new_constraints = true;
    }

    if (!new_constraints)
        return isl_bool_false;

    isl_scheduler_set_user_coefs(sched, coefs.copy());
    return isl_bool_true;
}

isl::union_map ast_generator::make_schedule
(const isl::union_set & domains, const isl::union_map & dependencies)
{
    // Collect sensible proximity dependencies.
    // Drop those which can not possible be satisfied
    // = a source has infinite number of sinks

    isl::union_map proximity_deps(dependencies.ctx());

    dependencies.for_each([&](const isl::map & m)
    {
        m.for_each([&](const isl::basic_map & dep)
        {
            int source_dim = m.get_space().dimension(isl::space::input);
            int sink_dim = m.get_space().dimension(isl::space::output);

            auto sink_pairs = (dep * dep).wrapped();

            isl::local_space space(sink_pairs.get_space());
            auto sink_a = space(isl::space::variable, source_dim);
            auto sink_b = space(isl::space::variable, source_dim + sink_dim);
            auto max_sink_distance = sink_pairs.maximum(sink_b - sink_a);

            if (!max_sink_distance.is_infinity())
            {
                if (debug::is_enabled()) {
                    cout << "Accepting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }

                proximity_deps = proximity_deps | dep;
            }
            else
            {
                if (debug::is_enabled()) {
                    cout << "Rejecting proximity dependence:";
                    m_printer.print(dep); cout << endl;
                }
            }

            return true;
        });
        return true;
    });

    // FIXME: statements with no dependencies
    // seem to always end up with an empty schedule.

    isl_schedule_constraints *constr =
            isl_schedule_constraints_on_domain(domains.copy());

    constr = isl_schedule_constraints_set_constraint_filter
            (constr, &add_schedule_constraints_helper, this);

    constr = isl_schedule_constraints_set_validity(constr, dependencies.copy());
    constr = isl_schedule_constraints_set_proximity(constr, proximity_deps.copy());

    isl_schedule * sched =
            isl_schedule_constraints_compute_schedule(constr);
    assert(sched);

    isl::union_map sched_map( isl_schedule_get_map(sched) );

    if (debug::is_enabled())
    {
        cout << "Schedule:" << endl;
        isl_printer_print_schedule(m_printer.get(), sched);
        cout << endl;

        cout << endl << "Schedule map:" << endl;
        print_each_in(sched_map);
        cout << endl;
    }

    isl_schedule_free(sched);

    return sched_map;
}

isl::union_map
ast_generator::make_init_schedule(isl::union_set & domains,
                                  isl::union_map & dependencies)
{
    isl::union_map init_schedule = make_schedule(domains, dependencies);

    if (debug::is_enabled())
    {
        cout << endl << "Init schedule:" << endl;
        print_each_in(init_schedule);
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
        print_each_in(steady_schedule);
        cout << endl;
    }

    return steady_schedule;
}

isl::union_map
ast_generator::schedule_finite_domains
(const isl::union_set & finite_domains, const isl::union_map & dependencies)
{
    auto schedule = make_schedule(finite_domains, dependencies).in_domain(finite_domains);

    if (debug::is_enabled())
    {
        cout << endl << "Finite schedule:" << endl;
        print_each_in(schedule);
        cout << endl;
    }

    return schedule;
}

pair<isl::union_map, isl::union_map>
ast_generator::make_periodic_schedule(const isl::union_map & schedule)
{
    int flow_dim = -1;
    int n_dims = 0;
    compute_period(schedule, flow_dim, n_dims);
    return make_pair(isl::union_map(m_model.context), isl::union_map(m_model.context));
}


pair<isl::union_map, isl::union_map>
ast_generator::schedule_infinite_domains
(const isl::union_set & infinite_domains, const isl::union_map & dependencies,
 isl::union_map & infinite_sched)
{
    using namespace isl;

    if (infinite_domains.is_empty())
        return make_pair(isl::union_map(m_model.context), isl::union_map(m_model.context));

    infinite_sched = make_schedule(infinite_domains, dependencies).in_domain(infinite_domains);

    if (debug::is_enabled())
    {
        cout << "Infinite schedule:" << endl;
        print_each_in(infinite_sched);
        cout << endl;
    }

    if (infinite_sched.is_empty())
        throw error("Could not compute infinite schedule.");

    return make_pair(isl::union_map(m_model.context), isl::union_map(m_model.context));
#if 0
    //auto infinite_sched_in_domain = infinite_sched.in_domain(infinite_domains);

    int flow_dim = -1;
    int n_dims = 0;

    int least_common_period = compute_period(infinite_sched, flow_dim, n_dims);

    int least_common_offset =
            common_offset(infinite_sched, flow_dim);

    auto period_range = set::universe(isl::space(m_model.context, set_tuple(n_dims)));
    {
        local_space cnstr_space(period_range.get_space());
        auto flow_var = cnstr_space(isl::space::variable, flow_dim);
        period_range.add_constraint(flow_var >= least_common_offset);
        period_range.add_constraint(flow_var < (least_common_offset + least_common_period));
    }

    auto period_sched_part = infinite_sched.in_range(period_range);

    isl::union_map period_sched(m_model.context);

    period_sched_part.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        // TODO:
#if 0
        if (!dynamic_cast<input_access*>(stmt->expr.get()))
        {
            period_sched = period_sched | m;
            return true;
        }
#endif

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

        // FIXME:
        int offset = min_i.integer();
        if (!stmt->array->period_offset)
            stmt->array->period_offset = offset;
        else if (stmt->array->period_offset != offset)
            cerr << "WARNING: different offsets for same buffer." << endl;

        return true;
    });

    if (debug::is_enabled())
    {
        cout << "Period schedule:" << endl;
        print_each_in(period_sched);
        cout << endl;
    }

    isl::union_map init_sched(m_model.context);

    infinite_sched.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        assert(stmt->flow_dim >= 0);

        local_space cnstr_space(m.get_space());

        auto stmt_flow_var = cnstr_space(isl::space::input, stmt->flow_dim);
        m.add_constraint(stmt_flow_var >= 0);

        auto sched_flow_var = cnstr_space(isl::space::output, flow_dim);
        m.add_constraint(sched_flow_var < least_common_offset);

        init_sched = init_sched | m;

        return true;
    });

    if (debug::is_enabled())
    {
        cout << "Init schedule:" << endl;
        print_each_in(init_sched);
        cout << endl;
    }

    m_schedule_flow_dim = flow_dim;
    m_schedule_period_offset = least_common_offset;

    return make_pair(init_sched, period_sched);
#endif
}

#if 1
int ast_generator::compute_period
(const isl::union_map & schedule, int & flow_dim_out, int & n_dims_out)
{
    // NOTE: Assuming that all the dependencies
    // are uniform in the infinite dimension of the schedule.
    // This means that, for each statement,
    // all instances of its infinite scheduling hyperplane
    // do exactly the same work and constitute a repeating period.
    // To get to the common period for all statements, we need to
    // get the least common multiple of the distances between
    // their infinite scheduling hyperplane instances.
    // That is, the LCM of the scheduling coefficients for the infinite
    // dimensionf of each statement.

    // For each statement, get the scheduling coefficient
    // for its infinite dimension.

    vector<pair<stmt_ptr,int>> ks;

    schedule.for_each( [&](const isl::map & stmt_sched)
    {
        auto name = stmt_sched.get_space().name(isl::space::input);
        stmt_ptr stmt;
        for (auto & s : m_model.statements)
        {
            if (s->name == name)
            {
                stmt = s;
                break;
            }
        }
        assert(stmt);

        if (!stmt->is_infinite)
            return true;

        // Get scheduling coefficient for the infinite dimension

        // NOTE: Assuming that the schedule does not differ across
        // the infinite domain (but may differ across others).

        stmt_sched.for_each( [&] (const isl::basic_map & stmt_sched_part)
        {
            // NOTE: Assuming that the schedule is a bijection,
            // the coefficients of interest must be only in equalities.

            // - Get the equalities matrix
            // - Among the rows, find the one that defines the first
            // dimension of the schedule.
            // - From this row, get the coefficient for the first dimension
            // of the statement.
            auto equalities = stmt_sched_part.equalities_matrix();
            auto space = stmt_sched_part.get_space();
            int n_rows = equalities.row_count();
            int n_in = space.dimension(isl::space::input);
            int stmt_coef_col = 0;
            int sched_coef_col = n_in;
            for (int row = 0; row < n_rows; ++row)
            {
                int sched_coef = equalities(row, sched_coef_col).value().integer();
                if (sched_coef)
                {
                    int stmt_coef = equalities(row, stmt_coef_col).value().integer();
                    ks.push_back(make_pair(stmt, std::abs(stmt_coef)));
                    // continue to next partial schedule for this stmt
                    return true;
                }
            }
            throw error("The first dimension of the schedule does not involve"
                        " the infinite statement dimension.");
        });

        return true;
    });

    // Find least common multiple of the coefficients.
    // This is the common period of statements.

    int least_common_period = 1;
    for (const auto & k : ks)
    {
        int period = k.second;
        least_common_period = lcm(least_common_period, period);
    }

    if (debug::is_enabled())
    {
        cout << "Least common period = " << least_common_period << endl;
    }

    // Compute how much of a statement's infinite dimension is covered
    // by the period.
    // This equals the offset in buffer indexes added at each period.
    for (const auto & k : ks)
    {
        auto stmt = k.first;
        int period = k.second;
        int span = least_common_period / period;

        if (debug::is_enabled())
        {
            cout << "Period spans " << span
                 << " iterations of " << stmt->name << endl;
        }

        if (!stmt->array->period)
            stmt->array->period = span;
        if (stmt->array->period != span)
            cerr << "WARNING: different buffer periods for the same array!" << endl;
    }

    return least_common_period;
}
#endif

#if 0
int ast_generator::common_offset(isl::union_map & schedule, int flow_dim)
{
    using namespace isl;

    // Assumption: schedule is bounded to iteration domains.

    int common_offset = std::numeric_limits<int>::min();

    schedule.for_each( [&](map & m)
    {
        auto id = m.id(isl::space::input);
        auto stmt = statement_for(id);
        if (stmt->flow_dim < 0)
            return true;

        auto space = m.get_space();
        int in_dims = space.dimension(space::input);

        // add constraint: iter[flow] < 0
        local_space cnstr_space(space);
        auto dim0_idx = cnstr_space(space::input, stmt->flow_dim);
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

    //isl::union_map finite_schedule_part(m_model.context);

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

    //isl::union_map infinite_schedule_part(m_model.context);

    infinite_schedule.for_each( [&]( isl::map & sched )
    {
        {
            // Lower-bound domain flow dim to 0;
            auto id = sched.id(isl::space::input);
            auto stmt = statement_for(id);

            isl::local_space space( sched.get_space() );
            auto in_flow = space(isl::space::input, stmt->flow_dim);
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
        print_each_in(combined_schedule);
        cout << endl;
    }
}

void ast_generator::compute_buffer_sizes( const isl::union_map & schedule,
                                          const data & d )
{
    using namespace isl;

    isl::space *time_space = nullptr;

    schedule.for_each([&time_space]( const map & m )
    {
        time_space = new space( m.range().get_space() );
        return false;
    });

    for (auto & array : m_model.arrays)
    {
        compute_buffer_size(schedule, d,
                            array, *time_space);
    }

    delete time_space;

    for (auto & array : m_model.arrays)
    {
        if (array->buffer_size.empty())
        {
            for (int dim = 0; dim < array->size.size(); ++dim)
            {
                if (dim == array->flow_dim)
                    // FIXME:
                    array->buffer_size.push_back(array->period_offset + array->period);
                else
                    array->buffer_size.push_back(array->size[dim]);
                assert(array->buffer_size.back() >= 0);
            }
        }
    }
}

void ast_generator::compute_buffer_size
( const isl::union_map & schedule,
  const data & d,
  const array_ptr & array,
  const isl::space & time_space )
{
    if (debug_buffer_size::is_enabled())
    {
        cout << "Buffer size for array: " << array->name << endl;
    }

    using namespace isl;
    using isl::expression;

    auto array_space = isl::space( m_model.context,
                                   isl::set_tuple( isl::identifier(array->name),
                                                   array->size.size() ) );

    auto array_sched_space = isl::space::from(array_space, time_space);

    // Filter and map writers and readers

    auto all_write_sched = schedule;
    all_write_sched.map_domain_through(d.write_relations);
    auto write_sched = all_write_sched.map_for(array_sched_space);

    auto all_read_sched = schedule;
    all_read_sched.map_domain_through(d.read_relations);
    auto read_sched = all_read_sched.map_for(array_sched_space);

    if (write_sched.is_empty() || read_sched.is_empty())
        return;

    // Do the work

    map not_later = order_greater_than_or_equal(time_space);
    map later = order_less_than(time_space);

    // Find all source instances scheduled before t, for all t.
    // => Create map: t -> src
    //    such that: time(src) <= t

    auto written_not_later = write_sched.inverse()( not_later );

    // Find all instances of source consumed after time t, for each t;
    // => Create map: t -> src
    //    such that: time(sink(src)) <= t, for all sink

    auto read_later = read_sched.inverse()( later );

    // Find all src instances live at the same time.
    // = Create map: t -> src,
    //   Such that: time(src) <= t and time(sink(src)) <= t, for all sink
    auto buffered = written_not_later & read_later;

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
        int buf_dim_count = array->size.size();
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

    array->buffer_size = buffer_size;
}

void ast_generator::find_inter_period_deps
( const isl::union_map & period_schedule,
  const data & d )
{
    cout << "Sched flot dim = " << m_schedule_flow_dim << endl;
    cout << "Sched period offset = " << m_schedule_period_offset << endl;
    for( auto & array : m_model.arrays )
    {
        auto array_space = isl::space( m_model.context,
                                       isl::set_tuple( isl::identifier(array->name),
                                                       array->size.size() ) );
        auto array_universe = isl::basic_set::universe(array_space);

        auto read_in_period =
                d.read_relations( period_schedule.domain() )
                & array_universe;

        auto writers = d.write_relations.inverse()( read_in_period );

        {
            auto sched = d.finite_schedule.in_domain(writers);
            if (!sched.is_empty())
            {
                array->inter_period_dependency = true;
                cout << "Array " << array->name << " written by finite:" << endl;
                m_printer.print(sched); cout << endl;
                continue;
            }
        }

        {
            array->inter_period_dependency = false;

            auto sched = d.infinite_schedule.in_domain(writers);
            if (sched.is_empty())
                continue;

            sched.for_each( [&](const isl::map & m )
            {
                auto s = m.range();
                isl::local_space spc(s.get_space());
                auto flow_dim = spc(isl::space::variable, m_schedule_flow_dim);
                auto min_time = s.minimum(flow_dim);

                if (min_time.integer() < m_schedule_period_offset)
                {
                    array->inter_period_dependency = true;

                    cout << "Array " << array->name << " written by infinite: " << endl;
                    m_printer.print(m); cout << endl;

                    return false;
                }

                return true;
            });
        }
    }
}
#endif
void ast_generator::print_each_in( const isl::union_set & us )
{
    us.for_each ( [&](const isl::set & s){
       m_printer.print(s);
       cout << endl;
       return true;
    });
}

void ast_generator::print_each_in( const isl::union_map & um )
{
    um.for_each ( [&](const isl::map & m){
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
