#ifndef STREAM_LANG_INTRINSICS_INCLUDED
#define STREAM_LANG_INTRINSICS_INCLUDED

#include <iostream>
#include <string>

namespace stream {

using std::string;
using std::ostream;

enum class primitive_op
{
    negate = 0,
    add,
    subtract,
    multiply,
    divide,
    divide_integer,
    modulo,
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

    compare_eq,
    compare_neq,
    compare_l,
    compare_g,
    compare_leq,
    compare_geq,

    logic_and,
    logic_or,

    conditional,

    // Total number of primitive_ops:
    //count
};

string name_of_primitive( primitive_op op );

inline ostream & operator<<(ostream & s, primitive_op op)
{
    s << name_of_primitive(op);
    return s;
}

}

#endif // STREAM_LANG_INTRINSICS_INCLUDED
