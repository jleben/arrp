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
    real32,
    real64,
    complex32,
    complex64,
    infinity
};

inline vector<primitive_type> all_real_types()
{
    return { primitive_type::real32, primitive_type::real64 };
}

inline vector<primitive_type> all_complex_types()
{
    return { primitive_type::complex32, primitive_type::complex64 };
}

inline vector<primitive_type> all_primitive_types()
{
    using pt = primitive_type;
    auto types = {
        pt::boolean,
        pt::integer,
        pt::real32,
        pt::real64,
        pt::complex32,
        pt::complex64,
        pt::infinity
    };
    return types;
}

inline vector<primitive_type> all_simple_numeric_types()
{
    using pt = primitive_type;
    auto types = {
        pt::integer,
        pt::real32,
        pt::real64
    };
    return types;
}

inline vector<primitive_type> all_numeric_types()
{
    using pt = primitive_type;
    auto types = {
        pt::integer,
        pt::real32,
        pt::real64,
        pt::complex32,
        pt::complex64
    };
    return types;
}

template<primitive_type> bool is_real() { return false; }
template<> inline bool is_real<primitive_type::real32>() { return true; }
template<> inline bool is_real<primitive_type::real64>() { return true; }

template<primitive_type> bool is_integer() { return false; }
template<> inline bool is_integer<primitive_type::integer>() { return true; }

template<primitive_type> bool is_complex() { return false; }
template<> inline bool is_complex<primitive_type::complex32>() { return true; }
template<> inline bool is_complex<primitive_type::complex64>() { return true; }

inline bool is_complex(primitive_type t)
{
    switch(t) {
    case primitive_type::complex32:
    case primitive_type::complex64:
        return true;
    default:
        return false;
    }
}

primitive_type primitive_type_for_name(const string &);

enum class primitive_op
{
    negate = 0,
    add,
    subtract,
    multiply,
    divide,
    divide_integer,
    modulo,
    bitwise_not,
    bitwise_and,
    bitwise_or,
    bitwise_xor,
    bitwise_lshift,
    bitwise_rshift,
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

    to_integer,
    to_real32,
    to_real64,
    to_complex32,
    to_complex64,

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

bool operator<=(primitive_type a, primitive_type b);

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
        s << "bool"; break;
    case primitive_type::integer:
        s << "int"; break;
    case primitive_type::real32:
        s << "real32"; break;
    case primitive_type::real64:
        s << "real64"; break;
    case primitive_type::complex32:
        s << "complex32"; break;
    case primitive_type::complex64:
        s << "complex64"; break;
    case primitive_type::infinity:
        s << "infinity"; break;
    default:
        s << "?";
    }
    return s;
}

}

#endif // STREAM_LANG_INTRINSICS_INCLUDED
