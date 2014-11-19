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
    vector<statement*> finite_statements;
    vector<statement*> infinite_statements;
    vector<statement*> invalid_statements;

    for(statement *stmt : statements)
    {
        vector<int> infinite_dims = stmt->infinite_dimensions();
        if (infinite_dims.empty())
            finite_statements.push_back(stmt);
        else if (infinite_dims.size() == 1)
        {
            infinite_statements.push_back(stmt);

            actor a(stmt);
            a.flow_dimension = infinite_dims.front();
            m_actors.emplace(stmt, a);
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
        throw std::runtime_error(msg.str());
    }

    for(auto & actor: m_actors)
    {
        compute_channels(actor.second);
    }

    compute_schedule();
}

void model::compute_channels( actor & sink )
{
    vector<stream_access*> accesses;

    sink.stmt->expr->find<stream_access>(accesses);
    if (accesses.empty())
        return;

    for(stream_access *access : accesses)
    {
        actor & source = m_actors.at(access->target);

        int source_flow_dim = -1;
        for (int out_dim = 0;
             out_dim < access->pattern.output_dimension();
             ++out_dim)
        {
            if (access->pattern.coefficients(out_dim, sink.flow_dimension) != 0)
            {
                source_flow_dim = out_dim;
                break;
            }
        }

        if (source_flow_dim < 0)
            throw std::runtime_error("Sink flow dimension does not map"
                                     " to any source dimension.");
        if (source_flow_dim != source.flow_dimension)
            throw std::runtime_error("Sink flow dimension does not map"
                                     " to source flow dimension.");

        int flow_pop = access->pattern.coefficients(source_flow_dim, sink.flow_dimension);

        vector<int> sink_index = sink.stmt->domain;
        sink_index[sink.flow_dimension] = 0;
        vector<int> source_index = access->pattern * sink_index;
        int flow_peek = std::max(1, source_index[source_flow_dim]);

        channel ch;
        ch.source = &source;
        ch.sink = &sink;
        ch.push = 1;
        ch.peek = flow_peek;
        ch.pop = flow_pop;

        m_channels.push_back(ch);

        cout << endl << "Channel:" << endl;
        cout << ch.source << "@" << source.flow_dimension << " " << ch.push << " -> "
             << ch.peek << "/" << ch.pop << " " << ch.sink << "@" << sink.flow_dimension
             << endl;
    }
}

void model::compute_schedule()
{
    //using namespace isl;

    if (m_actors.empty())
        return;

    isl::context isl_ctx;
    isl::printer isl_printer(isl_ctx);

    // FIXME: Multiple dependencies between same pair of statements

    unordered_set<actor*> involved_actors;

    for (const auto & chan: m_channels)
    {
        involved_actors.insert(chan.source);
        involved_actors.insert(chan.sink);
    }

    int rows = m_channels.size();
    int cols = involved_actors.size();

    isl::matrix flow_matrix(isl_ctx, rows, cols);

    for (int r = 0; r < rows; ++r)
        for(int c = 0; c < cols; ++c)
            flow_matrix(r,c) = 0;

    int row = 0;
    for (const auto & chan: m_channels)
    {
        auto source_loc = involved_actors.find(chan.source);
        int source_index = std::distance(involved_actors.begin(), source_loc);
        auto sink_loc = involved_actors.find(chan.sink);
        int sink_index = std::distance(involved_actors.begin(), sink_loc);

        cout << "source: " << chan.source->stmt->name << "@" << source_index << endl;
        cout << "sink: " << chan.sink->stmt->name << "@" << sink_index << endl;
        flow_matrix(row, source_index) = chan.push;
        flow_matrix(row, sink_index) = - chan.pop;
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

    isl::space statement_space(isl_ctx, isl::set_tuple(involved_actors.size()));
    auto init_counts = isl::set::universe(statement_space);
    auto init_cost = isl::expression::value(statement_space, 0);
    cout << "Init count cost expr:" << endl;
    isl_printer.print(init_cost); cout << endl;
    for (int i = 0; i < involved_actors.size(); ++i)
    {
        auto stmt = isl::expression::variable(statement_space, isl::space::variable, i);
        init_counts.add_constraint( stmt >= 0 );
        init_cost = stmt + init_cost;
        cout << "Init count cost expr:" << endl;
        isl_printer.print(init_cost); cout << endl;
    }
    for (const auto & chan: m_channels)
    {
        auto source_ref = involved_actors.find(chan.source);
        int source_index = std::distance(involved_actors.begin(), source_ref);
        auto sink_ref = involved_actors.find(chan.sink);
        int sink_index = std::distance(involved_actors.begin(), sink_ref);

        auto source = isl::expression::variable(statement_space,
                                                isl::space::variable,
                                                source_index);
        auto sink = isl::expression::variable(statement_space,
                                              isl::space::variable,
                                              sink_index);
        int source_steady = steady_counts(source_index,0).value().numerator();
        int sink_steady = steady_counts(sink_index,0).value().numerator();

        // p(a)*i(a) - o(b)*i(b) + [p(a)*s(a) - o(b)*s(b) - e(b) + o(b)] >= 0

        auto constraint =
                chan.push * source - chan.pop * sink
                + (chan.push * source_steady - chan.pop * sink_steady - chan.peek + chan.pop)
                >= 0;

        init_counts.add_constraint(constraint);
    }

    cout << "Viable initialization counts:" << endl;
    isl_printer.print(init_counts); cout << endl;

    auto init_optimum = init_counts.minimum(init_cost);
    init_counts.add_constraint( init_cost == init_optimum );
    auto init_optimum_point = init_counts.single_point();

    cout << "Initialization Counts:" << endl;
    isl_printer.print(init_optimum_point); cout << endl;

    assert(steady_counts.column_count() == 1);
    assert(steady_counts.row_count() == involved_actors.size());
    auto actor_ref = involved_actors.begin();
    for (int actor_idx = 0; actor_idx < involved_actors.size(); ++actor_idx, ++actor_ref)
    {
        actor & a = (**actor_ref);
        a.init_count =
                (int) init_optimum_point(isl::space::variable, actor_idx).numerator();
        a.steady_count =
                steady_counts(actor_idx,0).value().numerator();
    }
}

}
}
