#pragma once
#include "arrp.hpp"
#include "testing.hpp"
#include <iostream>

namespace arrp {
namespace testing {

template<typename T> struct io_traits
{
    typedef T unit_type;
    static const bool is_stream = false;
};

template<typename E> struct io_traits<stream_type<E>>
{
    typedef E unit_type;
    static const bool is_stream = true;
};

template<typename T>
struct array_size
{
    static void get_size(vector<int> & s) {}
};

template<typename E, size_t S>
struct array_size<E[S]>
{
    static void get_size(vector<int> & s)
    {
        s.push_back(S);
        array_size<E>::get_size(s);
    }
};

template<typename E>
struct array_size<stream_type<E>>
{
    static void get_size(vector<int> & s)
    {
        s.push_back(0);
        array_size<E>::get_size(s);
    }
};

template<typename program_traits>
class io_base
{
public:
    using output_type = typename program_traits::output_type;
    using output_unit_type = typename io_traits<output_type>::unit_type;

    io_base()
    {
        using namespace std;

        vector<int> size;
        array_size<output_type>::get_size(size);
        m_data.resize(size);

        cout << "Allocating size: ";
        cout << "[";
        for (auto & s : size)
                cout << s << " ";
        cout << "]";
    }

    void output(output_unit_type & a)
    {
        using namespace std;

        int offset;

        if (io_traits<output_type>::is_stream)
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
#if 0
    template <typename T, size_t S>
    void get_size (T (&a)[S], vector<int> & s)
    {
        s.push_back(S);
        get_size(a[0], s);
    }

    template <typename T>
    void get_size (T v, vector<int> &s)
    {}
#endif
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
        cout << "Storing value at " << offset << endl;
        m_data(offset) = value;
        ++offset;
    }

    array m_data;
};

template <typename traits>
class io : public io_base<traits> {};

}
}
