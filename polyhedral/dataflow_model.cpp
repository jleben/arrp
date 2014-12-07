#include "dataflow_model.hpp"

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
            cout << "Channels for: " << sink.stmt << "..." << endl;

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
                    cout << "-- " << access->target
                         << ": Non-actor access; not making a channel." << endl;
                }
                continue;
            }

            actor & source = source_record->second;

            int pop_rate = access->pattern.coefficients(source.flow_dimension, sink.flow_dimension);
            assert(pop_rate != 0);

            vector<int> sink_index = sink.stmt->domain;
            sink_index[sink.flow_dimension] = 0;
            vector<int> source_index = access->pattern * sink_index;
            int peek_rate = std::max(1, source_index[source.flow_dimension]);

            channel ch;
            ch.source = &source;
            ch.sink = &sink;
            ch.push = 1;
            ch.peek = peek_rate;
            ch.pop = pop_rate;

            m_channels.push_back(ch);

            if (debug::is_enabled())
            {
                cout << "-- " << ch.source->stmt << ": "
                     << ch.push << " -> "
                     << ch.peek << "/" << ch.pop
                     << endl;
            }
        }
    }
}

void model::compute_schedule()
{
    //using namespace isl;

    assert(!m_actors.empty());
    assert(!m_channels.empty());

    isl::context isl_ctx;
    isl::printer isl_printer(isl_ctx);

    int rows = m_channels.size();
    int cols = m_actors.size();

    isl::matrix flow_matrix(isl_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for(int c = 0; c < cols; ++c)
            flow_matrix(r,c) = 0;

    int row = 0;
    for (const auto & chan: m_channels)
    {
        flow_matrix(row, chan.source->id) = chan.push;
        flow_matrix(row, chan.sink->id) = - chan.pop;
        ++row;
    }

    if (debug::is_enabled())
    {
        cout << "Flow:" << endl;
        isl::print(flow_matrix);
    }

    isl::matrix steady_counts = flow_matrix.nullspace();

    if (debug::is_enabled())
    {
        cout << "Steady Counts:" << endl;
        isl::print(steady_counts);
    }

    // Initialization counts:

    // Number of tokens produced should be at least number of tokens consumed
    // after the initial epoch + one steady period.

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
        int source_steady = steady_counts(chan.source->id,0).value().numerator();
        int sink_steady = steady_counts(chan.sink->id,0).value().numerator();

        // p(a)*i(a) - o(b)*i(b) + [p(a)*s(a) - o(b)*s(b) - e(b) + o(b)] >= 0

        auto constraint =
                chan.push * source - chan.pop * sink
                + (chan.push * source_steady - chan.pop * sink_steady - chan.peek + chan.pop)
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

    assert(steady_counts.column_count() == 1);
    assert(steady_counts.row_count() == m_actors.size());

    for (auto & actor_record : m_actors)
    {
        actor & a = actor_record.second;
        a.init_count =
                (int) init_optimum_point(isl::space::variable, a.id).numerator();
        a.steady_count =
                steady_counts(a.id, 0).value().numerator();
    }
}

}
}
