/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef STREAM_UTILITY_MAPPING_INCLUDED
#define STREAM_UTILITY_MAPPING_INCLUDED

#include "matrix.hpp"

#include <vector>
#include <iostream>
#include <iomanip>

namespace stream {
namespace utility {

using std::vector;

class mapping
{
public:
    mapping() {}

    mapping(const matrix<int> & coef, const vector<int> & cst):
        coefficients(coef), constants(cst)
    {
        assert(coefficients.rows() == constants.size());
    }

    mapping(int input_dimension, int output_dimension):
        coefficients(output_dimension, input_dimension, 0),
        constants(output_dimension, 0)
    {
    }

    mapping(int size):
        coefficients(size, size, 0),
        constants(size, 0)
    {
    }

    static mapping identity(int in, int out)
    {
        mapping m;
        m.coefficients = matrix<int>::identity(out,in);
        m.constants.resize(out, 0);
        return m;
    }

    static mapping identity(int size)
    {
        mapping m;
        m.coefficients = matrix<int>::identity(size);
        m.constants.resize(size, 0);
        return m;
    }

    static mapping identity_to(const mapping & other )
    {
        return mapping::identity(other.input_dimension());
    }

    bool operator== ( const mapping & other ) const
    {
        return coefficients == other.coefficients && constants == other.constants;
    }

    int coefficient(int in_dim, int out_dim) const
    {
        return coefficients(out_dim, in_dim);
    }
    int & coefficient(int in_dim, int out_dim)
    {
        return coefficients(out_dim, in_dim);
    }

    int constant(int out_dim) const
    {
        return constants[out_dim];
    }
    int & constant(int out_dim)
    {
        return constants[out_dim];
    }

    int input_dimension() const { return coefficients.columns(); }
    int output_dimension() const { return coefficients.rows(); }

    void resize(int in_dim, int out_dim)
    {
        if (in_dim != coefficients.columns() || out_dim != coefficients.rows())
            coefficients = coefficients.resized(out_dim, in_dim);
        if (out_dim != constants.size())
            constants.resize(out_dim, 0);
    }

    void copy(const mapping & other, int in_offset = 0, int out_offset = 0)
    {
        for(int out =  0; out < other.output_dimension(); ++out)
        {
            for (int in = 0; in < other.input_dimension(); ++in)
            {
               coefficients(in_offset + in, out_offset + out) =
                        other.coefficients(in, out);
            }
            constants[out_offset + out] = other.constants[out];
        }
    }

    mapping input_range(int start, int size)
    {
        assert(start >= 0);
        assert(size >= 0);
        assert(start + size <= input_dimension());

        mapping dst(size, output_dimension());
        for( int row = 0; row < output_dimension(); ++row)
        {
            for (int col = 0; col < size; ++col)
            {
                dst.coefficient(row,col) = coefficient(row, start + col);
            }
            dst.constant(row) = constant(row);
        }
        return dst;
    }

    mapping output_range(int start, int size)
    {
        assert(start >= 0);
        assert(size >= 0);
        assert(start + size <= output_dimension());

        mapping dst(input_dimension(), size);
        for( int row = 0; row < size; ++row)
        {
            for (int col = 0; col < input_dimension(); ++col)
            {
                dst.coefficient(row,col) = coefficient(row + start, col);
            }
            dst.constant(row) = constant(row + start);
        }
        return dst;
    }

    void remove_input_dim(int in_dim, int count = 1)
    {
        coefficients.remove_column(in_dim, count);
    }

    void remove_output_dim(int out_dim, int count = 1)
    {
        coefficients.remove_row(out_dim, count);
        constants.erase(constants.begin() + out_dim,
                        constants.begin() + out_dim + count);
    }

    void insert_input_dim(int in_dim, int count = 1)
    {
        coefficients.insert_column(in_dim, count);
    }

    void insert_output_dim(int out_dim, int count = 1)
    {
        coefficients.insert_row(out_dim, count);
        constants.insert(constants.begin() + out_dim, count, 0);
    }

    matrix<int> coefficients;
    vector<int> constants;
};

inline mapping operator*(const mapping & m1, const mapping & m2)
{
    using namespace ::stream::utility;

    mapping dst;
    dst.coefficients = m1.coefficients * m2.coefficients;
    dst.constants = m1.coefficients * m2.constants + m1.constants;
    return dst;
}

inline vector<int> operator*(const mapping & m, const vector<int> & v)
{
    using namespace ::stream::utility;

    return m.coefficients * v + m.constants;
}

inline mapping operator+ (const mapping & m1, const mapping & m2)
{
    assert(m1.input_dimension() == m2.input_dimension());
    assert(m1.output_dimension() == m2.output_dimension());

    using namespace ::stream::utility;

    mapping dst;
    dst.coefficients = m1.coefficients + m2.coefficients;
    dst.constants = m1.constants + m2.constants;

    return dst;
}

inline
std::ostream & operator<<(std::ostream & s, const mapping & m)
{
    for (int row = 0; row < m.output_dimension(); ++row)
    {
        for (int col = 0; col < m.input_dimension(); ++col)
        {
            s << std::setw(4) << m.coefficients(row,col);
        }
        s << " | " << m.constants[row];
        s << std::endl;
    }
    return s;
}

}
}

#endif // STREAM_UTILITY_MAPPING_INCLUDED
