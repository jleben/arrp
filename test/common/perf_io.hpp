#pragma once

#include "io.hpp"
#include "testing.hpp"

namespace arrp {
namespace testing {

template <typename traits>
class perf_io : public io<traits>
{
public:
    using output_type = typename traits::output_type;
    using output_unit_type = typename array_traits<output_type>::unit_type;

    void output(output_unit_type & a)
    {
        static volatile double z;
        z = a[0];
    }
};

}
}
