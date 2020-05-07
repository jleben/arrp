#include "primitives.hpp"
#include "../common/error.hpp"

#include <numeric>
#include <sstream>
#include <unordered_map>
#include <algorithm>

using namespace std;

namespace stream {

primitive_type primitive_type_for_name(const string & name)
{
    static unordered_map<string, primitive_type> map =
    {
        { "bool", primitive_type::boolean },
        { "int", primitive_type::int32 },
        { "int8", primitive_type::int8 },
        { "uint8", primitive_type::uint8 },
        { "int16", primitive_type::int16 },
        { "uint16", primitive_type::uint16 },
        { "int32", primitive_type::int32 },
        { "uint32", primitive_type::uint32 },
        { "int64", primitive_type::int64 },
        { "uint64", primitive_type::uint64 },

        { "real", primitive_type::real64 },
        { "real32", primitive_type::real32 },
        { "real64", primitive_type::real64 },
        { "complex32", primitive_type::complex32 },
        { "complex64", primitive_type::complex64 },
    };

    auto m = map.find(name);
    if (m == map.end())
        return primitive_type::undefined;
    else
        return m->second;
}

string name_of_primitive( primitive_op op )
{
    switch(op)
    {
    case primitive_op::negate:
        return "!";
    case primitive_op::add:
        return "+";
    case primitive_op::subtract:
        return "-";
    case primitive_op::multiply:
        return "*";
    case primitive_op::divide:
        return "/";
    case primitive_op::divide_integer:
        return ":";
    case primitive_op::modulo:
        return "%";
    case primitive_op::bitwise_not:
        return ".not";
    case primitive_op::bitwise_and:
        return ".and";
    case primitive_op::bitwise_or:
        return ".or";
    case primitive_op::bitwise_xor:
        return ".xor";
    case primitive_op::bitwise_lshift:
        return ".<<";
    case primitive_op::bitwise_rshift:
        return ".>>";
    case primitive_op::raise:
        return "^";
    case primitive_op::exp:
        return "exp";
    case primitive_op::exp2:
        return "exp2";
    case primitive_op::log:
        return "log";
    case primitive_op::log2:
        return "log2";
    case primitive_op::log10:
        return "log10";
    case primitive_op::sqrt:
        return "sqrt";
    case primitive_op::sin:
        return "sin";
    case primitive_op::cos:
        return "cos";
    case primitive_op::tan:
        return "tan";
    case primitive_op::asin:
        return "asin";
    case primitive_op::acos:
        return "acos";
    case primitive_op::atan:
        return "atan";
    case primitive_op::ceil:
        return "ceil";
    case primitive_op::floor:
        return "floor";
    case primitive_op::abs:
        return "abs";
    case primitive_op::min:
        return "min";
    case primitive_op::max:
        return "max";
    case primitive_op::real:
        return "real";
    case primitive_op::imag:
        return "imag";
    case primitive_op::to_real32:
        return "real32";
    case primitive_op::to_int:
        return "int";
    case primitive_op::to_int8:
        return "int8";
    case primitive_op::to_uint8:
        return "uint8";
    case primitive_op::to_int16:
        return "int16";
    case primitive_op::to_uint16:
        return "uint16";
    case primitive_op::to_int32:
        return "int32";
    case primitive_op::to_uint32:
        return "uint32";
    case primitive_op::to_int64:
        return "int64";
    case primitive_op::to_uint64:
        return "uint64";
    case primitive_op::to_real64:
        return "real64";
    case primitive_op::to_complex32:
        return "complex32";
    case primitive_op::to_complex64:
        return "complex64";
    case primitive_op::compare_eq:
        return "==";
    case primitive_op::compare_neq:
        return "!=";
    case primitive_op::compare_l:
        return "<";
    case primitive_op::compare_g:
        return ">";
    case primitive_op::compare_leq:
        return "<=";
    case primitive_op::compare_geq:
        return ">=";
    case primitive_op::logic_and:
        return "&&";
    case primitive_op::logic_or:
        return "||";
    case primitive_op::conditional:
        return "if";
    default:
        return "<unknown primitive op>";
    }
}

static
primitive_type map_type(primitive_type in,
                        const unordered_map<primitive_type, primitive_type> & map)
{
    try
    {
        return map.at(in);
    }
    catch(std::out_of_range&)
    {
        throw no_type();
    }
}

static
primitive_type common_integer(const vector<primitive_type> & types)
{
    auto ct = common_type(types);
    if (is_integer(ct))
        return ct;
    throw no_type();
}

static
primitive_type num_conversion_result(primitive_type requested_type, vector<primitive_type> & args)
{
    if (args.size() == 1 and (is_numeric(args[0])))
        return requested_type;
    else
        throw no_type();
}

static
void check_num_args(vector<primitive_type> & t, int num)
{
    if (t.size() != num)
        throw no_type();
}

primitive_type result_type(primitive_op op, vector<primitive_type> & args)
{
    using pt = primitive_type;

    {
        bool all_args_undefined =
                std::all_of(args.begin(), args.end(),
                            [](pt t){ return t == pt::undefined; });

        if (all_args_undefined)
            return pt::undefined;
    }

    switch(op)
    {
    case primitive_op::negate:
    {
        check_num_args(args, 1);
        return map_type(args[0], {
            { pt::int8, pt::int8 },
            { pt::uint8, pt::int8 },
            { pt::int16, pt::int16 },
            { pt::uint16, pt::int16 },
            { pt::int32, pt::int32 },
            { pt::uint32, pt::int32 },
            { pt::int64, pt::int64 },
            { pt::uint64, pt::int64 },
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 },
            { pt::complex32, pt::complex32 },
            { pt::complex64, pt::complex64 },
            { pt::boolean, pt::boolean }
        });
    }
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_numeric(ct))
            return ct;
        break;
    }
    case primitive_op::divide:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_integer(ct))
            return pt::real64;
        if (is_real(ct) or is_complex(ct))
            return ct;
        break;
    }
    case primitive_op::divide_integer:
    case primitive_op::modulo:
    case primitive_op::bitwise_not:
    case primitive_op::bitwise_and:
    case primitive_op::bitwise_or:
    case primitive_op::bitwise_xor:
    case primitive_op::bitwise_lshift:
    case primitive_op::bitwise_rshift:
    {
        check_num_args(args, 2);
        return common_integer(args);
    }
    case primitive_op::raise:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_numeric(ct))
            return ct;
        break;
    }
    case primitive_op::exp:
    case primitive_op::log:
    case primitive_op::log10:
    case primitive_op::sqrt:
    case primitive_op::sin:
    case primitive_op::cos:
    case primitive_op::tan:
    case primitive_op::asin:
    case primitive_op::acos:
    case primitive_op::atan:
    {
        check_num_args(args, 1);
        auto & a = args[0];
        if (is_integer(a))
            return pt::real64;
        if (is_real(a) or is_complex(a))
            return a;
        break;
    }
    case primitive_op::log2:
    {
        check_num_args(args, 1);
        auto & a = args[0];
        if (is_integer(a))
            return pt::real64;
        if (is_real(a))
            return a;
        break;
    }
    case primitive_op::exp2:
    {
        check_num_args(args, 1);
        auto & a = args[0];
        if (is_integer(a) or is_real(a))
            return a;
    }
    case primitive_op::ceil:
    case primitive_op::floor:
    case primitive_op::abs:
    {
        check_num_args(args, 1);
        auto & a = args[0];
        if (is_integer(a) or is_real(a))
            return a;
        break;
    }
    case primitive_op::min:
    case primitive_op::max:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_integer(ct) or is_real(ct))
            return ct;
        break;
    }
    case primitive_op::real:
    case primitive_op::imag:
    {
        check_num_args(args, 1);
        auto & a = args[0];
        if (a == pt::complex32)
            return pt::real32;
        if (a == pt::complex64)
            return pt::complex64;
        break;
    }
    case primitive_op::to_int8:
        return num_conversion_result(pt::int8, args);
    case primitive_op::to_uint8:
        return num_conversion_result(pt::uint8, args);
    case primitive_op::to_int16:
        return num_conversion_result(pt::int16, args);
    case primitive_op::to_uint16:
        return num_conversion_result(pt::uint16, args);
    case primitive_op::to_int:
    case primitive_op::to_int32:
        return num_conversion_result(pt::int32, args);
    case primitive_op::to_uint32:
        return num_conversion_result(pt::uint32, args);
    case primitive_op::to_int64:
        return num_conversion_result(pt::int64, args);
    case primitive_op::to_uint64:
        return num_conversion_result(pt::uint64, args);

    case primitive_op::to_real32:
        return num_conversion_result(pt::real32, args);
    case primitive_op::to_real64:
        return num_conversion_result(pt::real64, args);

    case primitive_op::to_complex32:
        return num_conversion_result(pt::complex32, args);
    case primitive_op::to_complex64:
        return num_conversion_result(pt::complex64, args);

    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_numeric(ct) or ct == pt::boolean)
            return pt::boolean;
        break;
    }
    case primitive_op::compare_l:
    case primitive_op::compare_g:
    case primitive_op::compare_leq:
    case primitive_op::compare_geq:
    {
        check_num_args(args, 2);
        auto ct = common_type(args);
        if (is_integer(ct) or is_real(ct))
            return pt::boolean;
        break;
    }
    case primitive_op::logic_and:
    case primitive_op::logic_or:
    {
        check_num_args(args, 2);
        if (args[0] == pt::boolean and args[1] == pt::boolean)
            return pt::boolean;
        break;
    }
    case primitive_op::conditional:
    {
        check_num_args(args, 3);
        if (args[0] == pt::boolean)
            return common_type( { args[1], args[2] } );
        break;
    }
    default:;
    }

    throw no_type();
}

primitive_type common_type(const vector<primitive_type> & types)
{
    if (types.empty())
        throw no_type();

    // Since types form only a partial order,
    // the top might not be absolute top
    primitive_type top = *std::max_element(types.begin(), types.end());

    // Check whether top is absolute
    for (auto & t : types)
    {
        if (not (t <= top))
            throw no_type();
    }

    return top;
}

bool operator<(primitive_type a, primitive_type b)
{
    using pt = primitive_type;

    if (a == pt::undefined)
        return true;

    switch(b)
    {
    case pt::infinity:
        return false;
    case pt::boolean:
        return false;
    case pt::complex64:
    {
        for (auto & t : all_integer_types())
            if (a == t)
                return true;
        return a == pt::real32 || a == pt::real64;
    }
    case pt::complex32:
    {
        for (auto & t : all_integer_types())
            if (a == t)
                return true;
        return a == pt::real32;
    }
    case pt::real64:
    {
        for (auto & t : all_integer_types())
            if (a == t)
                return true;
        return a == pt::real32;
    }
    case pt::real32:
    {
        for (auto & t : all_integer_types())
            if (a == t)
                return true;
        return false;
    }
    default:
        return (is_signed_int(b) == is_signed_int(a) and int(a) < int(b));
    }
}

bool operator<=(primitive_type a, primitive_type b)
{
    return a == b or a < b;
}

}
