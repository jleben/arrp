#include "primitives.hpp"
#include "../common/error.hpp"

#include <numeric>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace stream {

primitive_type primitive_type_for_name(const string & name)
{
    static unordered_map<string, primitive_type> map =
    {
        { "bool", primitive_type::boolean },
        { "int", primitive_type::integer },
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
    case primitive_op::to_integer:
        return "int";
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

vector<prim_op_overload> overloads(primitive_op op)
{
    using pt = primitive_type;

    vector<prim_op_overload> overloads;

    switch(op)
    {
    case primitive_op::negate:
        return {
            { pt::integer, pt::integer },
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 },
            { pt::complex32, pt::complex32 },
            { pt::complex64, pt::complex64 },
            { pt::boolean, pt::boolean }
        };
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real32, pt::real32, pt::real32 },
            { pt::real64, pt::real64, pt::real64 },
            { pt::complex32, pt::complex32, pt::complex32 },
            { pt::complex64, pt::complex64, pt::complex64 }
        };
    case primitive_op::divide:
        return {
            { pt::real32, pt::real32, pt::real32 },
            { pt::real64, pt::real64, pt::real64 },
            { pt::complex32, pt::complex32, pt::complex32 },
            { pt::complex64, pt::complex64, pt::complex64 }
        };
    case primitive_op::divide_integer:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real32, pt::real32, pt::integer },
            { pt::real64, pt::real64, pt::integer }
        };
    case primitive_op::modulo:
        return {
            { pt::integer, pt::integer, pt::integer }
        };
    case primitive_op::bitwise_not:
        return {
            { pt::integer, pt::integer }
        };
    case primitive_op::bitwise_and:
    case primitive_op::bitwise_or:
    case primitive_op::bitwise_xor:
    case primitive_op::bitwise_lshift:
    case primitive_op::bitwise_rshift:
        return {
            { pt::integer, pt::integer, pt::integer }
        };
    case primitive_op::raise:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real32, pt::real32, pt::real32 },
            { pt::real64, pt::real64, pt::real64 },
        };
    case primitive_op::exp:
    case primitive_op::log:
    case primitive_op::log10:
    case primitive_op::sin:
    case primitive_op::cos:
    case primitive_op::tan:
    case primitive_op::asin:
    case primitive_op::acos:
    case primitive_op::atan:
    case primitive_op::sqrt:
        return {
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 },
            { pt::complex32, pt::complex32 },
            { pt::complex64, pt::complex64 }
        };
    case primitive_op::log2:
        return {
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 }
        };
    case primitive_op::exp2:
        return {
            { pt::integer, pt::integer },
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 },
        };
    case primitive_op::ceil:
    case primitive_op::floor:
        return {
            { pt::integer, pt::integer },
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 }
        };
    case primitive_op::abs:
        return {
            { pt::integer, pt::integer },
            { pt::real32, pt::real32 },
            { pt::real64, pt::real64 }
        };
    case primitive_op::min:
    case primitive_op::max:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real32, pt::real32, pt::real32 },
            { pt::real64, pt::real64, pt::real64 }
        };
    case primitive_op::real:
    case primitive_op::imag:
        return {
            { pt::complex32, pt::real32 },
            { pt::complex64, pt::real64 },
        };
    case primitive_op::to_integer:
        for (auto t : all_simple_numeric_types())
            overloads.push_back({ t, pt::integer });
        break;
    case primitive_op::to_real32:
        for (auto t : all_simple_numeric_types())
            overloads.push_back({ t, pt::real32 });
        break;
    case primitive_op::to_real64:
        for (auto t : all_simple_numeric_types())
            overloads.push_back({ t, pt::real64 });
        break;
    case primitive_op::to_complex32:
        for (auto t : all_numeric_types())
            overloads.push_back({ t, pt::complex32 });
        break;
    case primitive_op::to_complex64:
        for (auto t : all_numeric_types())
            overloads.push_back({ t, pt::complex64 });
        break;
    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
        return {
            { pt::integer, pt::integer, pt::boolean },
            { pt::real32, pt::real32, pt::boolean },
            { pt::real64, pt::real64, pt::boolean },
            { pt::complex32, pt::complex32, pt::boolean },
            { pt::complex64, pt::complex64, pt::boolean },
        };
    case primitive_op::compare_l:
    case primitive_op::compare_g:
    case primitive_op::compare_leq:
    case primitive_op::compare_geq:
        return {
            { pt::integer, pt::integer, pt::boolean },
            { pt::real32, pt::real32, pt::boolean },
            { pt::real64, pt::real64, pt::boolean }
        };
    case primitive_op::logic_and:
    case primitive_op::logic_or:
        return { { pt::boolean, pt::boolean, pt::boolean } };
    case primitive_op::conditional:
    {
        for (auto t : all_primitive_types())
            overloads.push_back(prim_op_overload({ pt::boolean, t, t, t }));
        break;
    }
    default:;
    }

    return overloads;
}

primitive_type result_type(primitive_op op, vector<primitive_type> & args)
{
    // FIXME: we should promote (int/int) to (f64/f64), not (f32/f32),
    // but still (int/f32) to (f32/f32).

    using type = primitive_type;
#if 0
    cout << "Computing result type for: " << op;
    for (auto & arg : args)
        cout << " " << arg;
    cout << endl;
#endif
    auto candidates = overloads(op);
    if (candidates.empty())
    {
        ostringstream msg;
        msg << "No overloads for op: " << op;
        throw error(msg.str());
    }

    // If any arg is undefined,
    // then if all overloads return the same type, return that type,
    // else return undefined.
    for (auto & arg : args)
    {
        if (arg == type::undefined)
        {
            auto t = candidates.front().types.back();
            for(int c = 1; c < candidates.size(); ++c)
            {
                if (candidates[c].types.back() != t)
                    return type::undefined;
            }
            return t;
        }
    }

    vector<int> promoted;

    for (int c = 0; c < candidates.size(); ++c)
    {
        if (args.size() != candidates[c].types.size() -1)
            continue;

        bool is_valid = true;
        bool is_exact = true;
        for (int i = 0; i < args.size(); ++i)
        {
            auto pt = candidates[c].types[i];
            auto at = args[i];
            if (pt == at)
                continue;
            is_exact = false;
            is_valid = at <= pt;
            if (!is_valid)
                break;
        }
        if (is_exact)
        {
            return candidates[c].types.back();
        }
        if (is_valid)
        {
            promoted.push_back(c);
        }
    }

    if (promoted.empty())
    {
        throw no_type();
    }

    if (promoted.size() == 1)
    {
        return candidates[promoted.front()].types.back();
    }

#if 0
    cout << "Multiple promotions for " << op;
    for(auto & a : args)
        cout << " " << a;
    cout << " = " << promoted.size() << endl;
    for (auto & c : promoted)
    {
        for (int p = 0; p < candidates[c].types.size() - 1; ++p)
        {
            cout << candidates[c].types[p] << " ";
        }
        cout << "-> " << candidates[c].types.back();
        cout << endl;
    }
#endif

    // Check if one overload is the singular bottom of their order

    vector<int> lowest_promoted;

    // FIXME: optimize:
    for (auto i : promoted)
    {
        bool ok = true;

        for(auto j : promoted)
        {
            if (j == i)
                continue;

            for (int k = 0; ok && k < candidates[i].types.size()-1; ++k)
            {
                auto a = candidates[i].types[k];
                auto b = candidates[j].types[k];
                ok = a <= b;
            }

            if (!ok)
                break;
        }

        if (ok)
            lowest_promoted.push_back(i);
    }

    if (lowest_promoted.size() != 1)
    {
        throw ambiguous_type();
    }

    auto result = candidates[lowest_promoted.front()].types.back();

    //cout << "result = " << result << endl;

    return result;
}

primitive_type common_type(primitive_type t1, primitive_type t2)
{
    if (t1 <= t2)
        return t2;

    if (t2 <= t1)
        return t1;

    throw no_type();
}

primitive_type common_type(const vector<primitive_type> & types)
{
    if (types.empty())
        throw no_type();

    auto accum_func =
            static_cast<primitive_type(*)(primitive_type,primitive_type)>(&common_type);

    primitive_type type =
            std::accumulate(types.begin(), types.end(),
                            primitive_type::undefined,
                            accum_func);

    return type;
}

bool operator<=(primitive_type a, primitive_type b)
{
    using t = primitive_type;

    if (a == b)
        return true;

    if (a == t::undefined)
        return true;

    switch(b)
    {
    case t::infinity:
        return false;
    case t::boolean:
        return false;
    case t::complex64:
        return a == t::real64 || a == t::integer;
    case t::complex32:
        return a == t::real32;
    case t::real64:
        return a == t::integer;
    default:
        return false;
    }
}

}
