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
using polyhedral::stmt_access;
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

    bool empty() { return m_actors.empty(); }

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
