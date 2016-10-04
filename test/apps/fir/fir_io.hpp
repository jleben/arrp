#pragma once
#include <io.hpp>

class program_id;

namespace test { struct traits; }

namespace arrp {
namespace testing {

template<>
class io<test::traits> : public io_base<test::traits>
{
public:
    void input_coefs(double k[3])
    {
        for (int i = 0; i < 3; ++i)
            k[i] = i+1;
    }

    void input_in1(double &v)
    {
        static int t = 0;
        v = t;
        t += 1;
    }
};

}
}
