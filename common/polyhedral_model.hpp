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

class array_variable
{
public:
    array_variable() {}
    array_variable(int size): size(size) {}
    int size = 0;
};

typedef std::shared_ptr<array_variable> array_var_ptr;

class array_var_vector : public vector<array_var_ptr>
{
public:
    using vector<array_var_ptr>::vector;
    array_var_vector() {}
    array_var_vector(array_var_ptr v):
        vector({v})
    {}
    array_var_vector(vector<int> sizes)
    {
        for(auto & size : sizes)
            push_back(std::make_shared<array_variable>(size));
    }
    template<typename C>
    array_var_vector(const C & container)
    {
        for (auto & var : container)
            push_back(var);
    }
    array_var_vector & operator<< (const array_var_ptr & var)
    {
        push_back(var);
        return *this;
    }
};

class array_index_term
{
public:
    array_index_term() {}
    array_index_term(int s): scalar(s) {}
    array_index_term(array_var_ptr v, int s = 1): var(v), scalar(s) {}
    array_var_ptr var;
    int scalar = 1;
};

class array_index_expr : public vector<array_index_term>
{
public:
    using vector<array_index_term>::vector;
    array_index_expr() {}
    array_index_expr(int c):
        vector<array_index_term>({array_index_term(c)}) {}
    array_index_expr(array_var_ptr v):
        vector<array_index_term>({array_index_term(v)}) {}
    array_index_expr(const array_index_term & i):
        vector<array_index_term>({i}) {}
};

class array_index_vector : public vector<array_index_expr>
{
public:
    using vector<array_index_expr>::vector;
    array_index_vector() {}
    array_index_vector(const array_index_expr & expr):
        vector({expr}) {}
    /*
    array_index_vector(const array_var_vector & vars)
    {
        for (auto & var : vars)
            push_back(var);
    }*/
    template<typename C>
    array_index_vector(const C & container)
    {
        for (auto & var : container)
            push_back(var);
    }
    array_index_vector & operator<< (const array_index_expr & expr)
    {
        push_back(expr);
        return *this;
    }
};

inline array_index_term operator*(array_var_ptr v, int s)
{
    return array_index_term(v,s);
}

inline array_index_expr operator+(array_var_ptr a, int b)
{
    return { array_index_term(a), array_index_term(b) };
}

inline array_index_expr operator+(const array_index_expr & e1,
                                  const array_index_expr & e2)
{
    array_index_expr e(e1);

    for (auto & t2 : e2)
    {
        bool added = false;
        for(auto & t : e)
        {
            if (t2.var == t.var)
            {
                t.scalar += t2.scalar;
                added = true;
                break;
            }
        }
        if (!added)
        {
            e.push_back(t2);
        }
    }

    return e;
}

inline array_index_expr operator*(const array_index_expr & e,
                                  int scalar)
{
    array_index_expr e2;
    for (auto & term : e)
    {
        e2.push_back( array_index_term {term.var, term.scalar * scalar} );
    }
    return e2;
}

class expression
{
public:
    expression(primitive_type t): type(t) {}

    virtual ~expression() {}

    template<typename T>
    void find( vector<T*> & container );

    primitive_type type;
};

typedef std::shared_ptr<expression> expression_ptr;

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
    array_index_vector write_index;
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
    iterator_access(const array_index_expr & e):
        expression(primitive_type::integer),
        expr(e)
    {}

    array_index_expr expr;

    // TODO: remove
    mapping relation;

    // TODO: remove the following, use "relation" instead:
    int dimension;
    int offset;
    int ratio;
};

class input_access : public expression
{
public:
    input_access(primitive_type t, int index):
        expression(t), index(index) {}
    input_access(primitive_type t, int i, const array_index_vector & ai):
        expression(t), index(i), array_index(ai) {}

    int index; // FIXME: rename?
    array_index_vector array_index;
};

class array_access : public expression
{
public:
    array_ptr target;
    array_index_vector index;

    // TODO: remove
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
    array_access(array_ptr a, const array_index_vector & i):
        expression(a->type),
        target(a),
        index(i)
    {}
};

class array_function : public expression
{
public:
    array_function(primitive_type t): expression(t) {}
    array_function(const vector<int> & size, expression_ptr expr):
        expression(expr->type),
        vars(size),
        expr(expr)
    {}
    array_function(array_ptr a):
        expression(a->type),
        vars(a->size)
    {
        expr = std::make_shared<array_access>(a, array_index_vector(vars));
    }
    array_function(const array_var_vector & v, expression_ptr e):
        expression(e->type), vars(v), expr(e) {}

    array_var_vector vars;
    expression_ptr expr;
};

typedef std::shared_ptr<array_function> array_func_ptr;

class array_func_apply : public expression
{
public:
    array_func_apply(expression_ptr f):
        expression(f->type), func(f) {}
    array_func_apply(expression_ptr f, const array_index_vector & a):
        expression(f->type), func(f), args(a) {}

    expression_ptr func;
    array_index_vector args;
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
