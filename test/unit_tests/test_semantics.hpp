#include "unit_test.hpp"

#include <sstream>

using namespace stream;
using namespace stream::semantic;
using namespace std;

namespace binop {

bool test_expression(const char *expr, const type & expected_result_type)
{
    ostringstream code;
    code << "result = " << expr;
    type_ptr result_type =
            unit_test::semantic_analysis(code.str(), "result");
    if (!result_type)
        return false;

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
        cerr << "OK.";
    else
        cerr << "FAIL: Result type mismatch:"
             << " expected = " << expected_result_type
             << " / actual = " << *result_type;

    return ok;
}

bool ii() { return test_expression("1 + 2", semantic::integer_num()); }
bool ir() { return test_expression("1 + 2.3", semantic::real_num()); }
bool ri() { return test_expression("2.3 + 1", semantic::real_num()); }
bool rr() { return test_expression("2.3 + 3.4", semantic::real_num()); }

}



/*
ii = 1 + 1;
ir = 1 + 2.3;
ri = 2.3 + 1;
rr = 2.3 + 3.4;
iR = 1 + 10..20;
rR = 2.3 + 10..20;
Ri = 10..20 + 1;
Rr = 10..20 + 2.3;
RR = 10..20 + 30..40;
iS(S) = 1 + S;
rS(S) = 2.3 + S;
Si(S) = S + 1;
Sr(S) = S + 2.3;
SS(S) = S + S;

// Incorrect:

f1 = 1 + 2.. ;
f2 = 2..3 * 2..5 ;
f3 = 2..3 * 7  +  2..5 * 7 ;
*/
