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

#ifndef STREAM_POLYHEDRAL_MODEL_INCLUDED
#define STREAM_POLYHEDRAL_MODEL_INCLUDED

#include "../common/primitives.hpp"
#include "../utility/mapping.hpp"
#include "../utility/debug.hpp"

#include <vector>
#include <string>
#include <iostream>
#include <memory>

namespace stream {
namespace polyhedral {

using std::vector;
using std::string;
using utility::mapping;

enum
{
    infinite = -1
};

template <primitive_type> struct cpp_type;
template<> struct cpp_type<primitive_type::integer> { typedef int type; };
template<> struct cpp_type<primitive_type::real> { typedef double type; };
template<> struct cpp_type<primitive_type::boolean> { typedef bool type; };

template <typename T> struct expr_type;
template<> struct expr_type<int> { static constexpr primitive_type type = primitive_type::integer; };
template<> struct expr_type<double> { static constexpr primitive_type type = primitive_type::real; };
template<> struct expr_type<bool> { static constexpr primitive_type type = primitive_type::boolean; };

class statement;
class array;

class expression
{
    // TODO: provide easy access to all dependencies,
    // (of all sub-expressions)
    // to be updated by slicing and transposition
public:
    expression(primitive_type t): type(t) {}

    virtual ~expression() {}

    template<typename T>
    void find( vector<T*> & container );

    primitive_type type;
};

typedef std::shared_ptr<expression> expression_ptr;

template <typename T>
class constant : public expression
{
public:
    constant(const T & v): expression(expr_type<T>::type), value(v) {}
    T value;
};

class primitive_expr : public expression
{
public:
    primitive_expr(primitive_type t): expression(t) {}
    primitive_expr(primitive_type t, primitive_op k, const vector<expression_ptr> & operands):
        expression(t),
        op(k), operands(operands)
    {}

    primitive_op op;
    vector<expression_ptr> operands;
};

class iterator_access : public expression
{
public:
    iterator_access(primitive_type t): expression(t) {}
    iterator_access(primitive_type t, int dimension, int offset=0, int ratio=1):
        expression(t),
        dimension(dimension),
        offset(offset),
        ratio(ratio)
    {}
    mapping relation;
    // TODO: remove the following, use "relation" instead:
    int dimension;
    int offset;
    int ratio;
};

class input_access : public expression
{
public:
    input_access(primitive_type t, int index): expression(t), index(index) {}
    int index;
};

class array
{
public:
    array(const string & name, primitive_type & t):
        name(name),
        type(t)
    {}

    string name;
    primitive_type type;
    vector<int> size;
    vector<int> buffer_size;
    int flow_dim = -1;
    int period = 0;
    int period_offset = 0;
    bool inter_period_dependency = true;
};

typedef std::shared_ptr<array> array_ptr;

class statement
{
public:
    statement() {}
    statement(const string & name): name(name) {}

    string name;
    expression_ptr expr = nullptr;
    vector<int> domain;

    array_ptr array;
    mapping write_relation;

    int flow_dim = -1;

    vector<int> infinite_dimensions()
    {
        vector<int> dimensions;
        for (int dim = 0; dim < domain.size(); ++dim)
            if (domain[dim] == infinite)
                dimensions.push_back(dim);
        return dimensions;
    }
};

typedef std::shared_ptr<statement> statement_ptr;

class array_access : public expression
{
public:
    array_ptr target;
    mapping pattern;

    array_access(array_ptr target):
        expression(target->type),
        target(target)
    {}
    array_access(array_ptr target, const mapping & relation):
        expression(target->type),
        target(target),
        pattern(relation)
    {}
};

template<typename T> inline
void expression::find( vector<T*> & container )
{
    if (auto node = dynamic_cast<T*>(this))
    {
        container.push_back(node);
    }

    if (auto operation = dynamic_cast<primitive_expr*>(this))
    {
        for (auto sub_expr : operation->operands)
            sub_expr->find<T>(container);
        return;
    }
}

struct debug : public stream::debug::topic<debug, stream::debug::all>
{ static string id() { return "polyhedral"; } };


class model
{
public:
    vector<array_ptr> arrays;
    vector<statement_ptr> statements;
};

} // namespace polyhedral
} // namespace stream

#endif // STREAM_POLYHEDRAL_MODEL_INCLUDED
