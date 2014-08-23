#include "unit_test.hpp"

#include <sstream>

using namespace stream;
using namespace stream::semantic;
using namespace std;

using unit_test::result;

namespace binop {

result test_expression(const char *expr, const type & expected_result_type)
{
    ostringstream code;
    code << "result = " << expr;
    type_ptr result_type =
            unit_test::semantic_analysis(code.str(), "result");
    if (!result_type)
        return unit_test::failure;

    bool ok;

    switch(result_type->get_tag())
    {
    case type::integer_num:
    case type::real_num:
    case type::range:
        ok = result_type->get_tag() == expected_result_type.get_tag();
        break;
    case type::stream:
    {
        ok = expected_result_type.is(type::stream);
        if (ok)
            ok = result_type->as<semantic::stream>().size ==
                    expected_result_type.as<semantic::stream>().size;
        break;
    }
    default:
        ok = false;
    }

    if (ok)
        cerr << "OK." << endl;
    else
        cerr << "FAIL: Result type mismatch:"
             << " expected = " << expected_result_type
             << " / actual = " << *result_type;

    return ok ? unit_test::success : unit_test::failure;
}

result ii() { return test_expression("1 + 2", semantic::integer_num()); }
result ir() { return test_expression("1 + 2.3", semantic::real_num()); }
result ri() { return test_expression("2.3 + 1", semantic::real_num()); }
result rr() { return test_expression("2.3 + 3.4", semantic::real_num()); }
result iR() { return test_expression("1 + 1..10", semantic::stream(10)); }
result rR() { return test_expression("2.3 + 1..10", semantic::stream(10)); }
result Ri() { return test_expression("1..10 + 1", semantic::stream(10)); }
result Rr() { return test_expression("1..10 + 2.3", semantic::stream(10)); }
result RR() { return test_expression("1..10 + 11..20", semantic::stream(10)); }
result iS() { return test_expression("1 + 1..10 * 6", semantic::stream(10)); }
result rS() { return test_expression("2.3 + 1..10 * 6", semantic::stream(10)); }
result Si() { return test_expression("1..10 * 6 + 1", semantic::stream(10)); }
result Sr() { return test_expression("1..10 * 6 + 2.3", semantic::stream(10)); }
result SS() { return test_expression("1..10 * 6 + 11..20 * 7", semantic::stream(10)); }
//result __() { return test_expression("__", semantic::stream(10)); }
}



/*
// Incorrect:

f1 = 1 + 2.. ;
f2 = 2..3 * 2..5 ;
f3 = 2..3 * 7  +  2..5 * 7 ;
*/
