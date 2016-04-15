/*
Compiler for language for stream processing

Copyright (C) 2015  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_FUNCTIONAL_MODEL_INCLUDED
#define STREAM_FUNCTIONAL_MODEL_INCLUDED

#include "primitives.hpp"
#include "linear_algebra.hpp"
#include "../frontend/location.hh"
#include <memory>
#include <vector>
#include <utility>
#include <iostream>

namespace stream {
namespace functional {

struct model; // to control verbose output

using std::vector;
using std::pair;
typedef parsing::location location_type;

class var
{
public:
    var() {}
    var(const string & name, const location_type & loc):
        name(name), location(loc)
    {}
    virtual ~var() {}
    string name;
    location_type location;
};

typedef std::shared_ptr<var> var_ptr;

inline std::ostream & operator<<(std::ostream & s, const var* v)
{
    if (v)
        s << "'" << v->name << "' ";
    s << '(' << (void*)v << ')';
    return s;
}

class expression
{
public:
    expression() {}
    expression(const location_type & loc): location(loc) {}
    virtual ~expression() {}
    location_type location;
    primitive_type type = primitive_type::undefined;
};
typedef std::shared_ptr<expression> expr_ptr;

template <typename T>
class constant : public expression
{
public:
    constant(const T & v): value(v) {}
    constant(const T & v, const location_type & loc):
        expression(loc), value(v) {}
    T value;
};

class primitive : public expression
{
public:
    primitive() {}
    primitive(primitive_op t,
              const vector<expr_ptr> & operands):
        kind(t), operands(operands) {}
    template<typename ...Ts>
    primitive(primitive_op t,
              Ts ... operands):
        kind(t), operands({ operands ... }) {}

    primitive_op kind;
    vector<expr_ptr> operands;
};

class case_expr : public expression
{
public:
    vector<pair<expr_ptr,expr_ptr>> cases;
};

class array_var : public var
{
public:
    enum { unconstrained = -1 };
    array_var() {}
    array_var(const string & name, expr_ptr range,
              const location_type & loc):
        var(name, loc), range(range) {}
    expr_ptr range;
};
typedef std::shared_ptr<array_var> array_var_ptr;

class array : public expression
{
public:
    vector<array_var_ptr> vars;
    expr_ptr expr;
    bool is_recursive = false;
};
typedef std::shared_ptr<array> array_ptr;

class array_app : public expression
{
public:
    expr_ptr object;
    vector<expr_ptr> args;
};

class array_size : public expression
{
public:
    array_size() {}
    array_size(expr_ptr o, expr_ptr d, const location_type & l):
        expression(l), object(o), dimension(d) {}
    expr_ptr object;
    expr_ptr dimension;
};

class func_app : public expression
{
public:
    expr_ptr object;
    vector<expr_ptr> args;
};

class func_var : public var
{
public:
    func_var() {}
    func_var(const string & name, const location_type & loc): var(name,loc) {}
};
typedef std::shared_ptr<func_var> func_var_ptr;

class identifier : public var
{
public:
    identifier(const string & name, expr_ptr e, const location_type & loc):
        var(name,loc), expr(e) {}
    expr_ptr expr;
    primitive_type type = primitive_type::undefined; // FIXME: Could be more elegant?
};
typedef std::shared_ptr<identifier> id_ptr;


class reference : public expression
{
public:
    reference(var_ptr v, const location_type & loc):
        expression(loc), var(v) {}
    var_ptr var;
};

class array_self_ref : public expression
{
public:
    array_self_ref(array_ptr arr, const location_type & loc):
        expression(loc), arr(arr) {}
    array_ptr arr;
};

class function : public expression
{
public:
    function() {}
    function(const vector<func_var_ptr> & v, expr_ptr e, const location_type & loc):
        expression(loc), vars(v), expr(e) {}
    vector<func_var_ptr> vars;
    expr_ptr expr;
};

class expr_scope : public expression
{
public:
    expr_scope() {}
    expr_scope(const vector<id_ptr> & ids, expr_ptr e, const location_type & loc):
        expression(loc), ids(ids), expr(e){}
    vector<id_ptr> ids;
    expr_ptr expr;
};

class affine_expr : public expression
{
public:
    affine_expr() {}
    affine_expr(const linexpr & e, const location_type & loc = location_type()):
        expression(loc), expr(e) {}
    linexpr expr;
};

class affine_set : public expression
{
public:
    affine_set() {}
    affine_set(const linear_set & s): set(s) {}
    linear_set set;
};

}
}

#endif // STREAM_FUNCTIONAL_MODEL_INCLUDED

