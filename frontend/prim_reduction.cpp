#include "prim_reduction.hpp"
#include "error.hpp"

#include <limits>

using namespace std;

namespace stream {
namespace functional {

struct undefined_value {};

bool to_b(expr_ptr expr)
{
    if(auto b = dynamic_cast<bool_const*>(expr.get()))
    {
        return b->value;
    }
    else
    {
        throw undefined_value();
    }
}

mpz_class to_i(expr_ptr expr)
{
    if (auto i = dynamic_cast<int_const*>(expr.get()))
    {
        return i->value();
    }

    throw undefined_value();
}

double to_d(expr_ptr expr)
{
    if (auto d = dynamic_cast<real_const*>(expr.get()))
    {
        return d->value;
    }
    else
    {
        throw undefined_value();
    }
}

template <typename F, typename ... Fs>
expr_ptr try_reduce(F f, Fs ... fs)
{
    try
    {
        return f();
    }
    catch (undefined_value &)
    {
        return try_reduce(fs...);
    }
}

expr_ptr c_i(mpz_class i, primitive_type t)
{
    return make_shared<int_const>(i, location_type(), make_shared<scalar_type>(t));
}

expr_ptr c_d(double d) { return make_shared<real_const>(d); }
expr_ptr c_b(bool b) { return make_shared<bool_const>(b); }

expr_ptr reduce_primitive(std::shared_ptr<primitive> op)
{
    auto & a = op->operands;

    // TODO: Reduce more, including casts...

    vector<primitive_type> arg_types;

    for (auto & arg : a)
    {
        if (!arg->type)
            return op;
        if (!arg->type->is_scalar())
            return op;
        arg_types.push_back(arg->type->scalar()->primitive);
    }

    primitive_type result_type;

    try { result_type = stream::result_type(op->kind, arg_types); }
    catch (no_type &) { return op; }

    try
    {
        switch(op->kind)
        {
        case primitive_op::add:
            return c_i(to_i(a[0]) + to_i(a[1]), result_type);
        case primitive_op::subtract:
            return c_i(to_i(a[0]) - to_i(a[1]), result_type);
        case primitive_op::multiply:
            return c_i(to_i(a[0]) * to_i(a[1]), result_type);
        case primitive_op::divide_integer:
            return c_i(to_i(a[0]) / to_i(a[1]), result_type);
        case primitive_op::negate:
            return c_i(-to_i(a[0]), result_type);
        default:
            return op;
        }
    }
    catch ( undefined_value & )
    {
        return op;
    }
}

}
}
