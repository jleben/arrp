#pragma once

#include <vector>
#include <iostream>

namespace arrp {
namespace testing {

using std::vector;
using std::ostream;

enum elem_type {
    bool_type,
    int_type,
    float_type,
    double_type
};


struct element
{
    union {
        bool b;
        int i;
        float f;
        double d;
    };

    elem_type type;

    element() {}
    element(bool v): b(v), type(bool_type) {}
    element(int v): i(v), type(int_type) {}
    element(float v): f(v), type(float_type) {}
    element(double v): d(v), type(double_type) {}
};

ostream & operator<< (ostream &, const element & e);

class array
{
public:
    array(const vector<int> & size, elem_type type):
        m_size(size), m_type(type) {}

    const vector<int> & size() const { return m_size; }

    element & operator()(int index)
    {
        return m_elements[index];
    }

    element & operator()(const vector<int> & index)
    {
        int flat_index = index[0];
        for (int i = 1; i < index.size(); ++i)
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
