
#include <io.hpp>

namespace test { struct traits; }

namespace arrp {
namespace testing {

template<>
class io<test::traits>
{
public:
    void f(int in[4], int out[2])
    {
        out[0] = in[0] + in[1];
        out[1] = in[2] + in[3];
    }

    int g(int in)
    {
        return in * 100;
    }

    double f1(int in)
    {
        return double(in) / 2;
    }

    void f2(int in[5], double out[5])
    {
        for(int i = 0; i < 5; ++i)
            out[i] = double(in[i]) / 2;
    }

    void f3(int in, double out[5])
    {
        for(int i = 0; i < 5; ++i)
        {
            out[i] = in / double(i+1);
        }
    }
};

}
}
