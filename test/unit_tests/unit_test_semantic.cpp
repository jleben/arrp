#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {

using namespace stream::semantic;
using namespace std;

template <typename scalar_type>
bool scalar_matches(const type & actual, const type & expected)
{
    if (actual.get_tag() != expected.get_tag())
        return false;

    const scalar_type & actual_scalar = actual.as<scalar_type>();
    const scalar_type & expected_scalar = expected.as<scalar_type>();
    if (actual_scalar.is_constant() != expected_scalar.is_constant())
        return false;
    if (actual_scalar.is_constant() && expected_scalar.is_constant())
        if (actual_scalar.constant_value() != expected_scalar.constant_value())
            return false;
    return true;
}

bool type_matches(const type & actual, const type & expected)
{
    switch(actual.get_tag())
    {
    case type::integer_num:
        return scalar_matches<integer_num>(actual, expected);
    case type::real_num:
        return scalar_matches<real_num>(actual, expected);
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
