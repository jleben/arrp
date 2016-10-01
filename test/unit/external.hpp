
#include <io.hpp>

class program_id;

namespace arrp {
namespace testing {

template<>
class io<program_id> : public io_base
{
public:
    void f(int in[4], int out[2])
    {
        out[0] = in[0] + in[1];
        out[1] = in[2] + in[3];
    }

    void g(int & in, int & out)
    {
        out = in * 100;
    }
};

}
}
