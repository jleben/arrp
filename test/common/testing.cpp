#include "testing.hpp"

#include <cmath>

using namespace std;

namespace arrp {
namespace testing {

bool compare(const element & a, const element & b)
{
    if (a.type != b.type)
        return false;

    switch(a.type)
    {
    case bool_type:
        return a.b == b.b;
    case int_type:
        return a.i == b.i;
    case float_type:
        return std::abs(a.f - b.f) < 0.001;
    case double_type:
        return std::abs(a.d - b.d) < 0.001;
    case complex_float_type:
        return std::abs(a.cf.real() - b.cf.real()) < 0.001 &&
                std::abs(a.cf.imag() - b.cf.imag()) < 0.001;
    case complex_double_type:
        return std::abs(a.cd.real() - b.cd.real()) < 0.001 &&
                std::abs(a.cd.imag() - b.cd.imag()) < 0.001;
    }

    return false;
}

ostream & operator<< (ostream & s, const element & e)
{
    s << e.type << ' ';
    switch(e.type)
    {
    case bool_type:
        s << e.b; break;
    case int_type:
        s << e.i; break;
    case float_type:
        s << e.f; break;
    case double_type:
        s << e.d; break;
    case complex_float_type:
        s << e.cf; break;
    case complex_double_type:
        s << e.cd; break;
    default:
        s << "?"; break;
    }
    return s;
}

}
}
