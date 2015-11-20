#include "primitives.hpp"

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
            { pt::boolean, pt::boolean }
        };
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
    case primitive_op::raise:
        return { { pt::integer, pt::integer, pt::integer }, { pt::real, pt::real, pt::real } };
    case primitive_op::divide:
        return { { pt::real, pt::real, pt::real } };
    case primitive_op::divide_integer:
        return { { pt::integer, pt::integer, pt::integer }, { pt::real, pt::real, pt::integer } };
    case primitive_op::exp:
    case primitive_op::log:
    case primitive_op::log2:
    case primitive_op::log10:
    case primitive_op::sqrt:
    case primitive_op::sin:
    case primitive_op::cos:
    case primitive_op::tan:
    case primitive_op::asin:
    case primitive_op::acos:
    case primitive_op::atan:
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
    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
    case primitive_op::compare_l:
    case primitive_op::compare_g:
    case primitive_op::compare_leq:
    case primitive_op::compare_geq:
        return { { pt::integer, pt::integer, pt::boolean }, { pt::real, pt::real, pt::boolean } };
    case primitive_op::logic_and:
    case primitive_op::logic_or:
        return { { pt::boolean, pt::boolean, pt::boolean } };
    case primitive_op::conditional:
        return { { pt::boolean, pt::integer, pt::integer, pt::integer } };
        return { { pt::boolean, pt::real, pt::real, pt::real } };
    default:
        return {};
    }
}

}
