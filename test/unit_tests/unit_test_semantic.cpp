#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {

using namespace stream::semantic;
using namespace std;

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

void test::test_type(const semantic::type_ptr & actual)
{
    if (!do_test_type)
        return;

    type_ptr test_type = actual;
    if (test_type->is(type::function))
        test_type = test_type->as<function>().result_type();

    bool ok = type_matches(*test_type, *m_expected_type);

    if (!ok)
    {
        ostringstream msg;
        msg << "Wrong result type:"
            << " expected = " << *m_expected_type
            << " / actual = " << *test_type;
        throw failure(msg.str());
    }
}

} // namespace unit_testing
} // namespace stream
