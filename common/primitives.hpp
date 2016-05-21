#ifndef STREAM_LANG_INTRINSICS_INCLUDED
#define STREAM_LANG_INTRINSICS_INCLUDED

#include <iostream>
#include <string>
#include <vector>

namespace stream {

using std::string;
using std::ostream;
using std::vector;

enum class primitive_type
{
    undefined,
    boolean,
    integer,
    real,
    complex,
};

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

    real,
    imag,

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

struct prim_op_overload
{
    prim_op_overload() {}
    prim_op_overload(std::initializer_list<primitive_type> l):
        types(l) {}
    vector<primitive_type> types;
};

vector<prim_op_overload> overloads(primitive_op);

struct ambiguous_type {};
struct no_type {};

primitive_type result_type(primitive_op, vector<primitive_type> & args);

primitive_type common_type(primitive_type, primitive_type);

primitive_type common_type(const vector<primitive_type> & types);

inline ostream & operator<<(ostream & s, primitive_op op)
{
    s << name_of_primitive(op);
    return s;
}

inline ostream & operator<<(ostream & s, primitive_type t)
{
    switch(t)
    {
    case primitive_type::boolean:
        s << "boolean"; break;
    case primitive_type::integer:
        s << "integer"; break;
    case primitive_type::real:
        s << "real"; break;
    case primitive_type::complex:
        s << "complex"; break;
    default:
        s << "?";
    }
    return s;
}

}

#endif // STREAM_LANG_INTRINSICS_INCLUDED
