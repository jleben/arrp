#pragma once

#include <cstddef>
#include <vector>

namespace arrp {

using std::size_t;
using std::vector;

template<typename E>
struct stream_type { typedef E element_type; };

template<typename E>
struct data_traits
{
    using scalar_type = E;

    static void size(vector<int> & v) {}
};

template<typename E, size_t S>
struct data_traits<E[S]>
{
    using scalar_type = typename data_traits<E>::scalar_type;

    static void size(vector<int> & v)
    {
        v.push_back(S);
        data_traits<E>::size(v);
    }
};

template<typename E>
struct data_traits<stream_type<E>>
{
    using scalar_type = typename data_traits<E>::scalar_type;

    static void size(vector<int> & v)
    {
        v.push_back(-1);
        data_traits<E>::size(v);
    }
};

inline
int floor_div(int a, int b)
{
    int d = a / b;
    return d - ((d < 0) & (d * b != a));
}

}
