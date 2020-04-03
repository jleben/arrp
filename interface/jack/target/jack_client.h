#pragma once

#include "buffer.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include <vector>
#include <condition_variable>
#include <mutex>

namespace arrp {
namespace jack_io {

class Jack_Client
{
public:
    Jack_Client();
    ~Jack_Client();

    void receive();

    void send();

    void transmit()
    {
        send();
        receive();
    }

    int frames_to_process() const { return d_frames_to_process; }

private:
    struct Hidden;

    static void* process_thread_cb(void *arg)
    {
        return static_cast<Jack_Client*>(arg)->process_thread();
    }

    static int process_cb (jack_nframes_t nframes, void *arg)
    {
        return static_cast<Jack_Client*>(arg)->process(nframes);
    }

    static void shutdown_cb(void *arg)
    {
        exit(1);
    }

    int process (jack_nframes_t nframes);

    void* process_thread();

    Hidden * d_hidden = nullptr;
    jack_client_t * d_client;
    vector<jack_port_t*> d_inputs;
    vector<jack_port_t*> d_outputs;
    int d_frames_to_process = 0;
};

}
}
