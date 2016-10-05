#pragma once
#include "testing.hpp"
#include <iostream>
#include <cassert>

namespace arrp {
namespace testing {

template<typename program_traits>
class io_base
{
public:
    using output_type = typename program_traits::output_type;
    using output_unit_type = typename array_traits<output_type>::unit_type;

    io_base()
    {
        using namespace std;

        vector<int> size;
        array_size<output_type>::get_size(size);

        if (array_traits<output_type>::is_stream)
        {
            assert(!size.empty());
            size[0] = 0;
        }

        m_data.resize(size);

#if 0
        cout << "Allocating size: ";
        cout << "[";
        for (auto & s : size)
                cout << s << " ";
        cout << "]";
#endif
    }

    void output(output_unit_type & a)
    {
        using namespace std;

        int offset;

        if (array_traits<output_type>::is_stream)
        {
            offset = m_data.total_count();
            m_data.extend(1);
        }
        else
        {
            offset = 0;
        }

        store(a, offset);
    }

    const array & data() const { return m_data; }

private:
    template <typename T, size_t S>
    void store(T (&a)[S], int & offset)
    {
        for(int i = 0; i < S; ++i)
            store(a[i], offset);
    }

    template <typename T>
    void store(T value, int & offset)
    {
        using namespace std;
        //cout << "Storing value at " << offset << endl;
        m_data(offset) = value;
        ++offset;
    }

    array m_data;
};

template <typename traits>
class io : public io_base<traits> {};

}
}
