/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "unit_test.hpp"

#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

namespace stream {
namespace unit_testing {
namespace primitives {

template<typename T> T test_const_lhs();
template<typename T> T test_const_rhs();
template<> int test_const_lhs<int>() { return 5; }
template<> int test_const_rhs<int>() { return 3; }
template<> double test_const_lhs<double>() { return 6.2; }
template<> double test_const_rhs<double>() { return 3.4; }

template<typename T> type_ptr type_for();
template<> type_ptr type_for<int>() { return int_type(); }
template<> type_ptr type_for<double>() { return real_type(); }

template<typename T> type_ptr type_for(T);
template<> type_ptr type_for<int>(int c) { return int_type(c); }
template<> type_ptr type_for<double>(double c) { return real_type(c); }

template<primitive_op>
struct primitive_test;

template<>
struct primitive_test<primitive_op::add>
{
    static int perform(int a, int b) { return a + b; }
    static double perform(int a, double b) { return a + b; }
    static double perform(double a, int b) { return a + b; }
    static double perform(double a, double b) { return a + b; }

    static
    string code(const string & a, const string & b) { return a + " + " + b; }
};

template<>
struct primitive_test<primitive_op::raise>
{
    static int perform(int a, int b) { return std::pow(a,b); }
    static double perform(int a, double b) { return std::pow(a,b); }
    static double perform(double a, int b) { return std::pow(a,b); }
    static double perform(double a, double b) { return std::pow(a,b); }

    static
    string code(const string & a, const string & b) { return "pow(" + a + "," + b + ")"; }
};

template<>
struct primitive_test<primitive_op::divide>
{
    static double perform(double a, double b) { return a / b; }

    static
    string code(const string & a, const string & b) { return a + " / " + b; }
};

template<>
struct primitive_test<primitive_op::divide_integer>
{
    static int perform(int a, int b) { return a / b; }
    static int perform(double a, int b) { return a / b; }
    static int perform(int a, double b) { return a / b; }
    static int perform(double a, double b) { return a / b; }

    static
    string code(const string & a, const string & b) { return a + " : " + b; }
};

template <primitive_op I, typename A, typename B, typename R>
result test_primitive_const()
{
    A lhs = test_const_lhs<A>();
    B rhs = test_const_rhs<B>();
    R result = primitive_test<I>::perform(lhs, rhs);

    std::stringstream code;
    code << "result = " << primitive_test<I>::code(to_string(lhs), to_string(rhs));
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

template <primitive_op I, typename A, typename B, typename R>
result test_primitive()
{
    std::stringstream code;
    code << "result(a,b) = " << primitive_test<I>::code("a","b");
    code.seekg(0);

    cout << "Input:" << endl;
    cout << code.str() << endl;

    test t;
    t.expect_type(type_for<R>());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new primitive_expr(polyhedral::expr_type<R>::type, I,
        {new input_access(polyhedral::expr_type<A>::type, 0),
         new input_access(polyhedral::expr_type<B>::type,1 )});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result", {type_for<A>(), type_for<B>()});
}

result add_int_int_const()
{
    return test_primitive_const<primitive_op::add,int,int,int>();
}

result add_int_real_const()
{
    return test_primitive_const<primitive_op::add,int,double,double>();
}

result add_real_int_const()
{
    return test_primitive_const<primitive_op::add,double,int,double>();
}

result add_real_real_const()
{
    return test_primitive_const<primitive_op::add,double,double,double>();
}

result add_int_int()
{
    return test_primitive<primitive_op::add,int,int,int>();
}

result add_int_real()
{
    return test_primitive<primitive_op::add,int,double,double>();
}

result add_real_int()
{
    return test_primitive<primitive_op::add,double,int,double>();
}

result add_real_real()
{
    return test_primitive<primitive_op::add,double,double,double>();
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
        stmt->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new constant<int>(1), new iterator_access(polyhedral::integer,0,3)});
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
        stmt->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new constant<double>(2.7), new iterator_access(polyhedral::integer,0,3)});
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
        stmt->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new iterator_access(polyhedral::integer,0,3), new constant<int>(222)});
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
        stmt->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new iterator_access(polyhedral::integer,0,3), new constant<double>(2.34)});
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
        stmt->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new iterator_access(polyhedral::integer,0,3),
         new iterator_access(polyhedral::integer,0,4)});
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
        in->expr = new input_access(polyhedral::real,0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new constant<int>(2), x});

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
        in->expr = new input_access(polyhedral::real,0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::real,primitive_op::add,
        {x, new constant<int>(2)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result add_istream_int()
{
    test t;
    std::istringstream code("result(x) = x + 2");

    t.set_target("result", { stream_type(primitive_type::integer, 4,5,6) });
    t.expect_type(stream_type(primitive_type::integer, 4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(polyhedral::integer,0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::integer, primitive_op::add,
        {x, new constant<int>(2)});

        t.expect_polyhedral_model({in,out});
    }

    return t.try_run(code);
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
        in->expr = new input_access(polyhedral::real,0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {new constant<double>(3.4), x});

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
        in->expr = new input_access(polyhedral::real,0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::real, primitive_op::add,
        {x, new constant<double>(3.4)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result power_int_int_const()
{
    return test_primitive_const<primitive_op::raise,int,int,int>();
}

result power_real_real_const()
{
    return test_primitive_const<primitive_op::raise,double,double,double>();
}

result power_int_real_const()
{
    return test_primitive_const<primitive_op::raise,int,double,double>();
}

result power_real_int_const()
{
    return test_primitive_const<primitive_op::raise,double,int,double>();
}

result power_int_int()
{
    return test_primitive<primitive_op::raise,int,int,int>();
}

result power_real_real()
{
    return test_primitive<primitive_op::raise,double,double,double>();
}

result power_int_real()
{
    return test_primitive<primitive_op::raise,int,double,double>();
}

result power_real_int()
{
    return test_primitive<primitive_op::raise,double,int,double>();
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
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new primitive_expr(polyhedral::real, primitive_op::raise,
        {x, new constant<int>(3)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result div_int_int_const()
{
    return test_primitive_const<primitive_op::divide,int,int,double>();
}
result div_int_real_const()
{
    return test_primitive_const<primitive_op::divide,int,double,double>();
}
result div_real_int_const()
{
    return test_primitive_const<primitive_op::divide,double,int,double>();
}
result div_real_real_const()
{
    return test_primitive_const<primitive_op::divide,double,double,double>();
}

result div_int_int()
{
    return test_primitive<primitive_op::divide,int,int,double>();
}
result div_int_real()
{
    return test_primitive<primitive_op::divide,int,double,double>();
}
result div_real_int()
{
    return test_primitive<primitive_op::divide,double,int,double>();
}
result div_real_real()
{
    return test_primitive<primitive_op::divide,double,double,double>();
}

result i_div_int_int_const()
{
    return test_primitive_const<primitive_op::divide_integer,int,int,int>();
}
result i_div_int_real_const()
{
    return test_primitive_const<primitive_op::divide_integer,int,double,int>();
}
result i_div_real_int_const()
{
    return test_primitive_const<primitive_op::divide_integer,double,int,int>();
}
result i_div_real_real_const()
{
    return test_primitive_const<primitive_op::divide_integer,double,double,int>();
}

result i_div_int_int()
{
    return test_primitive<primitive_op::divide_integer,int,int,int>();
}
result i_div_int_real()
{
    return test_primitive<primitive_op::divide_integer,int,double,int>();
}
result i_div_real_int()
{
    return test_primitive<primitive_op::divide_integer,double,int,int>();
}
result i_div_real_real()
{
    return test_primitive<primitive_op::divide_integer,double,double,int>();
}

}
}
}
