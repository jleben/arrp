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
type_ptr stream_type( const vector<int> & size)
{ return make_shared<semantic::stream>(size); }
type_ptr stream_type( int size)
{ return make_shared<semantic::stream>(size); }

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

result is_expr_type(const char *expr, const type & expected_type)
{
    ostringstream code;
    code << "result = " << expr;
    type_ptr result_type =
            unit_test::semantic_analysis(code.str(), "result");
    if (!result_type)
        return unit_test::failure;

    bool ok = type_matches(*result_type, expected_type);

    if (ok)
        return unit_test::success_msg();
    else
    {
        ostringstream msg;
        msg << "Wrong result type:"
            << " expected = " << expected_type
            << " / actual = " << *result_type;
        return failure_msg(msg.str());
    }
}

result is_func_type(const string &func,
                    const vector<type_ptr> & arg_types,
                    const type & expected_type)
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

    if (!type_matches(*result_type, expected_type))
    {
        ostringstream msg;
        msg << "Wrong result type:"
            << " expected = " << expected_type
            << " / actual = " << *result_type;
        return failure_msg(msg.str());
    }

    return success_msg();
}

namespace binop {

result ii() { return is_expr_type("1 + 2", semantic::integer_num()); }
result ir() { return is_expr_type("1 + 2.3", semantic::real_num()); }
result ri() { return is_expr_type("2.3 + 1", semantic::real_num()); }
result rr() { return is_expr_type("2.3 + 3.4", semantic::real_num()); }
result iR() { return is_expr_type("1 + 1..10", semantic::stream(10)); }
result rR() { return is_expr_type("2.3 + 1..10", semantic::stream(10)); }
result Ri() { return is_expr_type("1..10 + 1", semantic::stream(10)); }
result Rr() { return is_expr_type("1..10 + 2.3", semantic::stream(10)); }
result RR() { return is_expr_type("1..10 + 11..20", semantic::stream(10)); }
result iS() { return is_expr_type("1 + 1..10 * 6", semantic::stream(10)); }
result rS() { return is_expr_type("2.3 + 1..10 * 6", semantic::stream(10)); }
result Si() { return is_expr_type("1..10 * 6 + 1", semantic::stream(10)); }
result Sr() { return is_expr_type("1..10 * 6 + 2.3", semantic::stream(10)); }
result SS() { return is_expr_type("1..10 * 6 + 11..20 * 7", semantic::stream(10)); }

}

namespace reduction {

result stream_1d()
{
    string func("(S) = reduce(a,b in S) a + b");
    return is_func_type(func, {stream_type(10)}, real_num());
    //return is_expr_type("reduce(a,b in 1..10 * 2) a + b", real_num());
}
/*
result stream_to_int_const()
{
    return is_expr_type("reduce(a,b in 1..10 * 2) 999", real_num());
}*/

}

namespace mapping {

}

