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

#ifndef STREAM_POLYHEDRAL_MODEL_2_INCLUDED
#define STREAM_POLYHEDRAL_MODEL_2_INCLUDED

#include "primitives.hpp"
#include "functional_model.hpp"
#include "../frontend/location.hh"
#include "../utility/mapping.hpp"
#include "../utility/debug.hpp"

#include <isl-cpp/context.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/schedule.hpp>
#include <isl-cpp/expression.hpp>

#include <isl/ast.h>

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <list>

namespace stream {
namespace polyhedral {

using std::vector;
using std::list;
using std::string;
using std::unordered_map;
using std::shared_ptr;
using affine_matrix = utility::mapping;
using functional::expr_ptr;
typedef code_location location_type;

class array;
class statement;
class array_access;
typedef std::shared_ptr<array> array_ptr;
typedef std::shared_ptr<statement> stmt_ptr;

enum {
    infinite = -1
};

class array
{
public:
    array(): domain(nullptr) {}
    array(const string & name,
          const isl::set & d,
          primitive_type type):
        name(name), domain(d), type(type)
    {}

    string name;
    isl::set domain;
    primitive_type type;
    bool is_infinite = false;
    vector<int> size;

#if 1
    vector<int> buffer_size;
    //int flow_dim = -1;
    int period = 0;
    int first_period_access = 0;
    int last_period_access = 0;
    bool inter_period_dependency = true;
#endif
};

class statement
{
public:
    statement(const isl::set & d):
        name(d.name()), domain(d)
    {}

    string name;
    isl::set domain;
    functional::expr_ptr expr = nullptr;
    isl::map self_relations { nullptr };
    vector<shared_ptr<array_access>> array_accesses;
    unordered_map<array*, int> array_access_offset;
    bool streaming_needs_modulo = false;
    bool is_infinite = false;
    bool is_input_or_output = false;
};

class array_access : public functional::expression
{
public:
    array_ptr array;

    // indexes:
    // Used to generate C++.
    // There can be fewer indexes than array dimensions,
    // which means partial indexing.
    vector<functional::expr_ptr> indexes;

    // map:
    // Used for ISL model.
    isl::map map { nullptr };

    bool writing = false;
    bool reading = false;
};

class iterator_read : public functional::expression
{
public:
    iterator_read(int i, const location_type & l = location_type()):
        expression(l, functional::make_int_type()), index(i) {}
    int index;
};

class external_call : public functional::expression
{
public:
    external_call(const location_type & l = location_type()):
        expression(l) {}

    string name;
    vector<functional::expr_ptr> args;
};

class assignment : public functional::expression
{
public:
    assignment(const expr_ptr & d, const expr_ptr & v):
        expression(location_type(), v->type),
        destination(d),
        value(v)
    {}

    expr_ptr destination;
    expr_ptr value;
};

class io_channel
{
public:
    string name;
    functional::type_ptr type;
    array_ptr array;
    stmt_ptr statement;
    int latency = 0;
};

class model
{
public:
    isl::context context;
    vector<array_ptr> arrays;
    vector<stmt_ptr> statements;
    vector<io_channel> inputs;
    vector<io_channel> outputs;
    unordered_map<string, array_ptr> phase_ids;
    isl::union_map parallel_accesses { nullptr };

    isl::union_map clock_relations { nullptr };
};

class model_summary
{
public:
    model_summary(const model & m):
        array_domains(m.context),
        domains(m.context),
        write_relations(m.context),
        read_relations(m.context),
        dependencies(m.context),
        order_relations(m.context)
    {
        for (const auto & array : m.arrays)
        {
            array_domains |= array->domain;
        }

        for (const auto & stmt : m.statements)
        {
            domains = domains | stmt->domain;

            for(const auto & rel : stmt->array_accesses)
            {
                if (rel->reading)
                    read_relations = read_relations | rel->map;
                if (rel->writing)
                    write_relations = write_relations | rel->map;
            }
        }

        dependencies =
                read_relations.in_domain(domains).inverse()
                (write_relations.in_domain(domains));

        for (const auto & stmt : m.statements)
        {
            if (stmt->self_relations.is_valid())
                order_relations |= stmt->self_relations.in_domain(stmt->domain).in_range(stmt->domain);
        }

        if (m.clock_relations.is_valid())
        {
            order_relations |= m.clock_relations;
        }
    }

    isl::union_set array_domains;
    isl::union_set domains;
    isl::union_map write_relations;
    isl::union_map read_relations;
    isl::union_map dependencies;
    isl::union_map order_relations;
};

class schedule
{
public:
    schedule(const isl::context & ctx):
        tree(nullptr), full(ctx),
        prelude_tree(nullptr), prelude(ctx),
        period_tree(nullptr), period(ctx),
        tiled(ctx),
        params(isl::set::universe(isl::space(ctx, isl::parameter_tuple())))
    {}
    isl::schedule tree;
    isl::union_map full;
    isl::schedule prelude_tree;
    isl::union_map prelude;
    isl::schedule period_tree;
    isl::union_map period;
    isl::union_map tiled;
    isl::set params;
};

#if 0
class ast
{
public:
    CloogOptions * options;
    CloogState * state;
    clast_stmt * prelude;
    clast_stmt * period;
};
#endif

class ast_isl
{
public:
    ast_isl() {}

    ast_isl(const ast_isl & other)
    {
        full = isl_ast_node_copy(other.full);
        prelude = isl_ast_node_copy(other.prelude);
        period = isl_ast_node_copy(other.period);
    }

    ~ast_isl()
    {
        isl_ast_node_free(full);
        isl_ast_node_free(prelude);
        isl_ast_node_free(period);
    }

    ast_isl & operator=(const ast_isl & other)
    {
        full = isl_ast_node_copy(other.full);
        prelude = isl_ast_node_copy(other.prelude);
        period = isl_ast_node_copy(other.period);
        return *this;
    }

    isl_ast_node * full = nullptr;
    isl_ast_node * prelude = nullptr;
    isl_ast_node * period = nullptr;
};

struct ast_node_info
{
    bool is_parallelizable = false;
    bool is_parallel = false;
    bool is_vector = false;

    static ast_node_info * get_from_id(isl_id * id)
    {
        return reinterpret_cast<ast_node_info*>(isl_id_get_user(id));
    }

    static isl_id * create_on_id(isl::context &, const string & name = string());
};

} // namespace polyhedral
} // namespace stream

#endif // STREAM_POLYHEDRAL_MODEL_2_INCLUDED
