#include <cstdint>
#include <vector>
#include <string>

namespace stream {
namespace interface {

using std::vector;
using std::string;

struct buffer
{
    double *data;
    std::uint32_t phase;
};

typedef void (*process_fn)(double** inputs, buffer* buffers);

struct channel
{
    int init_count;
    int period_count;
    vector<int> size;
};

struct descriptor
{
    struct read_error {
        read_error(const string & what): what(what) {}
        string what;
    };

    vector<channel> inputs;
    channel output;
    vector<int> buffers;

    descriptor() {}
    static descriptor from_file( const string & filename );
};

class process
{
    process_fn m_finite_fn;
    process_fn m_period_fn;
    vector<buffer> m_buffers;
public:
    process( const descriptor & description,
             process_fn finite_fn,
             process_fn period_fn ):
        m_finite_fn(finite_fn),
        m_period_fn(period_fn)
    {
        // Allocate buffers
        for( const int & buffer_size : description.buffers )
        {
            int vol = buffer_size;
            double *data = vol ? new double[vol] : nullptr;
            m_buffers.push_back(buffer{data, 0});
        }
    }

    void run(double **inputs)
    {
        m_finite_fn(inputs, m_buffers.data());
    }

    void run_period(double **inputs)
    {
        m_period_fn(inputs, m_buffers.data());
    }

    double *output() const
    {
        return m_buffers.back().data;
    }

private:
    int volume( const vector<int> & v )
    {
        int size;
        if (v.empty())
            size = 0;
        else
        {
            size = v[0];
            for (int i = 1; i < v.size(); ++i)
                size *= v[i];
        }
        return size;
    }
};

} // namespace interface
} // namespace stream
