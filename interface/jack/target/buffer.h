#pragma once

#include <vector>
#include <cstdio>

namespace arrp {
namespace jack_io {

using std::vector;

template <typename T>
class Circular_Buffer
{
    int read_pos = 0;
    int count = 0;
    vector<T> data;

public:

    Circular_Buffer(int size): data(size) {
        printf("Buffer size %d\n", size);
    }

    int write_pos() { return (read_pos + count) % data.size(); }

    void push(T val)
    {
        data[write_pos()] = val;
        ++count;
    }

    int readable() const
    {
        return count;
    }

    int writable() const
    {
        return data.size() - count;
    }

    T pop()
    {
        int p = read_pos;
        read_pos = (read_pos + 1) % data.size();
        --count;
        return data[p];
    }
};

}
}
