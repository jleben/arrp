#pragma once

#include <arrp/linear_buffer.h>
#include <pcl.h>
#include <m_pd.h>

namespace arrp {
namespace puredata_io {

class Abstract_IO
{
public:
    using Buffer = Linear_Buffer<float>;

    Abstract_IO(int input_count, int output_count):
        d_inputs(input_count),
        d_outputs(output_count)
    {
    }

    virtual ~Abstract_IO() {}

    vector<Buffer> & inputs() { return d_inputs; }
    vector<Buffer> & outputs() { return d_outputs; }

    void clock()
    {
        ++d_elapsed_ticks;
        if(d_elapsed_ticks >= d_buffer_size)
        {
            co_resume();
            d_elapsed_ticks = 0;
        }
    }

    // Always (re)starts processing from initial state.
    void process(int buf_size)
    {
        d_buffer_size = buf_size;
        d_elapsed_ticks = 0;

        prologue();

        for(;;)
        {
            period();
        }
    }

protected:
    virtual void prologue() = 0;
    virtual void period() = 0;

    vector<Buffer> d_inputs;
    vector<Buffer> d_outputs;
    int d_buffer_size = 0;
    int d_elapsed_ticks = 0;

    template <int S>
    void input(float (&value)[S])
    {
        for (int i = 0; i < S; ++i)
        {
            value[i] = d_inputs[i].pop();
        }
    }

    template <int S>
    void output(float (&value)[S])
    {
        for (int i = 0; i < S; ++i)
        {
            d_outputs[i].push(value[i]);
        }
    }

    void input(float & value)
    {
        value = d_inputs[0].pop();
    }

    void output(float & value)
    {
        d_outputs[0].push(value);
    }
};

}
}

