
#include <io.hpp>

namespace test { struct traits; }

namespace arrp {
namespace testing {

template<>
class io<test::traits>
{
    int d_x = 0;

public:
    void input_x(int & x)
    {
        printf("input: %d\n", d_x);
        x = d_x;
        ++d_x;
    }
};

}
}

