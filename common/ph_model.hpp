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

#include <cloog/cloog.h>

#include <vector>
#include <string>
#include <iostream>
#include <memory>

namespace stream {
namespace polyhedral {

struct debug : public stream::debug::topic<debug, stream::debug::all>
{ static string id() { return "polyhedral"; } };

using std::vector;
using std::string;
using affine_matrix = utility::mapping;
typedef parsing::location location_type;

class statement;
class array;

enum {
    infinite = -1
};

isl::matrix to_isl_equalities_matrix(const affine_matrix &, const isl::context &);
isl::map to_isl_map(const affine_matrix &, const isl::space &);

class array
{
public:
    array(const string & name,
          const isl::set & domain):
        name(name), domain(domain) {}

    string name;
    isl::set domain;
    bool is_infinite = false;

#if 1
    vector<int> buffer_size;
    //int flow_dim = -1;
    int period = 0;
    int period_offset = 0;
    bool inter_period_dependency = true;
#endif
};
typedef std::shared_ptr<array> array_ptr;

struct array_relation
{
    array_ptr array;
    affine_matrix matrix;
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
    array_ptr array;
    array_relation write_relation;
    vector<array_relation> read_relations;
    bool is_infinite = false;
};
typedef std::shared_ptr<statement> stmt_ptr;

class array_read : public functional::expression
{
public:
    array_read(array_ptr a, const affine_matrix & r,
               const location_type & l):
        expression(l), array(a), read_relation(r) {}

    array_ptr array;
    affine_matrix read_relation;
};

class iterator_read : public functional::expression
{
public:
    iterator_read(int i, const location_type & l):
        expression(l), index(i) {}
    int index;
};

class model
{
public:
    isl::context context;
    vector<array_ptr> arrays;
    vector<stmt_ptr> statements;
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

            {
                auto space = isl::space::from(stmt->domain.get_space(),
                                              stmt->array->domain.get_space());
                auto map = to_isl_map(stmt->write_relation.matrix, space);
                write_relations = write_relations | map;
            }

            for(const auto & rel : stmt->read_relations)
            {
                auto space = isl::space::from(stmt->domain.get_space(),
                                              rel.array->domain.get_space());
                auto map = to_isl_map(rel.matrix, space);
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
        full(ctx), prelude(ctx), period(ctx) {}
    isl::union_map full;
    isl::union_map prelude;
    isl::union_map period;
};

class ast
{
public:
    CloogOptions * options;
    CloogState * state;
    clast_stmt * prelude;
    clast_stmt * period;
};

} // namespace polyhedral
} // namespace stream

#endif // STREAM_POLYHEDRAL_MODEL_2_INCLUDED
