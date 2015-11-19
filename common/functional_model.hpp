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

#include "../common/primitives.hpp"
#include "../frontend/location.hh"

#include <memory>
#include <vector>

namespace stream {
namespace functional {

using std::vector;
typedef parsing::location location_type;

class var
{
public:
    virtual ~var() {}
};

typedef std::shared_ptr<var> var_ptr;

class expression
{
public:
    expression() {}
    expression(const location_type & loc): location(loc) {}
    virtual ~expression() {}
    location_type location;
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
        type(t), operands(operands) {}

    primitive_op type;
    vector<expr_ptr> operands;
};

class array_var : public var
{
public:
    enum { unconstrained = -1 };
    array_var() {}
    array_var(expr_ptr range): range(range) {}
    expr_ptr range;
};
typedef std::shared_ptr<array_var> array_var_ptr;

class array_def : public expression
{
public:
    vector<array_var_ptr> vars;
    expr_ptr expr;
};

class array_var_ref : public expression
{
public:
    array_var_ref(array_var_ptr v, const location_type & loc):
        expression(loc), var(v) {}
    array_var_ptr var;
};

class array_app : public expression
{
public:
    expr_ptr object;
    vector<expr_ptr> args;
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
    func_var(const string & name): name(name) {}
    string name;
};
typedef std::shared_ptr<func_var> func_var_ptr;

class func_def;
typedef std::shared_ptr<func_def> func_def_ptr;

class func_def
{
public:
    string name;
    location_type location;
    vector<func_var_ptr> vars;
    vector<func_def_ptr> defs;
    expr_ptr expr;
};

class func_var_ref : public expression
{
public:
    func_var_ref(func_var_ptr v, const location_type & loc):
        expression(loc), var(v) {}
    func_var_ptr var;
};

class func_ref : public expression
{
public:
    func_ref() {}
    func_ref(func_def_ptr func, const location_type & loc):
        expression(loc), func(func) {}
    func_def_ptr func;
};

}
}

#endif // STREAM_FUNCTIONAL_MODEL_INCLUDED

