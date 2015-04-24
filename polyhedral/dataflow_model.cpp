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

#include "../common/dataflow_model.hpp"

#include <isl-cpp/context.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/expression.hpp>
#include <isl-cpp/matrix.hpp>
#include <isl-cpp/utility.hpp>
#include <isl-cpp/printer.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace stream {
namespace dataflow {

struct debug : public stream::debug::topic<debug, stream::debug::all>
{ static string id() { return "dataflow"; } };

model::model( const vector<statement*> & statements )
{
    if (debug::is_enabled())
        cout << endl << "### Dataflow Analysis ###" << endl;

    vector<statement*> finite_statements;
    vector<statement*> infinite_statements;
    vector<statement*> invalid_statements;

    int actor_id = 0;
    for(statement *stmt : statements)
    {
        vector<int> infinite_dims = stmt->infinite_dimensions();
        if (infinite_dims.empty())
            finite_statements.push_back(stmt);
        else if (infinite_dims.size() == 1)
        {
            infinite_statements.push_back(stmt);

            actor a(stmt, actor_id++);
            a.flow_dimension = infinite_dims.front();
            m_actors.emplace(stmt, a);

            stmt->flow_dim = a.flow_dimension;
        }
        else
            invalid_statements.push_back(stmt);
    }

    if (debug::is_enabled())
    {
        cout << endl << "Statement types:" << endl;
        cout << "- finite: " << finite_statements.size() << endl;
        cout << "- infinite: " << infinite_statements.size() << endl;
        cout << "- invalid: " << invalid_statements.size() << endl;
    }

    if (!invalid_statements.empty())
    {
        ostringstream msg;
        msg << "The following statements are infinite"
            << " in more than 1 dimension: " << endl;
        for (statement *stmt: invalid_statements)
        {
            msg << "- " << stmt->name << endl;
        }
        throw std::runtime_error(msg.str());
    }

    compute_channels();

    if (!m_channels.empty())
        compute_schedule();

    for (auto & actor_record : m_actors)
    {
        actor & a = actor_record.second;
        if (!a.steady_count)
        {
            cout << "WARNING: Infinite statement with no channels: " << a.stmt->name << endl;
            cout << '\t' << "Assuming default steady-period count of 1.";
            a.steady_count = 1;
        }
    }
}

void model::compute_channels()
{
    for (auto & actor_record : m_actors)
    {
        actor & sink = actor_record.second;

        if (debug::is_enabled())
            cout << "Channels for: " << sink.stmt->name << "..." << endl;

        vector<stmt_access*> accesses;
        sink.stmt->expr->find<stmt_access>(accesses);
        if (accesses.empty())
            return;

        for(stmt_access *access : accesses)
        {
            auto source_record = m_actors.find(access->target);
            if (source_record == m_actors.end())
            {
                if (debug::is_enabled())
                {
                    cout << "-- " << access->target->name
                         << ": Non-actor access; not making a channel." << endl;
                }
                continue;
            }

            actor & source = source_record->second;

            // pop(source,sink) = M_source,sink[d_flow(source), d_flow(sink)]

            int pop_rate = access->pattern.coefficients(source.flow_dimension,
                                                        sink.flow_dimension);
            assert(pop_rate != 0);

            // peek(source,sink) = single-token-volume(source,sink)[d_flow(sink)] :
            //    single-token-volume(source,sink) =
            //        M_source,sink * [d_1, d_2, d_flow,...]
            //    d_x = D_sink(x)
            //    d_flow = 0

            vector<int> sink_index = sink.stmt->domain;
            sink_index[sink.flow_dimension] = 0;

            vector<int> source_index = access->pattern * sink_index;

            int peek_rate = source_index[source.flow_dimension] + 1;

            channel ch;
            ch.source = &source;
            ch.sink = &sink;
            ch.push = 1;
            ch.peek = peek_rate;
            ch.pop = pop_rate;

            m_channels.push_back(ch);

            if (debug::is_enabled())
            {
                cout << "-- " << ch.source->stmt->name << ": "
                     << ch.push << " -> "
                     << ch.peek << "/" << ch.pop
                     << endl;
            }
        }
    }
}

/*
  Steady-state counts:

  Number of executions of actors in the steady (periodic) schedule
  = the minimal nullspace of the flow matrix.

  Flow matrix F is m*n matrix,
  where m is number of channels and n is number of actors,
  and F(i,j) = push(c_i, a_j) - pop(c_i, a_j).
*/
vector<int> model::steady_counts()
{
    isl::context isl_ctx;

    int rows = m_channels.size();
    int cols = m_actors.size();

    isl::matrix flow_matrix(isl_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for(int c = 0; c < cols; ++c)
            flow_matrix(r,c) = 0;

    int row = 0;
    for (const auto & chan: m_channels)
    {
        if (chan.source != chan.sink)
        {
            flow_matrix(row, chan.source->id) = chan.push;
            flow_matrix(row, chan.sink->id) = - chan.pop;
        }
        else
        {
            // In a consistent model, this should always be 0.
            // Nevertheless...
            flow_matrix(row, chan.sink->id) = chan.push - chan.pop;
        }
        ++row;
    }

    if (debug::is_enabled())
    {
        cout << "Flow:" << endl;
        isl::print(flow_matrix);
    }

    isl::matrix nullspace = flow_matrix.nullspace();

    assert(nullspace.column_count() == 1);
    assert(nullspace.row_count() == m_actors.size());

    vector<int> counts;
    counts.reserve(m_actors.size());

    for(unsigned int i = 0; i < m_actors.size(); ++i)
    {
        counts.push_back( nullspace(i,0).value().numerator() );
    }

    if (debug::is_enabled())
    {
        cout << "Steady Counts:" << endl;
        for(int c : counts)
            cout << c << " ";
        cout << endl;
    }

    return counts;
}

/*
  Initialization counts:

  Number of tokens produced should be at least number of tokens consumed
  after the initial epoch + one steady period.

  Notation:
    c_init(s) = init count of statement s
    c_steady(s) = steady count of statement s
    push(s) = push rate of statement s along an anonymous channel
    pop(s) = pop rate of statement s along an anonymous channel
    peek(s) = peek rate of statement s along an anonymous channel

  Purpose:
    Determine appropriate c_init(s) for all s.

  Optimization problem:
  Minimize:
    c_init(s1) + c_init(s2) + ... + c_init(sn)
  Such that:
    For each sx:
      c_init(sx) >= 0
    For each channel (sa,sb):
      buffer_after_init(sa,sb) >= peek_ahead(sb)
        buffer_after_init(sa,sb) = c_init(sa) * push(sa) - c_init(sb) * pop(sb);
        peek_ahead(sb) = peek(sb) - pop(sb);
*/
vector<int> model::initial_counts()
{
    isl::context isl_ctx;
    isl::printer isl_printer(isl_ctx);

    isl::space statement_space(isl_ctx, isl::set_tuple(m_actors.size()));
    auto init_counts = isl::set::universe(statement_space);
    auto init_cost = isl::expression::value(statement_space, 0);
    for (int i = 0; i < m_actors.size(); ++i)
    {
        auto stmt = isl::expression::variable(statement_space, isl::space::variable, i);
        init_counts.add_constraint( stmt >= 0 );
        init_cost = stmt + init_cost;
    }
    for (const auto & chan: m_channels)
    {
        auto source = isl::expression::variable(statement_space,
                                                isl::space::variable,
                                                chan.source->id);
        auto sink = isl::expression::variable(statement_space,
                                              isl::space::variable,
                                              chan.sink->id);

        auto constraint =
                chan.push * source - chan.pop * sink
                - chan.peek + chan.pop
                >= 0;

        init_counts.add_constraint(constraint);
    }

    if (debug::is_enabled())
    {
        cout << "Viable initialization counts:" << endl;
        isl_printer.print(init_counts); cout << endl;
    }

    auto init_optimum = init_counts.minimum(init_cost);
    init_counts.add_constraint( init_cost == init_optimum );
    auto init_optimum_point = init_counts.single_point();

    if (debug::is_enabled())
    {
        cout << "Initialization Counts:" << endl;
        isl_printer.print(init_optimum_point); cout << endl;
    }

    vector<int> init_counts_vec;
    init_counts_vec.reserve(m_actors.size());

    for(unsigned int i = 0; i < m_actors.size(); ++i)
    {
        int count = (int) init_optimum_point(isl::space::variable, i).numerator();
        init_counts_vec.push_back(count);
    }

    return init_counts_vec;
}

void model::compute_schedule()
{
    //using namespace isl;

    assert(!m_actors.empty());
    assert(!m_channels.empty());

    vector<int> steady_counts = this->steady_counts();
    vector<int> init_counts = this->initial_counts();

    assert(steady_counts.size() == m_actors.size());
    assert(init_counts.size() == m_actors.size());

    for (auto & actor_record : m_actors)
    {
        actor & a = actor_record.second;
        a.init_count = init_counts[a.id];
        a.steady_count = steady_counts[a.id];
    }
}

}
}
