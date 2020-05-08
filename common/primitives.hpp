#ifndef STREAM_LANG_INTRINSICS_INCLUDED
#define STREAM_LANG_INTRINSICS_INCLUDED

#include <iostream>
#include <string>
#include <vector>

namespace stream {

using std::string;
using std::ostream;
using std::vector;

// NOTE: Primitive types:

// The type 'int' in source language is a synonym for
// a signed integer type with an implementation-defined size,
// at least 32 bits.

// Integer literals get a signed integer type by default
// or an unsigned integer type if suffixed by 'u'.
// In both cases, the type has at least the size of 'int',
// or larger if needed to represent the value (up to 64 bits).

enum class primitive_type
{
    // NOTE: Order is important to distinguish type groups

    undefined,

    boolean,

    int8,
    uint8,
    int16,
    uint16,
    int32,
    uint32,
    int64,
    uint64,

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
        pt::int8,
        pt::uint8,
        pt::int16,
        pt::uint16,
        pt::int32,
        pt::uint32,
        pt::int64,
        pt::uint64,
        pt::real32,
        pt::real64,
        pt::complex32,
        pt::complex64,
        pt::infinity
    };
    return types;
}

inline
vector<primitive_type> combine_types(const vector<primitive_type> & a,
                                     const vector<primitive_type> & b)
{
    vector<primitive_type> result;
    result.reserve(a.size() + b.size());
    for (auto & x : a)
        result.push_back(x);
    for (auto & x : b)
        result.push_back(x);
    return result;
}

inline void add_types(vector<primitive_type> & a, const vector<primitive_type> & b)
{
    for (auto & x : b)
        a.push_back(x);
}

inline vector<primitive_type> all_integer_types()
{
    using pt = primitive_type;
    auto types = {
        pt::int8,
        pt::uint8,
        pt::int16,
        pt::uint16,
        pt::int32,
        pt::uint32,
        pt::int64,
        pt::uint64,
    };
    return types;
}

inline vector<primitive_type> all_simple_numeric_types()
{
    using pt = primitive_type;
    return combine_types(all_integer_types(), { pt::real32, pt::real64 });
}

inline vector<primitive_type> all_numeric_types()
{
    using pt = primitive_type;
    vector<pt> types = all_simple_numeric_types();
    types.push_back(pt::complex32);
    types.push_back(pt::complex64);
    return types;
}

inline bool is_real(primitive_type t)
{ return t == primitive_type::real32 or t == primitive_type::real64; }

template<primitive_type> bool is_real() { return false; }
template<> inline bool is_real<primitive_type::real32>() { return true; }
template<> inline bool is_real<primitive_type::real64>() { return true; }

inline
bool is_integer(primitive_type t)
{
    return t >= primitive_type::int8 and t <= primitive_type::uint64;
}
template<primitive_type t> bool is_integer() { return is_integer(t); }

inline
bool is_signed_int(primitive_type t)
{
    using pt = primitive_type;

    switch (t)
    {
    case pt::int8:
    case pt::int16:
    case pt::int32:
    case pt::int64:
        return true;
    default:
        return false;
    }
}
template<primitive_type> bool is_signed_int() { return false; }
template<> inline bool is_signed_int<primitive_type::int8>() { return true; }
template<> inline bool is_signed_int<primitive_type::int16>() { return true; }
template<> inline bool is_signed_int<primitive_type::int32>() { return true; }
template<> inline bool is_signed_int<primitive_type::int64>() { return true; }


inline bool is_complex(primitive_type t)
{ return t == primitive_type::complex32 or t == primitive_type::complex64; }

template<primitive_type> bool is_complex() { return false; }
template<> inline bool is_complex<primitive_type::complex32>() { return true; }
template<> inline bool is_complex<primitive_type::complex64>() { return true; }


inline bool is_numeric(primitive_type t)
{
    return is_integer(t) or is_real(t) or is_complex(t);
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

    to_int,
    to_int8,
    to_uint8,
    to_int16,
    to_uint16,
    to_int32,
    to_uint32,
    to_int64,
    to_uint64,

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

struct ambiguous_type {};
struct no_type {};

primitive_type result_type(primitive_op, vector<primitive_type> & args);

primitive_type common_type(const vector<primitive_type> & types);

bool operator<(primitive_type a, primitive_type b);
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
    case primitive_type::int8:
        s << "int8"; break;
    case primitive_type::uint8:
        s << "uint8"; break;
    case primitive_type::int16:
        s << "int16"; break;
    case primitive_type::uint16:
        s << "uint16"; break;
    case primitive_type::int32:
        s << "int32"; break;
    case primitive_type::uint32:
        s << "uint32"; break;
    case primitive_type::int64:
        s << "int64"; break;
    case primitive_type::uint64:
        s << "uint64"; break;
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
