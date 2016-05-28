#include "prim_reduction.hpp"
#include "error.hpp"

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

int to_i(expr_ptr expr)
{
    if (auto i = dynamic_cast<int_const*>(expr.get()))
    {
        return i->value;
    }
    else
    {
        throw undefined_value();
    }
}

double to_d(expr_ptr expr)
{
    if (auto d = dynamic_cast<real_const*>(expr.get()))
    {
        return d->value;
    }
    else if (auto i = dynamic_cast<int_const*>(expr.get()))
    {
        return i->value;
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

template <typename F>
expr_ptr try_reduce(F f)
{
    return f();
}

expr_ptr c_i(int i) { return make_shared<int_const>(i); }
expr_ptr c_d(double d) { return make_shared<real_const>(d); }
expr_ptr c_b(bool b) { return make_shared<bool_const>(b); }

expr_ptr reduce_primitive(std::shared_ptr<primitive> op)
{
    auto & a = op->operands;

    try
    {
        switch(op->kind)
        {
        case primitive_op::add:
            return try_reduce([&](){ return c_i(to_i(a[0]) + to_i(a[1])); } );
        case primitive_op::subtract:
            return try_reduce([&](){ return c_i(to_i(a[0]) - to_i(a[1])); } );
        case primitive_op::multiply:
            return try_reduce([&](){ return c_i(to_i(a[0]) * to_i(a[1])); } );
        case primitive_op::divide_integer:
            return try_reduce([&](){ return c_i(to_i(a[0]) / to_i(a[1])); } );
        case primitive_op::negate:
            return try_reduce([&](){ return c_i(-to_i(a[0])); });
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
