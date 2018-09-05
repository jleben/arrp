#include "built_in_types.hpp"
#include <cassert>

using namespace std;

namespace arrp {

struct array_type_constructor : type_constructor
{
    array_type_constructor(): type_constructor("array") {}

    void print(ostream & out, const vector<type_ptr> & arguments) override
    {
        assert(arguments.size() == 2);
        out << "[" << arguments[0] << ":" << arguments[1] << "]";
    }
};

struct func_type_constructor : type_constructor
{
    func_type_constructor(): type_constructor("function") {}

    void print(ostream & out, const vector<type_ptr> & arguments) override
    {
        assert(arguments.size() == 2);
        out << "(" << arguments[0] << ")"
            << " -> "
            << "(" << arguments[1] << ")";
    }
};

type_cons_ptr make_primitive_type(const string & name)
{
    return shared(new type_cons(shared(new type_constructor(name))));
}

type_class::instantiator simple_class_instance(type_ptr t)
{
    return [=](){ return vector<type_ptr>{ t }; };
}

built_in_types::built_in_types()
{
    m_array_cons = shared(new array_type_constructor);
    m_func_cons = shared(new func_type_constructor);

    m_boolean = make_primitive_type("bool");
    m_integer32 = make_primitive_type("int32");
    m_integer64 = make_primitive_type("int64");
    m_real32 = make_primitive_type("real32");
    m_real64 = make_primitive_type("real64");
    m_complex32 = make_primitive_type("complex32");
    m_complex64 = make_primitive_type("complex64");
    m_infinity = make_primitive_type("inf");

    m_integral = shared(new type_class("Integral"));
    m_integral->instances.push_back(simple_class_instance(m_integer32));
    m_integral->instances.push_back(simple_class_instance(m_integer64));

    m_real = shared(new type_class("Real"));
    m_real->instances.push_back(simple_class_instance(m_real32));
    m_real->instances.push_back(simple_class_instance(m_real64));

    m_complex = shared(new type_class("Complex"));
    m_complex->instances.push_back(simple_class_instance(m_complex32));
    m_complex->instances.push_back(simple_class_instance(m_complex64));

    m_numeric = shared(new type_class("Numeric", m_integral->instances | m_real->instances | m_complex->instances));

    m_divisible = shared(new type_class("Divisible", m_real->instances | m_complex->instances ));

    m_scalar = shared(new type_class("Scalar",  m_integral->instances | m_real->instances));

    m_elementary = shared(new type_class("Elementary", m_integral->instances | m_real->instances | m_complex->instances));
    m_elementary->instances.push_back(simple_class_instance(m_boolean));

    m_indexable = shared(new type_class("Indexable"));
    for (const auto & t : { m_boolean, m_integer32, m_integer64, m_real32, m_real64, m_complex32, m_complex64 })
    {
        m_indexable->instances.push_back([=](){
            return vector<type_ptr>{ t, t };
        });
    };
    m_indexable->instances.push_back([=](){
        auto elem = shared(new type_var);
        auto a = array(type_ptr(new type_var), elem);
        return vector<type_ptr>{ a, elem };
    });

    m_array_size = shared(new type_class("ArraySize"));
    m_array_size->instances.push_back(simple_class_instance(integer64()));
    m_array_size->instances.push_back(simple_class_instance(infinity()));

    using stream::primitive_op;
    {
        auto f = binary_op(m_numeric);
        m_primitive_ops[primitive_op::add] = f;
        m_primitive_ops[primitive_op::multiply] = f;
        m_primitive_ops[primitive_op::subtract] = f;
    }
    {
        m_primitive_ops[primitive_op::divide] = binary_op(m_divisible);
    }
    {
        auto f = binary_op(m_integral);
        m_primitive_ops[primitive_op::divide_integer] = f;
        m_primitive_ops[primitive_op::modulo] = f;
    }
    {
        m_primitive_ops[primitive_op::raise] = binary_op(m_scalar);
    }
    {
        auto f = unary_op(m_divisible);
        m_primitive_ops[primitive_op::exp] = f;
        m_primitive_ops[primitive_op::log] = f;
        m_primitive_ops[primitive_op::log10] = f;
        m_primitive_ops[primitive_op::sin] = f;
        m_primitive_ops[primitive_op::cos] = f;
        m_primitive_ops[primitive_op::tan] = f;
        m_primitive_ops[primitive_op::asin] = f;
        m_primitive_ops[primitive_op::acos] = f;
        m_primitive_ops[primitive_op::atan] = f;
        m_primitive_ops[primitive_op::sqrt] = f;
    }
    {
        m_primitive_ops[primitive_op::log2] = unary_op(m_real);
    }
    {
        m_primitive_ops[primitive_op::exp2] = unary_op(m_scalar);
    }
    {
        auto f = unary_op(m_scalar);
        m_primitive_ops[primitive_op::floor] = f;
        m_primitive_ops[primitive_op::ceil] = f;
        m_primitive_ops[primitive_op::abs] = f;
    }
    {
        auto f = binary_op(m_scalar);
        m_primitive_ops[primitive_op::min] = f;
        m_primitive_ops[primitive_op::max] = f;
    }
    {
        // FIXME: real, imag
    }
    {
        auto s = shared(new type_var);
        add_constraint(m_scalar, s);

        m_primitive_ops[primitive_op::to_integer] = function(s, m_integer32);
        m_primitive_ops[primitive_op::to_real32] = function(s, m_real32);
        m_primitive_ops[primitive_op::to_real64] = function(s, m_real64);
    }
    {
        auto n = shared(new type_var);
        add_constraint(m_numeric, n);

        m_primitive_ops[primitive_op::to_complex32] = function(n, m_complex32);
        m_primitive_ops[primitive_op::to_complex64] = function(n, m_complex64);
    }
    {
        auto e = shared(new type_var);
        add_constraint(m_elementary, e);
        auto f = function({ e, e }, m_boolean);
        m_primitive_ops[primitive_op::compare_eq] = f;
        m_primitive_ops[primitive_op::compare_neq] = f;
    }
    {
        auto s = shared(new type_var);
        add_constraint(m_scalar, s);
        auto f = function({ s, s }, m_boolean);
        m_primitive_ops[primitive_op::compare_l] = f;
        m_primitive_ops[primitive_op::compare_g] = f;
        m_primitive_ops[primitive_op::compare_leq] = f;
        m_primitive_ops[primitive_op::compare_geq] = f;
    }
    {
        auto f = function({ m_boolean, m_boolean }, m_boolean);
        m_primitive_ops[primitive_op::logic_and] = f;
        m_primitive_ops[primitive_op::logic_or] = f;
    }
    {
        auto e = shared(new type_var);
        add_constraint(m_elementary, e);
        auto f = function({ m_boolean, e, e }, e);
        m_primitive_ops[primitive_op::conditional] = f;
    }
}

type_cons_ptr built_in_types::array(type_ptr size, type_ptr elem)
{
    return shared(new type_cons(m_array_cons, { size, elem }));
}

type_cons_ptr built_in_types::array(const vector<type_ptr> & sizes, type_ptr elem)
{
    type_cons_ptr first;
    type_cons_ptr last;

    for (const auto & size : sizes)
    {
        auto a = array(size, type_ptr());
        if (!first)
            first = a;
        if (last)
            last->arguments[1] = a;
        last = a;
    }

    if (last)
        last->arguments[1] = elem;

    return first;
}

type_cons_ptr built_in_types::finite_array(type_ptr elem)
{
    return shared(new type_cons(m_array_cons, { integer64(), elem }));
}

type_cons_ptr built_in_types::infinite_array(type_ptr elem)
{
    return shared(new type_cons(m_array_cons, { infinity(), elem }));
}

type_cons_ptr built_in_types::function(type_ptr param, type_ptr result)
{
    return shared(new type_cons(m_func_cons, { param, result }));
}

type_cons_ptr built_in_types::function(const vector<type_ptr> & params, type_ptr result)
{
    type_cons_ptr last;
    type_cons_ptr first;

    for (const auto & param : params)
    {
        auto f = function(param, nullptr);
        if (!first)
            first = f;
        if (last)
            last->arguments[1] = f;
        last = f;
    }

    if (last)
        last->arguments[1] = result;

    return first;
}

type_ptr built_in_types::unary_op(type_class_ptr klass)
{
    auto var = shared(new type_var);
    add_constraint(klass, var);
    return function(var, var);
}

type_ptr built_in_types::binary_op(type_class_ptr klass)
{
    auto var = shared(new type_var);
    add_constraint(klass, var);
    return function({ var, var }, var);
}

type_ptr built_in_types::primitive_op(stream::primitive_op kind)
{
    return m_primitive_ops[kind];
}

}

