#pragma once
#include <io.hpp>

class program_id;

namespace test { struct traits; }

namespace arrp {
namespace testing {

template<>
class io<test::traits>
{
public:
    void input_coefs_a(double a[2])
    {
        for (int i = 0; i < 2; ++i)
            a[i] = 1.0/(i+1);
    }

    void input_coefs_b(double b[3])
    {
        for (int i = 0; i < 3; ++i)
            b[i] = 1.0/(i+1);
    }

    void input_x(double &v)
    {
        static int t = 0;
        v = t;
        t += 1;
    }
};

}
}
