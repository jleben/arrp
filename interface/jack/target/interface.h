#pragma once

#include "buffer.h"
#include "jack_client.h"

#include <stdexcept>

namespace arrp {
namespace jack_io {

class Abstract_IO
{
public:
    using Buffer = Circular_Buffer<float>;

    static constexpr int default_buffer_size = 50000;

    Abstract_IO(Jack_Client * jack, int input_count, int output_count):
        d_jack(jack),
        d_inputs(input_count, default_buffer_size),
        d_outputs(output_count, default_buffer_size)
    {
        printf("#inputs: %d, #outputs %d\n", input_count, output_count);
        // FIXME: Properly size buffers
    }

    vector<Buffer> & inputs() { return d_inputs; }
    vector<Buffer> & outputs() { return d_outputs; }

    // To implement in subclass:
    // void process();

protected:
    Jack_Client * d_jack;
    vector<Buffer> d_inputs;
    vector<Buffer> d_outputs;

    template <int S>
    void input(float (&value)[S])
    {
        for (int i = 0; i < S; ++i)
        {
            ensure_readable(d_inputs[i], 1);
            value[i] = d_inputs[i].pop();
        }
    }

    template <int S>
    void output(float (&value)[S])
    {
        for (int i = 0; i < S; ++i)
        {
            ensure_writable(d_outputs[i], 1);
            d_outputs[i].push(value[i]);
        }
    }

    void input(float & value)
    {
        static int i = 0;
        //printf("input %d\n", ++i);
        ensure_readable(d_inputs[0], 1);
        value = d_inputs[0].pop();
    }

    void output(float & value)
    {
        static int i = 0;
        //printf("output %d\n", ++i);
        //printf("output\n");
        ensure_writable(d_outputs[0], 1);
        d_outputs[0].push(value);
    }

    void ensure_readable(Buffer & buf, int frames)
    {
        while(buf.readable() < frames)
        {
            d_jack->transmit();
        }
    }

    void ensure_writable(Buffer & buf, int frames)
    {
        while(buf.readable() >= d_jack->frames_to_process())
        {
            d_jack->transmit();
        }

        if (buf.writable() < frames)
            throw std::runtime_error("Output buffer overflow.");
    }
};

}
}
