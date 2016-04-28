#include "func_types.hpp"

using namespace std;

namespace stream {
namespace functional {

void scalar_type::print(ostream & s) const
{
    s << primitive;
}

void array_type::print(ostream & s) const
{
    s << size;
}

void function_type::print(ostream & s) const
{
    s << 'f' << '(' << arg_count << ')';
}

std::ostream & operator<<(std::ostream & s, const array_size_vec & v)
{
    s <<'[';
    int i = 0;
    for (auto & e : v)
    {
        if (i > 0)
            s << ",";
        if (e != -1)
            s << e;
        else
            s << '*';
        ++i;
    }
    s << ']';
    return s;
}

}
}
