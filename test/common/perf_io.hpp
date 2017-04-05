#pragma once

#include "io.hpp"
#include "testing.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>

namespace arrp {
namespace testing {

using std::ostringstream;

// Printing is added to ensure no computation is optimized away
// and also doubles as a debugging device.

template <typename traits>
class perf_io : public io<traits>
{
    bool m_print = false;

public:
    using output_type = typename traits::output_type;
    using output_unit_type = typename array_traits<output_type>::unit_type;

    bool is_printing() { return m_print; }
    void set_printing(bool value) { m_print = value; }

    template<typename T>
    ALWAYS_INLINE
    void output(T v)
    {
        using namespace std;

        if (m_print)
        {
            ostringstream text;
            print(text, v);
        }
    }

    template <typename T, size_t S>
    ALWAYS_INLINE
    void output(T (&a)[S])
    {
        using namespace std;

        if (m_print)
        {
            ostringstream text;
            print(text, a);
        }
    }

private:
    template <typename T, size_t S>
    void print(ostringstream & text, T (&a)[S])
    {
        using namespace std;
        auto pos = text.tellp();
        for (int i = 0; i < S; ++i)
        {
            text.seekp(pos);
            text << i << ' ';
            print(text, a[i]);
        }
    }

    template <typename T>
    void print(ostringstream & text, T v)
    {
        using namespace std;
        text << ">> " << v << endl;
        cout << text.str();
    }
};

}
}
