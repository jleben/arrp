#ifndef STREAM_UTILITY_MATRIX_INCLUDED
#define STREAM_UTILITY_MATRIX_INCLUDED

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>

namespace stream {
namespace utility {

using std::vector;
using std::pair;

template<typename T>
class matrix
{

public:
    typedef int index_type;
    typedef T value_type;

    typedef T* iterator;
    typedef const T* const_iterator;

    template<typename U, std::size_t rows, std::size_t columns>
    matrix( U const (&array) [rows][columns]):
        m_rows(rows),
        m_columns(columns),
        m_storage(rows * columns, 0)
    {
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < columns; ++c)
            {
                m_storage[r*columns + c] = array[r][c];
            }
        }
    }

    matrix():
        m_rows(0),
        m_columns(0)
    {}

    matrix(int rows, int columns):
        m_rows(rows),
        m_columns(columns),
        m_storage(rows * columns)
    {
    }

    matrix(int rows, int columns, const T & value):
        m_rows(rows),
        m_columns(columns),
        m_storage(rows * columns, value)
    {
    }

    static matrix<T> identity( int rows, int columns )
    {
        matrix<T> m(rows, columns, 0);
        for (int i = 0; i < std::min(rows, columns); ++i)
            m(i,i) = 1;
        return m;
    }

    int rows() const { return m_rows; }

    int columns() const { return m_columns; }

    T & operator()(int row, int column)
    {
        return m_storage[row * m_columns + column];
    }

    const T & operator()(int row, int column) const
    {
        return const_cast<matrix&>(*this)(row, column);
    }

    std::vector<T> row(int index) const
    {
        std::vector<T> vec;
        vec.reserve(m_columns);
        for (int c = 0; c < m_columns; ++c)
            vec.push_back( (*this)(index, c) );
        return vec;
    }

    std::vector<T> column(int index) const
    {
        std::vector<T> vec;
        vec.reserve(m_rows);
        for (int r = 0; r < m_rows; ++r)
            vec.push_back( (*this)(r, index) );
        return vec;
    }

    void operator*= (const T & multiplier)
    {
        for (auto & item : m_storage)
            item *= multiplier;
    }

    // iterator support

    iterator begin()
    {
        return m_storage.data();
    }

    iterator end()
    {
        return m_storage.data() + m_rows * m_columns;
    }

    const_iterator begin() const
    {
        return m_storage.data();
    }

    const_iterator end() const
    {
        return m_storage.data() + m_rows * m_columns;
    }

    matrix<T> reordered( vector<int> row_order )
    {
        matrix<T> dst(row_order.size(), columns());
        for (int dst_row = 0; dst_row < row_order.size(); ++dst_row)
        {
            int src_row = row_order[dst_row];
            assert(src_row >= 0 && src_row < m_rows);
            for (int col = 0; col < m_columns; ++col)
                dst(dst_row, col) = (*this)(src_row, col);
        }
        return dst;
    }

    matrix<T> resized( int rows, int columns )
    {
        matrix<T> dst(rows, columns, 0);
        for (int row = 0; row < std::min(rows, m_rows); ++row)
            for (int col = 0; col < std::min(columns, m_columns); ++col)
                dst(row,col) = (*this)(row,col);
        return dst;
    }

private:
    int m_rows;
    int m_columns;
    std::vector<T> m_storage;
};

template <typename T>
vector<T> operator*( const matrix<T> & m, const vector<T> & v)
{
    assert(m.columns() == v.size());

    vector<T> v_(m.rows());

    for(int row = 0; row < m.rows(); ++row)
    {
        T value = 0;
        for(int col = 0; col < m.columns(); ++col)
            value += m(row,col) * v[col];
        v_[row] = value;
    }

    return v_;
}

template <typename T>
matrix<T> operator*( const matrix<T> & m1, const matrix<T> & m2 )
{
    assert(m1.columns() == m2.rows());

    matrix<T> dst(m1.rows(), m2.columns());

    for (int row = 0; row < m1.rows(); ++row)
    {
        for (int col = 0; col < m2.columns(); ++col)
        {
            T value = 0;
            for (int i = 0; i < m2.rows(); ++i)
            {
                value += m1(row,i) * m2(i,col);
            }
            dst(row,col) = value;
        }
    }

    return dst;
}

template <typename T>
vector<T> operator+( const vector<T> & v1, const vector<T> & v2 )
{
    assert(v1.size() == v2.size());
    vector<T> dst(v1.size());
    for (unsigned int i = 0; i < dst.size(); ++i)
        dst[i] = v1[i] + v2[i];
    return dst;
}

template <typename T>
pair< matrix<T>, vector<T> >
compose( const matrix<int> & map1, const vector<T> & offset1,
         const matrix<int> & map2, const vector<T> & offset2 )
{
    pair< matrix<T>, vector<T> > dst;
    dst.first = map1 * map2;
    dst.second = map1 * offset2 + offset1;
    return dst;
}

template <typename T>
inline std::ostream & operator<< ( std::ostream & stream, const matrix<T> & m )
{
    for (int r = 0; r < m.rows(); ++r)
    {
        for (int c = 0; c < m.columns(); ++c)
        {
            std::cout << m(r,c) << '\t';
        }
        std::cout << std::endl;
    }
    return stream;
}

}
}

#endif
