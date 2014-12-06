#include "unit_test.hpp"

#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

namespace stream {
namespace unit_testing {
namespace intrinsics {

template<typename T> T test_const_lhs();
template<typename T> T test_const_rhs();
template<> int test_const_lhs<int>() { return 2; }
template<> int test_const_rhs<int>() { return 3; }
template<> double test_const_lhs<double>() { return 2.3; }
template<> double test_const_rhs<double>() { return 3.4; }

template<typename T> type_ptr type_for();
template<> type_ptr type_for<int>() { return int_type(); }
template<> type_ptr type_for<double>() { return real_type(); }

template<typename T> type_ptr type_for(T);
template<> type_ptr type_for<int>(int c) { return int_type(c); }
template<> type_ptr type_for<double>(double c) { return real_type(c); }

template<polyhedral::intrinsic::of_kind intrinsic_type>
struct intrinsic_test;

template<>
struct intrinsic_test<polyhedral::intrinsic::add>
{
    template<typename A, typename B, typename R>
    static
    R perform(A a, B b) { return a + b; }

    static
    string code(const string & a, const string & b) { return a + " + " + b; }
};

template<>
struct intrinsic_test<polyhedral::intrinsic::raise>
{
    template<typename A, typename B, typename R>
    static
    R perform(A a, B b) { return std::pow(a,b); }

    static
    string code(const string & a, const string & b) { return "pow(" + a + "," + b + ")"; }
};

template <polyhedral::intrinsic::of_kind I, typename A, typename B, typename R>
result test_intrinsic_const()
{
    A lhs = test_const_lhs<A>();
    B rhs = test_const_rhs<B>();
    R result = intrinsic_test<I>::template perform<A,B,R>(lhs, rhs);

    std::stringstream code;
    code << "result = " << intrinsic_test<I>::code(to_string(lhs), to_string(rhs));
    code.seekg(0);

    cout << "Input:" << endl;
    cout << code.str() << endl;

    test t;
    t.expect_type(type_for<R>(result));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new constant<R>(result);
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

template <polyhedral::intrinsic::of_kind I, typename A, typename B, typename R>
result test_intrinsic()
{
    std::stringstream code;
    code << "result(a,b) = " << intrinsic_test<I>::code("a","b");
    code.seekg(0);

    cout << "Input:" << endl;
    cout << code.str() << endl;

    test t;
    t.expect_type(type_for<R>());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(I,
        {new input_access(0), new input_access(1)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result", {type_for<A>(), type_for<B>()});
}

result add_int_int_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::add,int,int,int>();
}


result add_int_real_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::add,int,double,double>();
}

result add_real_int_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::add,double,int,double>();
}

result add_real_real_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::add,double,double,double>();
}

result add_int_int()
{
    return test_intrinsic<polyhedral::intrinsic::add,int,int,int>();
}

result add_int_real()
{
    return test_intrinsic<polyhedral::intrinsic::add,int,double,double>();
}

result add_real_int()
{
    return test_intrinsic<polyhedral::intrinsic::add,double,int,double>();
}

result add_real_real()
{
    return test_intrinsic<polyhedral::intrinsic::add,double,double,double>();
}

result add_int_range()
{
    std::istringstream code("result = 1 + 3..10");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<int>(1), new iterator_access(0,3)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result add_real_range()
{
    std::istringstream code("result = 2.7 + 3..10");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<double>(2.7), new iterator_access(0,3)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result add_range_int()
{
    std::istringstream code("result = 3..10 + 222");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new constant<int>(222)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result add_range_real()
{
    std::istringstream code("result = 3..10 + 2.34");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new constant<double>(2.34)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result add_range_range()
{
    std::istringstream code("result = 3..10 + 4..11");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new iterator_access(0,4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result add_int_stream()
{
    test t;
    std::istringstream code("result(x) = 2 + x");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {new constant<int>(2), x});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result add_stream_int()
{
    test t;
    std::istringstream code("result(x) = x + 2");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {x, new constant<int>(2)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}


result add_real_stream()
{
    test t;
    std::istringstream code("result(x) = 3.4 + x");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {new constant<double>(3.4), x});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result add_stream_real()
{
    test t;
    std::istringstream code("result(x) = x + 3.4");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {x, new constant<double>(3.4)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result power_int_int_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::raise,int,int,int>();
}

result power_real_real_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::raise,double,double,double>();
}

result power_int_real_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::raise,int,double,double>();
}

result power_real_int_const()
{
    return test_intrinsic_const<polyhedral::intrinsic::raise,double,int,double>();
}

result power_int_int()
{
    return test_intrinsic<polyhedral::intrinsic::raise,int,int,int>();
}

result power_real_real()
{
    return test_intrinsic<polyhedral::intrinsic::raise,double,double,double>();
}

result power_int_real()
{
    return test_intrinsic<polyhedral::intrinsic::raise,int,double,double>();
}

result power_real_int()
{
    return test_intrinsic<polyhedral::intrinsic::raise,double,int,double>();
}

result power_stream_int()
{
    test t;
    std::istringstream code("result(x) = pow(x,3)");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::raise, {x, new constant<int>(3)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

}
}
}
