#pragma once

#include <vector>

namespace arrp {

using std::vector;

template <typename T>
class Linear_Buffer
{
private:
    vector<T> m_storage;
    T * m_data = nullptr;
    int m_capacity { 0 };
    int m_readPos { 0 };
    int m_writePos { 0 };

public:
    Linear_Buffer() {}

    Linear_Buffer(T * data, int size):
        m_data(data), m_capacity(size)
    {}

    Linear_Buffer(int capacity):
        m_storage(capacity), m_data(m_storage.data()), m_capacity(capacity)
    {}

    T * data() { return m_data; }
    const T * data() const { return m_data; }

    int size() const { return m_capacity; }

    T & operator[](int i) { return m_data[i]; }
    const T & operator[](int i) const { return m_data[i]; }

    int readable() const { return m_writePos - m_readPos; }
    int writable() const { return m_capacity - m_writePos; }

    int readPos() const { return m_readPos; }
    int writePos() const { return m_writePos; }

    void shift()
    {
        int count = m_writePos - m_readPos;
        for (int i = 0; i < count; ++i)
        {
            m_data[i] = m_data[m_readPos + i];
        }
        m_readPos = 0;
        m_writePos = count;
    }

    void clear()
    {
        m_readPos = 0;
        m_writePos = 0;
    }

    void push(const T & v)
    {
        m_data[m_writePos] = v;
        ++m_writePos;
    }

    T pop()
    {
        int r = m_readPos;
        ++m_readPos;
        return m_data[r];
    }

    void produce(int count)
    {
        m_writePos += count;
    }

    void consume(int count)
    {
        m_readPos += count;
    }
};

}
