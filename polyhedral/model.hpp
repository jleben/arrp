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

#include <vector>
#include <string>
#include <iostream>
#include "../utility/matrix.hpp"
#include "../utility/debug.hpp"

namespace stream {
namespace polyhedral {

using std::vector;
using std::string;
using utility::matrix;

enum
{
    infinite = -1
};

enum numerical_type
{
    integer,
    real
};

template <numerical_type> struct cpp_type;
template<> struct cpp_type<integer> { typedef int type; };
template<> struct cpp_type<real> { typedef double type; };

template <typename T> struct expr_type;
template<> struct expr_type<int> { static constexpr numerical_type type = integer; };
template<> struct expr_type<double> { static constexpr numerical_type type = real; };

class statement;

class mapping
{
public:
    mapping() {}

    mapping(const matrix<int> & coef, const vector<int> & cst):
        coefficients(coef), constants(cst)
    {
        assert(coefficients.rows() == constants.size());
    }

    mapping(int input_dimension, int output_dimension):
        coefficients(output_dimension, input_dimension, 0),
        constants(output_dimension, 0)
    {
    }

    static mapping identity(int in, int out)
    {
        mapping m;
        m.coefficients = matrix<int>::identity(out,in);
        m.constants.resize(out, 0);
        return m;
    }

    bool operator== ( const mapping & other ) const
    {
        return coefficients == other.coefficients && constants == other.constants;
    }

    int coefficient(int in_dim, int out_dim) const
    {
        return coefficients(out_dim, in_dim);
    }
    int & coefficient(int in_dim, int out_dim)
    {
        return coefficients(out_dim, in_dim);
    }

    int constant(int out_dim) const
    {
        return constants[out_dim];
    }
    int & constant(int out_dim)
    {
        return constants[out_dim];
    }

    int input_dimension() const { return coefficients.columns(); }
    int output_dimension() const { return coefficients.rows(); }

    void resize(int in_dim, int out_dim)
    {
        coefficients = coefficients.resized(out_dim, in_dim);
        constants.resize(out_dim, 0);
    }

    matrix<int> coefficients;
    vector<int> constants;
};

inline mapping operator*(const mapping & m1, const mapping & m2)
{
    using namespace ::stream::utility;

    mapping dst;
    dst.coefficients = m1.coefficients * m2.coefficients;
    dst.constants = m1.coefficients * m2.constants + m1.constants;
    return dst;
}

inline vector<int> operator*(const mapping & m, const vector<int> & v)
{
    using namespace ::stream::utility;

    return m.coefficients * v + m.constants;
}

std::ostream & operator<<(std::ostream &, const mapping & m);

class expression
{
    // TODO: provide easy access to all dependencies,
    // (of all sub-expressions)
    // to be updated by slicing and transposition
public:
    expression(numerical_type t): type(t) {}

    virtual ~expression() {}

    template<typename T>
    void find( vector<T*> & container );

    numerical_type type;
};

template <typename T>
class constant : public expression
{
public:
    constant(const T & v): expression(expr_type<T>::type), value(v) {}
    T value;
};

class intrinsic : public expression
{
public:
    enum of_kind
    {
        negate,
        add,
        subtract,
        multiply,
        divide,
        divide_integer,
        raise,
        exp,
        exp2,
        log,
        log2,
        log10,
        sqrt,
        sin,
        cos,
        tan,
        asin,
        acos,
        atan,
        ceil,
        floor,
        abs,
        min,
        max,

        count
    };

    intrinsic(numerical_type t): expression(t) {}
    intrinsic(numerical_type t, of_kind k, const vector<expression*> & operands):
        expression(t),
        kind(k), operands(operands)
    {}

    of_kind kind;
    vector<expression*> operands;
};

class iterator_access : public expression
{
public:
    iterator_access(numerical_type t): expression(t) {}
    iterator_access(numerical_type t, int dimension, int offset=0, int ratio=1):
        expression(t),
        dimension(dimension),
        offset(offset),
        ratio(ratio)
    {}
    int dimension;
    int offset;
    int ratio;
};

class input_access : public expression
{
public:
    input_access(numerical_type t, int index): expression(t), index(index) {}
    int index;
};

class statement
{
public:
    statement(): expr(nullptr), inter_period_dependency(true) {}
    string name;
    expression * expr;
    vector<int> domain;
    vector<int> buffer;
    bool inter_period_dependency;

    vector<int> infinite_dimensions()
    {
        vector<int> dimensions;
        for (int dim = 0; dim < domain.size(); ++dim)
            if (domain[dim] == infinite)
                dimensions.push_back(dim);
        return dimensions;
    }
};

class stmt_access : public expression
{
public:
    stmt_access(statement *target):
        expression(target->expr->type),
        target(target) {}
    statement * target;
    mapping pattern;
};

class reduction_access : public expression
{
public:
    reduction_access(numerical_type t): expression(t) {}
    statement * initializer;
    statement * reductor;
};

template<typename T> inline
void expression::find( vector<T*> & container )
{
    if (auto node = dynamic_cast<T*>(this))
    {
        container.push_back(node);
    }

    if (auto operation = dynamic_cast<intrinsic*>(this))
    {
        for (auto sub_expr : operation->operands)
            sub_expr->find<T>(container);
        return;
    }
}

struct debug : public stream::debug::topic<debug, stream::debug::all>
{ static string id() { return "polyhedral"; } };

} // namespace polyhedral
} // namespace stream

#endif // STREAM_POLYHEDRAL_MODEL_INCLUDED
