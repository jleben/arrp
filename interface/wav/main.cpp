#include "interface.hpp"
#include <arguments.hpp> // https://github.com/jleben/cpp-arguments
#include <sndfile.hh>
#include <iostream>

#include PROGRAM_HEADER

using traits = PROGRAM_NAMESPACE::traits;
using interface_type = wav_interface<traits>;
using program_type = PROGRAM_NAMESPACE::program<interface_type>;

using namespace std;

int main(int argc, char * argv[])
{
    float duration_sec = 1.f;
    //int block_size;
    int sample_rate = 48000;
    string output_path = "output.wav";

    Arguments::Parser parser;
    parser.add_option("-dur", duration_sec);
    //parser.add_option("-block", block_size);
    parser.add_option("-sr", sample_rate);
    parser.add_option("-o", output_path);
    parser.parse(argc, argv);

    int64_t duration_frames = duration_sec * sample_rate;

    cerr << "Total duration " << duration_sec << " s"
         << ", count " << duration_frames << " frames."
         << endl;

    auto p = new program_type;
    p->io = new interface_type(sample_rate, output_path);

    p->prelude();

    //printf("Count: %ld Duration: %f\n", p->io->output_count(), p->io->output_count() / float(sample_rate));

    while(p->io->output_count() < duration_frames)
    {
        //printf("Count: %ld Duration: %f\n", p->io->output_count(), p->io->output_count() / float(sample_rate));
        p->period();
    }

    delete p->io;
    delete p;
}
