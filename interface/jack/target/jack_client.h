#pragma once

#include <arrp/linear_buffer.h>
#include <jack/jack.h>

#include <vector>
#include <condition_variable>
#include <mutex>
#include <string>

namespace arrp {
namespace jack_io {

using std::vector;
using std::string;

class Jack_Client
{
public:
    Jack_Client(const string & name, int input_count, int output_count);
    ~Jack_Client();

    void receive();

    void send();

    void transmit()
    {
        send();
        receive();
    }

    void clock()
    {
        ++d_clock_ticks;

        // Note: Keep looping until we get some frames
        while (d_clock_ticks >= d_frames_to_process)
        {
            d_clock_ticks = 0;
            transmit();
        }
    }

    void input(int i, float & value)
    {
        value = d_input_bufs[i].pop();
    }

    void output(int i, float & value)
    {
        d_output_bufs[i].push(value);
    }

private:
    using Buffer = Linear_Buffer<float>;

    static void* process_thread_cb(void *arg)
    {
        return static_cast<Jack_Client*>(arg)->process_thread();
    }

    static void shutdown_cb(void *arg)
    {
        exit(1);
    }

    void* process_thread();

    virtual void process() = 0;

    jack_client_t * d_client;
    vector<jack_port_t*> d_inputs;
    vector<jack_port_t*> d_outputs;
    int d_frames_to_process = 0;

    vector<Buffer> d_input_bufs;
    vector<Buffer> d_output_bufs;
    int d_clock_ticks = 0;
};

}
}
