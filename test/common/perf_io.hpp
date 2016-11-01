#pragma once
#include "testing.hpp"

namespace arrp {
namespace testing {

template<typename program_traits>
class perf_io_base
{
public:
    using output_type = typename program_traits::output_type;
    using output_unit_type = typename array_traits<output_type>::unit_type;

    void output(output_unit_type & a)
    {
    }
};

template <typename traits>
class perf_io : public perf_io_base<traits> {};

}
}
