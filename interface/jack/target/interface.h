#pragma once

#include "jack_client.h"
#include <arrp/linear_buffer.h>

#include <stdexcept>

namespace arrp {
namespace jack_io {

class Abstract_IO
{
public:
    using Buffer = Linear_Buffer<float>;

    Abstract_IO(Jack_Client * jack, int input_count, int output_count):
        d_jack(jack),
        d_inputs(input_count),
        d_outputs(output_count)
    {
    }

    vector<Buffer> & inputs() { return d_inputs; }
    vector<Buffer> & outputs() { return d_outputs; }

    void clock()
    {
        ++d_clock_ticks;

        // Note: Keep looping until we get some frames
        while (d_clock_ticks >= d_jack->frames_to_process())
        {
            d_clock_ticks = 0;
            d_jack->transmit();
        }
    }

protected:
    Jack_Client * d_jack;
    vector<Buffer> d_inputs;
    vector<Buffer> d_outputs;
    int d_clock_ticks = 0;

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
