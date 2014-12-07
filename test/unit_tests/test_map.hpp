#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {

namespace map {

result stream1_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) y");

    t.expect_type(stream_type(9));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping::identity(1,1);

        statement *out = new statement;
        out->domain = {9};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_add_scalar()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) y + 3");

    t.expect_type(stream_type(9));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping::identity(1,1);

        statement *out = new statement;
        out->domain = {9};
        out->expr = new intrinsic(polyhedral::real, intrinsic::add, {y, new constant<int>(3)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_mul_range()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) y * 3..5");

    t.expect_type(stream_type(9,3));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping::identity(2,1);

        iterator_access *r = new iterator_access(polyhedral::integer, 1, 3);

        statement *out = new statement;
        out->domain = {9,3};
        out->expr = new intrinsic(polyhedral::real, intrinsic::multiply, {y, r});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_add_stream1()
{
    test t;

    std::istringstream code("f(a,b) = for each(x in a) x + b");

    t.expect_type(stream_type(9,5));

    {
        using namespace polyhedral;

        statement *a = new statement;
        a->domain = {9};
        a->expr = new input_access(polyhedral::real, 0);

        statement *b = new statement;
        b->domain = {5};
        b->expr = new input_access(polyhedral::real, 1);

        stmt_access *a_elem = new stmt_access(a);
        a_elem->pattern = mapping::identity(2,1);

        stmt_access *b_elem = new stmt_access(b);
        b_elem->pattern = mapping(2,1);
        b_elem->pattern.coefficient(1,0) = 1;

        statement *out = new statement;
        out->domain = {9,5};
        out->expr = new intrinsic(polyhedral::real, intrinsic::add, {a_elem, b_elem});

        t.expect_polyhedral_model({a,b,out});
    }

    return run(t, code, "f", { stream_type(9), stream_type(5) });
}

result stream1_take_n_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 4 in x) y");

    t.expect_type(stream_type(6,4));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(2,1);
        y->pattern.coefficient(0,0) = 1;
        y->pattern.coefficient(1,0) = 1;

        statement *out = new statement;
        out->domain = {6,4};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_take_n_raise_int()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 4 in x) y ^ 5");

    t.expect_type(stream_type(6,4));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(2,1);
        y->pattern.coefficient(0,0) = 1;
        y->pattern.coefficient(1,0) = 1;

        statement *out = new statement;
        out->domain = {6,4};
        out->expr = new intrinsic(polyhedral::real, intrinsic::raise, {y, new constant<int>(5)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_take_n_sub_range()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 4 in x) y - 5..8");

    t.expect_type(stream_type(6,4));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(2,1);
        y->pattern.coefficient(0,0) = 1;
        y->pattern.coefficient(1,0) = 1;

        iterator_access *r = new iterator_access(polyhedral::integer, 1, 5);

        statement *out = new statement;
        out->domain = {6,4};
        out->expr = new intrinsic(polyhedral::real, intrinsic::subtract, {y, r});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_take_n_every_n_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 3 every 2 in x) y");

    t.expect_type(stream_type(4,3));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(2,1);
        y->pattern.coefficient(0,0) = 2;
        y->pattern.coefficient(1,0) = 1;

        statement *out = new statement;
        out->domain = {4,3};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream3_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) y");

    t.expect_type(stream_type(9,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {9,11,13};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_max_double()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) max(y,100.5)");

    t.expect_type(stream_type(9,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {9,11,13};
        out->expr = new intrinsic(polyhedral::real, intrinsic::max,
        {y, new constant<double>(100.5)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_take_n_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 4 in x) y");

    t.expect_type(stream_type(6,4,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(4,3);
        y->pattern.coefficient(0,0) = 1;
        y->pattern.coefficient(1,0) = 1;
        y->pattern.coefficient(2,1) = 1;
        y->pattern.coefficient(3,2) = 1;

        statement *out = new statement;
        out->domain = {6,4,11,13};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_take_n_every_n_identity()
{
    test t;

    std::istringstream code("f(x) = for each(y takes 3 every 2 in x) y");

    t.expect_type(stream_type(4,3,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(polyhedral::real, 0);

        stmt_access *y = new stmt_access(in);
        y->pattern = mapping(4,3);
        y->pattern.coefficient(0,0) = 2;
        y->pattern.coefficient(1,0) = 1;
        y->pattern.coefficient(2,1) = 1;
        y->pattern.coefficient(3,2) = 1;

        statement *out = new statement;
        out->domain = {4,3,11,13};
        out->expr = y;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result range_identity()
{
    test t;

    std::istringstream code("f = for each(y in 3..7) y");

    t.expect_type(stream_type(5));

    {
        using namespace polyhedral;

        iterator_access *y = new iterator_access(polyhedral::real, 0, 3);

        statement *out = new statement;
        out->domain = {5};
        out->expr = y;

        t.expect_polyhedral_model({out});
    }

    return run(t, code, "f");
}

result range_add_int()
{
    test t;

    std::istringstream code("f = for each(y in 3..7) y + 123");

    t.expect_type(stream_type(5));

    {
        using namespace polyhedral;

        iterator_access *y = new iterator_access(polyhedral::integer, 0, 3);

        statement *out = new statement;
        out->domain = {5};
        out->expr = new intrinsic(polyhedral::real, intrinsic::add,
        {y, new constant<int>(123)});

        t.expect_polyhedral_model({out});
    }

    return run(t, code, "f");
}

result range_add_range()
{
    test t;

    std::istringstream code("f = for each(y in 3..7) y + 5..7");

    t.expect_type(stream_type(5,3));

    {
        using namespace polyhedral;

        iterator_access *y = new iterator_access(polyhedral::integer, 0, 3);
        iterator_access *r = new iterator_access(polyhedral::integer, 1, 5);

        statement *out = new statement;
        out->domain = {5,3};
        out->expr = new intrinsic(polyhedral::real, intrinsic::add, {y, r});

        t.expect_polyhedral_model({out});
    }

    return run(t, code, "f");
}

result range_every_n_identity()
{
    test t;

    std::istringstream code("f = for each(y every 2 in 10..17) y");

    t.expect_type(stream_type(4));

    {
        using namespace polyhedral;

        iterator_access *y = new iterator_access(polyhedral::real, 0, 10, 2);

        statement *out = new statement;
        out->domain = {4};
        out->expr = y;

        t.expect_polyhedral_model({out});
    }

    return run(t, code, "f");
}

result range_to_stream1()
{
    test t;

    std::istringstream code("f(x) = for each(in 1..9) x");

    t.expect_type(stream_type(9,5));

    {
        using namespace polyhedral;

        statement *x = new statement;
        x->domain = {5};
        x->expr = new input_access(polyhedral::real, 0);

        stmt_access *x_elem = new stmt_access(x);
        x_elem->pattern = mapping(2,1);
        x_elem->pattern.coefficient(1,0) = 1;

        statement *out = new statement;
        out->domain = {9,5};
        out->expr = x_elem;

        t.expect_polyhedral_model({x,out});
    }

    return run(t, code, "f", { stream_type(5) });
}

/*
result stream1()
{
    string func("(S) = for each( x in S ) x");
    return is_func_type(func, {stream_type(9)}, stream_type(9));
}

result stream1_to_real()
{
    string func("(S) = for each( in S ) 1.234");
    return is_func_type(func, {stream_type(9)}, stream_type(9));
}

result stream1_to_int()
{
    string func("(S) = for each( in S ) 1");
    return is_func_type(func, {stream_type(9)}, stream_type(9));
}

result stream1_to_stream2()
{
    string func("(S,X) = for each( in S ) X");
    return is_func_type(func, {stream_type(9), stream_type(5,2)},
                        stream_type(9,5,2));
}

result stream1_take_n()
{
    string func("(S) = for each( x takes 3 in S ) x");
    return is_func_type(func, {stream_type(9)}, stream_type(7,3));
}

result stream1_take_n_every_n()
{
    string func("(S) = for each( x takes 3 every 3 in S ) x");
    return is_func_type(func, {stream_type(9)}, stream_type(3,3));
}

result stream3()
{
    string func("(S) = for each( x in S ) x");
    return is_func_type(func, {stream_type(9,5,2)}, stream_type(9,5,2));
}

result stream3_take_n()
{
    string func("(S) = for each( x takes 3 in S ) x");
    return is_func_type(func, {stream_type(9,5,2)}, stream_type(7,3,5,2));
}

result range()
{
    string expr("for each( x in 1..9 ) x");
    return is_expr_type(expr, stream_type(9));
}

result range_take_n()
{
    string expr("for each( x takes 3 in 1..9 ) 111");
    return is_expr_type(expr, stream_type(7));
}

result range_take_n_every_n()
{
    string expr("for each( x takes 3 every 3 in 1..9 ) 111");
    return is_expr_type(expr, stream_type(3));
}
*/

}

}
}
