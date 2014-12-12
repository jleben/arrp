#ifndef STREAM_LANG_TESTING_INCLUDED
#define STREAM_LANG_TESTING_INCLUDED

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <random>
#include <cstdlib>

namespace stream {
namespace testing {

using std::vector;
using std::string;
using std::ostream;

int outcome(bool correct)
{
    using namespace std;
    cout << (correct ? "CORRECT" : "INCORRECT") << endl;
    return correct ? 0 : 1;
}

template<int h> int volume()
{
    return h;
}

template<int h, int t0, int ...t> int volume()
{
    return h * volume<t0,t...>();
}

template<typename T, int ...S>
struct multi_array
{
    multi_array()
    {
        m_data = new T[volume()];
        m_owner = true;
    }

    multi_array(const multi_array & other)
    {
        m_data = new T[volume()];
        m_owner = true;

        int count = volume();
        for (int i = 0; i < count; ++i)
            m_data[i] = other.m_data[i];
    }

    multi_array(const multi_array && other)
    {
        m_data = other.m_data;
        m_owner = other.m_owner;
        other.m_owner = false;
    }

    void operator=(const multi_array & other)
    {
        if (m_owner)
            delete m_data;

        m_data = new T[volume()];
        m_owner = true;

        int count = volume();
        for (int i = 0; i < count; ++i)
            m_data[i] = other.m_data[i];
    }

    void operator=(const multi_array && other)
    {
        if (m_owner)
            delete m_data;

        m_data = other.m_data;
        m_owner = other.m_owner;
        other.m_owner = false;
    }

    ~multi_array()
    {
        if (m_owner)
            delete m_data;
    }

    multi_array(T* d): m_data(d), m_owner(false) {}

    T* data() { return m_data; }
    bool is_owner() { return m_owner; }

    static constexpr int volume() { return testing::volume<S...>(); }

    bool operator==(const multi_array<T,S...> & other)
    {
        int vol = volume();
        for (int i = 0; i < vol; ++i)
        {
            if (m_data[i] != other.m_data[i])
                return false;
        }
        return true;
    }

    template<int size_head, int ...size_tail>
    struct indexer
    {
        template<typename ...int_type>
        static
        int index(int base, int idx_head, int_type ... idx_tail)
        {
            return indexer<size_tail...>::index(idx_head + base * size_head, idx_tail...);
        }
    };
    template<int size_head>
    struct indexer<size_head>
    {
        static
        int index(int base, int idx_head)
        {
            return idx_head + base * size_head;
        }
    };

    template<typename ...int_type>
    T & operator()(int_type ...idxs)
    {
        int idx = indexer<S...>::index(0,idxs...);
        return m_data[idx];
    }

    template<typename ...int_type>
    const T & operator()(int_type ...idxs) const
    {
        int idx = indexer<S...>::index(0,idxs...);
        return m_data[idx];
    }

    static
    multi_array random(double lo, double hi, std::uint32_t seed)
    {
        std::minstd_rand gen(seed);
        std::uniform_real_distribution<> distrib(lo, hi);

        multi_array a;
        for (int i = 0; i < volume(); ++i)
        {
            a.m_data[i] = distrib(gen);
        }

        return a;
    }

private:
    T * m_data;
    mutable bool m_owner;
};

template<typename T, int S0>
inline
ostream & operator<<( ostream & stream, const multi_array<T,S0> & array )
{
    using namespace std;
    for (int i = 0; i < S0; ++i)
        stream << std::setw(4) << array(i) << endl;
    return stream;

}

template<typename T, int S0, int S1>
inline
ostream & operator<<( ostream & stream, const multi_array<T,S0,S1> & array )
{
    using namespace std;
    for (int i0 = 0; i0 < S0; ++i0)
    {
        for (int i1 = 0; i1 < S1; ++i1)
            stream << std::setw(4) << array(i0,i1) << ' ';
        stream << endl;
    }
    return stream;
}

}
}

#endif // STREAM_LANG_TESTING_INCLUDED
