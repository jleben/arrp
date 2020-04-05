// NOTE: This file is processed by CMake

#include <arrp/jack_io/jack_client.h>

#include @ARRP_JACK_IO_HEADER@
#include <iostream>

using namespace std;

namespace arrp {
namespace jack_io {

struct Jack_Client::Hidden
{
    Kernel * d_kernel;
};

Jack_Client::Jack_Client()
{
    d_hidden = new Hidden;
    d_hidden->d_kernel = new Kernel();
    d_hidden->d_kernel->io = new IO(this);

    string client_name = d_hidden->d_kernel->io->name();
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    /* open a client connection to the JACK server */

    auto * client = d_client = jack_client_open (client_name.c_str(), options, &status, server_name);
    if (client == NULL)
    {
        cerr << "jack_client_open() failed, " << "status = " << status << endl;
        if (status & JackServerFailed)
        {
            cerr << "Unable to connect to JACK server.";
        }
        exit (1);
    }
    if (status & JackServerStarted)
    {
        cerr << "JACK server started." << endl;
    }
    if (status & JackNameNotUnique)
    {
        client_name = jack_get_client_name(client);
        cerr << "Unique name '" << client_name << "' assigned." << endl;
    }

    //jack_set_process_callback (client, &Jack_Client::process_cb, this);
    jack_set_process_thread(client, &Jack_Client::process_thread_cb, this);
    jack_on_shutdown (client, &Jack_Client::shutdown_cb, this);

    /* display the current sample rate. */

    cerr << "Sample rate: " << jack_get_sample_rate (client) << endl;

    // FIXME: Abort if unexpected samplerate?


    /* create ports */

    for (int i = 0; i < d_hidden->d_kernel->io->inputs().size(); ++i)
    {
        auto name = string("input") + to_string(i);
        auto * port = jack_port_register (client, name.c_str(),
                                         JACK_DEFAULT_AUDIO_TYPE,
                                         JackPortIsInput, 0);
        if (!port)
        {
            throw std::runtime_error("Failed to create input port.");
        }

        d_inputs.push_back(port);
    }

    for (int i = 0; i < d_hidden->d_kernel->io->outputs().size(); ++i)
    {
        auto name = string("output") + to_string(i);
        auto * port = jack_port_register (client, name.c_str(),
                                          JACK_DEFAULT_AUDIO_TYPE,
                                          JackPortIsOutput, 0);
        if (!port)
        {
            throw std::runtime_error("Failed to create output port.");
        }

        d_outputs.push_back(port);
    }

    /* Tell the JACK server that we are ready to roll.  Our
         * process() callback will start running now. */

    if (jack_activate (client))
    {
        throw std::runtime_error("Failed to activate client.");
    }

    /* Connect the ports.  You can't do this before the client is
         * activated, because we can't make connections to clients
         * that aren't running.
         */

    const char **ports = jack_get_ports (client, NULL, NULL,
                                         JackPortIsPhysical|JackPortIsOutput);
    if (ports != NULL)
    {
        for (int i = 0; i < d_inputs.size() and ports[i] != nullptr; ++i)
        {
            if (jack_connect(client, ports[i], jack_port_name(d_inputs[i]))) {
                cerr << "cannot connect input ports" << endl;
            }
        }
        free (ports);
    }


    ports = jack_get_ports (client, NULL, NULL,
                            JackPortIsPhysical|JackPortIsInput);
    if (ports != NULL)
    {
        for (int i = 0; i < d_outputs.size() and ports[i] != nullptr; ++i)
        {
            if (jack_connect(client, jack_port_name(d_outputs[i]), ports[i])) {
                cerr << "cannot connect output ports" << endl;
            }
        }

        free (ports);
    }
}

Jack_Client::~Jack_Client()
{
    jack_client_close (d_client);
}

#if 0
int
Jack_Client::process (jack_nframes_t nframes)
{
    jack_default_audio_sample_t *in, *out;

    for (int i = 0; i < d_inputs.size(); ++i)
    {
        auto * buf = jack_port_get_buffer (d_inputs[i], nframes);
        for (int f = 0; f < nframes; ++f)
        {
            d_intf.input(i).push(buf[f]);
        }
    }

    // FIXME: while input buffers have enough data
    d_intf.process();

    for (int i = 0; i < d_outputs.size(); ++i)
    {
        auto * buf = jack_port_get_buffer (d_outputs[i], nframes);
        int zeros = std::max(0, nframes - d_intf.output(i).readable());
        for (int f = 0; f < zeros; ++f)
        {
            buf[f] = 0;
        }
        for (int f = zeros; f < nframes; ++f)
        {
            buf[f] = d_intf.output(i).pop();
        }
    }

    return 0;
}
#endif

void * Jack_Client::process_thread()
{
    receive();

    try
    {
        d_hidden->d_kernel->prelude();

        for(;;)
        {
            d_hidden->d_kernel->period();
        }
    }
    catch (std::runtime_error & e)
    {
        cerr << "Arrp Jack Client: Error: " << e.what() << endl;
        exit(1);
    }

    exit(0);

    return nullptr;
}


void Jack_Client::receive()
{
    printf("Receive\n");

    d_frames_to_process = jack_cycle_wait(d_client);

    printf("Frames = %d\n", d_frames_to_process);

    for (int i = 0; i < d_inputs.size(); ++i)
    {
        auto & buf = d_hidden->d_kernel->io->inputs()[i];
        float * data = (jack_default_audio_sample_t*) jack_port_get_buffer(d_inputs[i], d_frames_to_process);
        buf = Linear_Buffer<float>(data, d_frames_to_process);
        buf.produce(d_frames_to_process);
    }

    for (int i = 0; i < d_outputs.size(); ++i)
    {
        auto & buf = d_hidden->d_kernel->io->outputs()[i];
        float * data = (jack_default_audio_sample_t*) jack_port_get_buffer(d_outputs[i], d_frames_to_process);
        buf = Linear_Buffer<float>(data, d_frames_to_process);
    }
}

void Jack_Client::send()
{
    printf("Send\n");

    for (int i = 0; i < d_outputs.size(); ++i)
    {
        auto & buf = d_hidden->d_kernel->io->outputs()[i];
        if (buf.readable() != d_frames_to_process)
        {
            printf("Buffer underrun for output %d.\n", i);
        }
        buf.clear();
    }

    jack_cycle_signal(d_client, 0);
}

}
}
