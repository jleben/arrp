#include "primitives.hpp"

#include <numeric>

using namespace std;

namespace stream {

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

    switch(op)
    {
    case primitive_op::negate:
        return {
            { pt::integer, pt::integer },
            { pt::real, pt::real },
            { pt::complex, pt::complex },
            { pt::boolean, pt::boolean }
        };
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real, pt::real, pt::real },
            { pt::complex, pt::complex, pt::complex }
        };
    case primitive_op::divide:
        return {
            { pt::real, pt::real, pt::real },
            { pt::complex, pt::complex, pt::complex }
        };
    case primitive_op::divide_integer:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real, pt::real, pt::integer }
        };
    case primitive_op::raise:
        return {
            { pt::integer, pt::integer, pt::integer },
            { pt::real, pt::real, pt::real },
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
            { pt::real, pt::real },
            { pt::complex, pt::complex }
        };
    case primitive_op::log2:
        return { { pt::real, pt::real } };
    case primitive_op::exp2:
        return { {pt::integer, pt::integer }, { pt::real, pt::real } };
    case primitive_op::ceil:
    case primitive_op::floor:
        return { {pt::integer, pt::integer }, { pt::real, pt::integer } };
    case primitive_op::abs:
        return { {pt::integer, pt::integer }, { pt::real, pt::real } };
    case primitive_op::min:
    case primitive_op::max:
        return { { pt::integer, pt::integer, pt::integer }, { pt::real, pt::real, pt::real } };
    case primitive_op::real:
    case primitive_op::imag:
        return { { pt::complex, pt::complex, pt::real } };
    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
        return {
            { pt::integer, pt::integer, pt::boolean },
            { pt::real, pt::real, pt::boolean },
            { pt::complex, pt::complex, pt::boolean }
        };
    case primitive_op::compare_l:
    case primitive_op::compare_g:
    case primitive_op::compare_leq:
    case primitive_op::compare_geq:
        return { { pt::integer, pt::integer, pt::boolean }, { pt::real, pt::real, pt::boolean } };
    case primitive_op::logic_and:
    case primitive_op::logic_or:
        return { { pt::boolean, pt::boolean, pt::boolean } };
    case primitive_op::conditional:
        return {
            { pt::boolean, pt::integer, pt::integer, pt::integer },
            { pt::boolean, pt::real, pt::real, pt::real },
            { pt::boolean, pt::complex, pt::complex, pt::complex }
        };
    default:
        return {};
    }
}

primitive_type result_type(primitive_op op, vector<primitive_type> & args)
{
    using type = primitive_type;

    auto candidates = overloads(op);
    if (candidates.empty())
        throw no_type();

    // If any arg is undefined,
    // and all overloads return the same type,
    // then skip type checking and return that type.
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
            auto t = common_type(pt,at);
            if (t == pt)
                continue;
            is_valid = false;
            break;
        }
        if (is_exact)
            return candidates[c].types.back();
        if (is_valid)
            promoted.push_back(c);
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
                auto c = common_type(a,b);
                ok = (c == b);
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
    using type = primitive_type;

    if (t1 == t2)
        return t1;
    if (t1 == type::undefined)
        return t2;
    if (t2 == type::undefined)
        return t1;
    if (t1 == type::boolean || t2 == type::boolean)
        throw no_type();
    if (t1 == type::complex || t2 == type::complex)
        return type::complex;
    if (t1 == type::real || t2 == type::real)
        return type::real;
    // Else both must be integers (caught by first case),
    // or an unexpected type:
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

}
