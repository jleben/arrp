#ifndef STREAM_LANG_DATAFLOW_MODEL_INCLUDED
#define STREAM_LANG_DATAFLOW_MODEL_INCLUDED

#include "model.hpp"
#include "../utility/debug.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace stream {
namespace dataflow {

using polyhedral::statement;
using polyhedral::stream_access;
using std::vector;
using std::unordered_map;
using std::string;

struct actor
{
    actor(statement* stmt, int id):
        stmt(stmt),
        id(id),
        flow_dimension(0),
        init_count(0),
        steady_count(0)
    {}

    statement *stmt;
    int id;
    int flow_dimension;
    int init_count;
    int steady_count;
};

class model
{
public:
    struct debug : public stream::debug::topic<debug, stream::debug::all>
    { static string id() { return "dataflow"; } };

    model( const vector<statement*> & );

    const dataflow::actor * find_actor_for( statement * stmt ) const
    {
        auto ref = m_actors.find(stmt);
        if (ref != m_actors.end())
            return &ref->second;
        else
            return nullptr;
    }
    const dataflow::actor & actor_for( statement * stmt ) const
    {
        return m_actors.at(stmt);
    }

private:
    struct channel
    {
        actor *source;
        actor *sink;
        int push;
        int peek;
        int pop;
    };

    void compute_channels();
    void compute_schedule();

    unordered_map<statement*, actor> m_actors;
    vector<channel> m_channels;
};

}
}

#endif // STREAM_LANG_DATAFLOW_MODEL_INCLUDED
