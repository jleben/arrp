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
    matrix<int> map;
    vector<int> offset;
    //vector<access_pattern> pattern;
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
