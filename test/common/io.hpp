#pragma once
#include "testing.hpp"

namespace arrp {
namespace testing {

class io_base
{
public:
    template <typename T, size_t S>
    void output(T (&a)[S])
    {
        for (int i = 0; i < S; ++i)
        {
            output(a[i]);
        }
    }

    template <typename T>
    void output(T value)
    {
        m_data.push_back(value);
    }

    const vector<element> & data() const { return m_data; }

private:
    vector<element> m_data;
};

template <typename T>
class io : public io_base {};

}
}
