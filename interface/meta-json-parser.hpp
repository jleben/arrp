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

#include <cstdint>
#include <vector>
#include <string>

namespace stream {
namespace meta_json {

using std::vector;
using std::string;

enum numerical_type {
    integer,
    real
};

template<numerical_type>
struct represent;

template<> struct represent<integer> { typedef int type; };
template<> struct represent<real> { typedef double type; };

struct buffer
{
    void *data;
    std::uint32_t phase;
};

struct descriptor
{
    struct channel
    {
        numerical_type type;
        int init_count;
        int period_count;
        vector<int> size;
    };

    struct buffer
    {
        numerical_type type;
        int size;
    };

    struct read_error {
        read_error(const string & what): what(what) {}
        string what;
    };

    vector<channel> inputs;
    channel output;
    vector<buffer> buffers;

    descriptor() {}
    static descriptor from_file( const string & filename );
};

class process
{
public:
    vector<buffer> m_buffers;

    process( const descriptor & description )
    {
        // Allocate buffers
        for( const auto & buffer_info : description.buffers )
        {
            void *data;
            switch(buffer_info.type)
            {
            case integer:
                data = new int[buffer_info.size];
                break;
            case real:
                data = new double[buffer_info.size];
                break;
            }
            m_buffers.push_back(buffer{data, 0});
        }
    }

    template <numerical_type T>
    typename represent<T>::type *output() const
    {
        return reinterpret_cast<typename represent<T>::type*>(m_buffers.back().data);
    }
};

} // namespace interface
} // namespace stream
