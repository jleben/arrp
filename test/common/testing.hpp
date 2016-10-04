#pragma once

#include <vector>
#include <iostream>
#include <complex>

namespace arrp {
namespace testing {

using std::vector;
using std::ostream;
using std::complex;

enum elem_type {
    bool_type,
    int_type,
    float_type,
    double_type,
    complex_float_type,
    complex_double_type
};


struct element
{
    union {
        bool b;
        int i;
        float f;
        double d;
        complex<float> cf;
        complex<double> cd;
    };

    elem_type type;

    element() {}
    element(bool v): b(v), type(bool_type) {}
    element(int v): i(v), type(int_type) {}
    element(float v): f(v), type(float_type) {}
    element(double v): d(v), type(double_type) {}
    element(const complex<float> & v):
        cf(v), type(complex_float_type) {}
    element(const complex<double> & v):
        cd(v), type(complex_double_type) {}
};

ostream & operator<< (ostream &, const element & e);

class array
{
public:
    array() {}

    array(const vector<int> & size, elem_type type):
        m_size(size), m_type(type), m_elements(total_count(size))
    {}

    elem_type type() const { return m_type; }

    static int total_count(const vector<int> & size)
    {
        int flat_size = 1;
        for (auto & dim_size : size)
            flat_size *= dim_size;
        return flat_size;
    }

    int total_count () const
    {
        return (int) m_elements.size();
    }

    int count() const
    {
        if (m_size.empty())
            return m_elements.size();
        else
            return m_size[0];
    }

    bool is_empty() const { return m_elements.empty(); }

    const vector<int> & size() const { return m_size; }

    void resize(const vector<int> & size)
    {
        m_size = size;
        m_elements.resize(total_count(size));
    }

    void extend(int size)
    {
        if (m_size.empty())
            m_size = { size };
        else
            m_size[0] += size;

        m_elements.resize(total_count(m_size));
    }

    const element & operator()(int index) const
    {
        return m_elements[index];
    }

    element & operator()(int index)
    {
        return m_elements[index];
    }

    element & operator()(const vector<int> & index)
    {
        int flat_index = 0;
        for (int i = 0; i < index.size(); ++i)
        {
            flat_index *= m_size[i];
            flat_index += index[i];
        }
        return m_elements[flat_index];
    }

private:
    vector<int> m_size;
    elem_type m_type;
    vector<element> m_elements;
};

bool compare(const element & a, const element & b);

}
}
