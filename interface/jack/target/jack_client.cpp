#include "jack_client.h"

#include <iostream>

using namespace std;

namespace arrp {
namespace jack_io {

void printJackStatus(JackStatus status, ostream & out)
{
    if(status & JackServerFailed)
    {
        out << "Unable to connect to JACK server. Is server running?" << endl;
    }
    if (status & JackInvalidOption)
    {
        out << "Invalid options were provided." << endl;
    }
    if (status & JackServerError)
    {
        out << "Error communicating with server." << endl;
    }
    if (status & JackInitFailure)
    {
        out << "Unable to initialize client." << endl;
    }
    if (status & JackShmFailure)
    {
        out << "Unable to access shared memory." << endl;
    }
    if (status & JackVersionError)
    {
        out << "JACK client protocol version is not supported." << endl;
    }
}

Jack_Client::Jack_Client(const string & name, int input_count, int output_count):
    d_input_bufs(input_count),
    d_output_bufs(output_count)
{
    string client_name = name;
    const char *server_name = NULL;
    jack_options_t options = JackNoStartServer;
    jack_status_t status;

    /* open a client connection to the JACK server */

    auto * client = d_client = jack_client_open (client_name.c_str(), options, &status, server_name);
    if (client == NULL)
    {
        cerr << "Failed to create Jack client." << endl;
        printJackStatus(status, cerr);
        exit (1);
    }
    if (status & JackNameNotUnique)
    {
        client_name = jack_get_client_name(client);
        cerr << "Jack client name already in use. Changed to '" << client_name << "'." << endl;
    }

    jack_set_process_thread(client, &Jack_Client::process_thread_cb, this);
    jack_on_shutdown (client, &Jack_Client::shutdown_cb, this);

    /* display the current sample rate. */

    cerr << "Sample rate: " << jack_get_sample_rate (client) << endl;

    // FIXME: Abort if unexpected samplerate?


    /* create ports */

    for (int i = 0; i < d_input_bufs.size(); ++i)
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

    for (int i = 0; i < d_output_bufs.size(); ++i)
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

void * Jack_Client::process_thread()
{
    try
    {
        receive();
        process();
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
    //printf("Receive\n");

    d_frames_to_process = jack_cycle_wait(d_client);

    //printf("Frames = %d\n", d_frames_to_process);

    for (int i = 0; i < d_inputs.size(); ++i)
    {
        auto & buf = d_input_bufs[i];
        if (buf.readable())
            fprintf(stderr, "Error: Input buffer %d not consumed.\n", i);

        float * data = (jack_default_audio_sample_t*) jack_port_get_buffer(d_inputs[i], d_frames_to_process);
        buf = Linear_Buffer<float>(data, d_frames_to_process);
        buf.produce(d_frames_to_process);
    }

    for (int i = 0; i < d_outputs.size(); ++i)
    {
        auto & buf = d_output_bufs[i];
        if (buf.readable())
            fprintf(stderr, "Error: Output buffer %d not consumed.\n", i);

        float * data = (jack_default_audio_sample_t*) jack_port_get_buffer(d_outputs[i], d_frames_to_process);
        buf = Linear_Buffer<float>(data, d_frames_to_process);
    }
}

void Jack_Client::send()
{
    //printf("Send\n");

    for (int i = 0; i < d_outputs.size(); ++i)
    {
        auto & buf = d_output_bufs[i];
        if (buf.readable() != d_frames_to_process)
        {
            fprintf(stderr, "Error: Output buffer %d unexpected data size (%d/%d)\n", i, buf.readable(), d_frames_to_process);
        }
        buf.clear();
    }

    jack_cycle_signal(d_client, 0);
}

void Jack_Client::shutdown_cb(void *arg)
{
    cerr << "JACK server is shutting down this client." << endl;
    exit(1);
}

}
}
