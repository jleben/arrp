#ifndef STREAM_POLYHEDRAL_MODEL_INCLUDED
#define STREAM_POLYHEDRAL_MODEL_INCLUDED

#include <vector>
#include "../utility/matrix.hpp"

namespace stream {
namespace polyhedral {

using std::vector;
using utility::matrix;

enum
{
    infinite = -1
};

class statement;

class access_pattern
{
public:
    access_pattern(int from, int to, int at, int by):
        source_dimension(from),
        target_dimension(to),
        offset(at),
        stride(by)
    {}
    int source_dimension;
    int target_dimension;
    int offset;
    int stride;
};

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

    int input_dimension() const { return coefficients.columns(); }
    int output_dimension() const { return coefficients.rows(); }

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

class expression
{
    // TODO: provide easy access to all dependencies,
    // (of all sub-expressions)
    // to be updated by slicing and transposition
public:
    virtual ~expression() {}
};

template <typename T>
class constant : public expression
{
public:
    constant(const T & v): value(v) {}
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
        raise,
        exp,
        log,
        sqrt
    };

    of_kind kind;
    vector<expression*> operands;
};

class stream_access : public expression
{
public:
    stream_access(): target(nullptr) {}
    statement * target;
    mapping pattern;
};

class statement
{
public:
    statement(): expr(nullptr) {}
    vector<int> domain;
    expression * expr;
};

}
}

#endif // STREAM_POLYHEDRAL_MODEL_INCLUDED
