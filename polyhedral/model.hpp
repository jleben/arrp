#ifndef STREAM_POLYHEDRAL_MODEL_INCLUDED
#define STREAM_POLYHEDRAL_MODEL_INCLUDED

#include <vector>
#include <string>
#include <iostream>
#include "../utility/matrix.hpp"

namespace stream {
namespace polyhedral {

using std::vector;
using std::string;
using utility::matrix;

enum
{
    infinite = -1
};

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
    virtual ~expression() {}

    template<typename T>
    void find( vector<T*> & container );
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

class reduction_access : public expression
{
public:
    statement * initializer;
    statement * reductor;
};

class input_access : public expression
{
public:
    input_access(int index): index(index) {}
    int index;
};

class statement
{
public:
    statement(): expr(nullptr) {}
    string name;
    vector<int> domain;
    vector<int> buffer;
    expression * expr;

    vector<int> infinite_dimensions()
    {
        vector<int> dimensions;
        for (int dim = 0; dim < domain.size(); ++dim)
            if (domain[dim] == infinite)
                dimensions.push_back(dim);
        return dimensions;
    }
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

}
}

#endif // STREAM_POLYHEDRAL_MODEL_INCLUDED
