#include "unit_test.hpp"

#include <sstream>
#include <initializer_list>
#include <memory>

using namespace stream;
using namespace stream::semantic;
using namespace std;

using unit_test::result;
using unit_test::failure_msg;
using unit_test::success_msg;

type_ptr int_type() { return make_shared<integer_num>(); }
type_ptr real_type() { return make_shared<real_num>(); }
type_ptr range_type() { return make_shared<range>(); }

type_ptr stream_type( int size)
{ return make_shared<semantic::stream>(size); }

template <typename ...T>
type_ptr stream_type(const T & ...size)
{ return make_shared<semantic::stream>( vector<int>({size...}) ); }

bool type_matches(const type & actual, const type & expected)
{
    switch(actual.get_tag())
    {
    case type::integer_num:
    case type::real_num:
    case type::range:
        return actual.get_tag() == expected.get_tag();
    case type::stream:
    {
        bool ok = expected.is(type::stream);
        if (ok)
            ok = actual.as<semantic::stream>().size ==
                    expected.as<semantic::stream>().size;
        return ok;
    }
    default:
        return false;
    }
}

result is_expr_type(const string & expr, const type_ptr & expected_type)
{
    ostringstream code;
    code << "result = " << expr;
    type_ptr result_type =
            unit_test::semantic_analysis(code.str(), "result");
    if (!result_type)
        return unit_test::failure;

    bool ok = type_matches(*result_type, *expected_type);

    if (ok)
        return unit_test::success_msg();
    else
    {
        ostringstream msg;
        msg << "Wrong result type:"
            << " expected = " << *expected_type
            << " / actual = " << *result_type;
        return failure_msg(msg.str());
    }
}

result is_func_type(const string &func,
                    const vector<type_ptr> & arg_types,
                    const type_ptr & expected_type)
{
    ostringstream code;
    code << "function" << func;

    type_ptr func_type =
            unit_test::semantic_analysis(code.str(), "function", arg_types);
    if (!func_type)
        return unit_test::failure;

    if (!func_type->is(type::function))
    {
        return failure_msg("No function type.");
    }

    semantic::function & f = func_type->as<semantic::function>();
    type_ptr result_type = f.result_type();
    if (!result_type)
        return failure_msg("No function result type.");

    if (!type_matches(*result_type, *expected_type))
    {
        ostringstream msg;
        msg << "Wrong result type:"
            << " expected = " << *expected_type
            << " / actual = " << *result_type;
        return failure_msg(msg.str());
    }

    return success_msg();
}

namespace binop {

result ii() { return is_expr_type("1 + 2", int_type()); }
result ir() { return is_expr_type("1 + 2.3", real_type()); }
result ri() { return is_expr_type("2.3 + 1", real_type()); }
result rr() { return is_expr_type("2.3 + 3.4", real_type()); }
result iR() { return is_expr_type("1 + 1..10", stream_type(10)); }
result rR() { return is_expr_type("2.3 + 1..10", stream_type(10)); }
result Ri() { return is_expr_type("1..10 + 1", stream_type(10)); }
result Rr() { return is_expr_type("1..10 + 2.3", stream_type(10)); }
result RR() { return is_expr_type("1..10 + 11..20", stream_type(10)); }
result iS() { return is_expr_type("1 + 1..10 * 6", stream_type(10)); }
result rS() { return is_expr_type("2.3 + 1..10 * 6", stream_type(10)); }
result Si() { return is_expr_type("1..10 * 6 + 1", stream_type(10)); }
result Sr() { return is_expr_type("1..10 * 6 + 2.3", stream_type(10)); }
result SS() { return is_expr_type("1..10 * 6 + 11..20 * 7", stream_type(10)); }
result raise_int_int() { return is_expr_type("2 ^ 4", int_type()); }
result raise_real_real() { return is_expr_type("2.2 ^ 4.4", real_type()); }
result raise_int_real() { return is_expr_type("2 ^ 4.4", real_type()); }
result raise_real_int() { return is_expr_type("2.2 ^ 4", real_type()); }
result raise_stream_int() { return is_expr_type("(1..9 * 2) ^ 4", stream_type(9)); }
}

namespace slicing {

result stream1_by_int ()
{
    string func("(S) = S[1]");
    return is_func_type(func, {stream_type(9)}, real_type());
}

result stream1_by_range ()
{
    string func("(S) = S[1..3]");
    return is_func_type(func, {stream_type(9)}, stream_type(3));
}

result stream3_by_int ()
{
    string func("(S) = S[1]");
    return is_func_type(func, {stream_type(9,7,3)}, stream_type(7,3));
}

result stream3_by_range ()
{
    string func("(S) = S[1..4]");
    return is_func_type(func, {stream_type(9,7,3)}, stream_type(4,7,3));
}

result stream3_by_int2 ()
{
    string func("(S) = S[1,2]");
    return is_func_type(func, {stream_type(9,7,3)}, stream_type(3));
}

result stream3_by_range2 ()
{
    string func("(S) = S[1..4, 1..5]");
    return is_func_type(func, {stream_type(9,7,3)}, stream_type(4,5,3));
}

result stream3_by_range_int_range ()
{
    string func("(S) = S[1..4, 2, 1..5]");
    return is_func_type(func, {stream_type(9,7,11)}, stream_type(4,5));
}

}

namespace transposition {

result stream3_to_2()
{
    string func("(S) = S{2}");
    return is_func_type(func, {stream_type(3,5,7)}, stream_type(5,3,7));
}

result stream4_to_3_2()
{
    string func("(S) = S{3,2}");
    return is_func_type(func, {stream_type(3,5,7,9)}, stream_type(7,5,3,9));
}

}

namespace reduction {

string func("(S) = reduce(a,b in S) a + b");

result stream_1d()
{
    return is_func_type(func, {stream_type(10)}, real_type());
}
result stream_1d_to_int()
{
    string func("(S) = reduce(a,b in S) 999");
    return is_func_type(func, {stream_type(10)}, real_type());
}
result stream_1d_to_stream_1d()
{
    string func("(S,X) = reduce(a,b in S) X");
    return is_func_type(func, {stream_type(10), stream_type(5)}, stream_type(5));
}

}

namespace mapping {

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

}
