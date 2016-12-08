#pragma once

#include "io.hpp"
#include "testing.hpp"

namespace arrp {
namespace testing {

static volatile double z;

template <typename traits>
class perf_io : public io<traits>
{
public:
    using output_type = typename traits::output_type;
    using output_unit_type = typename array_traits<output_type>::unit_type;

    template<typename T,size_t S>
    void output(T (&a)[S])
    {
        output(a[0]);
    }

    template<typename T>
    void output(T v)
    {
        z = v;
    }
};

}
}
