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

#include <isl/ast.h>

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <unordered_map>

namespace stream {
namespace polyhedral {

using std::vector;
using std::string;
using std::unordered_map;
using affine_matrix = utility::mapping;
typedef code_location location_type;

class array;
class statement;
typedef std::shared_ptr<array> array_ptr;
typedef std::shared_ptr<statement> stmt_ptr;

enum {
    infinite = -1
};

struct array_relation
{
    array_relation() {}
    array_relation(array_ptr a, const affine_matrix & m,
                   const vector<int> & s = vector<int>()):
        array(a), matrix(m), size(s) {}

    array_ptr array = nullptr;
    affine_matrix matrix;
    vector<int> size;
};

isl::map to_isl_map(const stmt_ptr &, const array_relation &);

class array
{
public:
    array(const string & name,
          const isl::set & domain,
          primitive_type type):
        name(name), domain(domain), type(type) {}

    string name;
    isl::set domain;
    primitive_type type;
    bool is_infinite = false;
    vector<int> size;

#if 1
    vector<int> buffer_size;
    //int flow_dim = -1;
    int period = 0;
    int period_offset = 0;
    bool inter_period_dependency = true;
#endif
};

class statement
{
public:
    statement(const isl::set & d):
        name(d.name()), domain(d)
    {
        auto id = domain.id();
        id.data = this;
        domain.set_id(id);
    }

    string name;
    isl::set domain;
    functional::expr_ptr expr = nullptr;
    array_relation write_relation;
    vector<array_relation*> read_relations;
    unordered_map<array*, int> array_access_offset;
    bool streaming_needs_modulo = false;
    bool is_infinite = false;
};

class array_read : public functional::expression
{
public:
    array_read(array_ptr a, const affine_matrix & r,
               const location_type & l):
        expression(l), relation(a,r) {}

    array_relation relation;
    //array_ptr array;
    //affine_matrix matrix;
};

class iterator_read : public functional::expression
{
public:
    iterator_read(int i, const location_type & l):
        expression(l, functional::make_int_type()), index(i) {}
    int index;
};

class external_call : public functional::expression
{
public:
    external_call(const location_type & l):
        expression(l) {}
    string name;
    array_relation source;
};

class model
{
public:
    isl::context context;
    vector<array_ptr> arrays;
    vector<stmt_ptr> statements;
    unordered_map<string, array_ptr> phase_ids;
};

class model_summary
{
public:
    model_summary(const model & m):
        domains(m.context),
        write_relations(m.context),
        read_relations(m.context),
        dependencies(m.context)
    {
        for (const auto & stmt : m.statements)
        {
            domains = domains | stmt->domain;

            if (stmt->write_relation.array)
            {
                auto space = isl::space::from(stmt->domain.get_space(),
                                              stmt->write_relation.array->domain.get_space());
                auto map = to_isl_map(stmt, stmt->write_relation);
                write_relations = write_relations | map;
            }

            for(const auto & rel : stmt->read_relations)
            {
                auto space = isl::space::from(stmt->domain.get_space(),
                                              rel->array->domain.get_space());
                auto map = to_isl_map(stmt, *rel);
                read_relations = read_relations | map;
            }
        }

        dependencies =
                read_relations.in_domain(domains).inverse()
                (write_relations.in_domain(domains));
    }

    isl::union_set domains;
    isl::union_map write_relations;
    isl::union_map read_relations;
    isl::union_map dependencies;
};

class schedule
{
public:
    schedule(const isl::context & ctx):
        tree(nullptr), full(ctx),
        prelude_tree(nullptr), prelude(ctx),
        period_tree(nullptr), period(ctx),
        params(isl::set(isl::space(ctx, isl::parameter_tuple())))
    {}
    isl::schedule tree;
    isl::union_map full;
    isl::schedule prelude_tree;
    isl::union_map prelude;
    isl::schedule period_tree;
    isl::union_map period;
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
    ~ast_isl()
    {
        isl_ast_node_free(prelude);
        isl_ast_node_free(period);
    }

    isl_ast_node * prelude = nullptr;
    isl_ast_node * period = nullptr;
};

} // namespace polyhedral
} // namespace stream

#endif // STREAM_POLYHEDRAL_MODEL_2_INCLUDED
