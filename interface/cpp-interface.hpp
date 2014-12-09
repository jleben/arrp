#include <cstdint>
#include <vector>
#include <string>

namespace stream {
namespace interface {

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
