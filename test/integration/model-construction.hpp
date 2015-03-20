#ifndef STREAM_TESTING_POLY_CONSTRUCTION_INCLUDED
#define STREAM_TESTING_POLY_CONSTRUCTION_INCLUDED

#include "../../common/polyhedral_model.hpp"

namespace stream {
namespace testing {

using polyhedral::expression;
using polyhedral::primitive_expr;
using polyhedral::constant;

struct op
{
    op(expression *e): e(e) {}
    expression *e;

    operator expression*()
    {
        return e;
    }
};

inline op operator+ (op lhs, op rhs)
{
    return op(new primitive_expr(primitive_type::real, primitive_op::add, {lhs.e, rhs.e}));
}

inline op operator- (op lhs, op rhs)
{
    return op(new primitive_expr(primitive_type::real, primitive_op::subtract, {lhs.e, rhs.e}));
}

inline op operator* (op lhs, op rhs)
{
    return op(new primitive_expr(primitive_type::real, primitive_op::multiply, {lhs.e, rhs.e}));
}

inline op operator/ (op lhs, op rhs)
{
    return op(new primitive_expr(primitive_type::real, primitive_op::divide, {lhs.e, rhs.e}));
}

template <typename T>
inline op const_op(T val)
{
    return op(new constant<T>(val));
}


}
}

#endif // STREAM_TESTING_POLY_CONSTRUCTION_INCLUDED
