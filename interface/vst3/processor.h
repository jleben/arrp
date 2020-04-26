#pragma once

#include <public.sdk/source/vst/vstaudioeffect.h>
#include <pcl.h>
#include <vector>
#include <cstdio>

namespace arrp {
namespace vst3 {

namespace Vst = Steinberg::Vst;
using std::vector;

class ProcessorBase : public Vst::AudioEffect
{
public:
    using int32 = Steinberg::int32;
    using TBool = Steinberg::TBool;

    enum Channels
    {
        NoChannels,
        Mono,
        Stereo
    };

    ProcessorBase(Channels inputs, Channels outputs):
        input_channels(inputs),
        output_channels(outputs)
    {
        kernel_stack.resize(1024 * 1024);
    }

    ~ProcessorBase()
    {
        if (has_routine)
            co_delete(kernel_routine);
    }

    static
    Vst::SpeakerArrangement speaker_arrangement(Channels channels)
    {
        switch(channels)
        {
        case Mono:
            return Vst::SpeakerArr::kMono;
        case Stereo:
            return Vst::SpeakerArr::kStereo;
        default:
            return 0;
        }
    }

    static
    int num_channels(Channels channels)
    {
        switch(channels)
        {
        case NoChannels:
            return 0;
        case Mono:
            return 1;
        case Stereo:
            return 2;
        }

        return 0;
    }

    int num_input_buses()
    {
        return input_channels == NoChannels ? 0 : 1;
    }

    int num_output_buses()
    {
        return output_channels == NoChannels ? 0 : 1;
    }

    Steinberg::tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE
    {
        {
            auto result = AudioEffect::initialize (context);
            printf("AudioEffect::initialize: %d\n", result);
            if (result != Steinberg::kResultTrue)
                return Steinberg::kResultFalse;
        }

        if (input_channels != NoChannels)
            addAudioInput (STR16("AudioInput"), speaker_arrangement(input_channels));

        if (output_channels != NoChannels)
            addAudioOutput (STR16("AudioOutput"), speaker_arrangement(output_channels));

        return Steinberg::kResultTrue;
    }

    Steinberg::tresult PLUGIN_API setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns,
                                           Vst::SpeakerArrangement* outputs, int32 numOuts) SMTG_OVERRIDE
    {
        // FIXME: Support changing speaker arrangement while keeping expected
        // number of channels.

        if ( numIns != num_input_buses() or
             numOuts != num_output_buses() or
             (numIns and inputs[0] != speaker_arrangement(input_channels)) or
             numOuts and outputs[0] != speaker_arrangement(output_channels))
        {
            printf("setBusArrangements: No.\n");
            return Steinberg::kResultFalse;
        }

        auto result = AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
        printf("AudioEffect::setBusArrangements: %d\n", result);
        return result;
    }

    Steinberg::tresult PLUGIN_API
    canProcessSampleSize (int32 symbolicSampleSize) SMTG_OVERRIDE
    {
        if (symbolicSampleSize == Vst::kSample32 or
            symbolicSampleSize == Vst::kSample64)
            return Steinberg::kResultTrue;
        else
            return Steinberg::kResultFalse;
    }

    Steinberg::tresult PLUGIN_API setupProcessing (Vst::ProcessSetup& setup) SMTG_OVERRIDE
    {
        sample_rate = setup.sampleRate;
        sample_size_64bit = setup.symbolicSampleSize == Vst::kSample64;

        printf("setupProcessing: SR %f, sample size %d\n", sample_rate, setup.symbolicSampleSize);

        return AudioEffect::setupProcessing (setup);
    }

    Steinberg::tresult PLUGIN_API setActive (TBool active) SMTG_OVERRIDE
    {
        printf("setActive %d\n", active);

        if (active)
        {
            if (has_routine)
                co_delete(kernel_routine);

            kernel_routine =
                    co_create(kernel_routine_func, this,
                              kernel_stack.data(), kernel_stack.size());
            has_routine = true;

            create_kernel();
        }
        else
        {
            if (has_routine)
                co_delete(kernel_routine);

            has_routine = false;
        }

        return AudioEffect::setActive(active);
    }

    Steinberg::tresult PLUGIN_API setProcessing (TBool enabled) SMTG_OVERRIDE
    {
        printf("setProcessing %d\n", enabled);

        // FIXME: Should restart processing here, but in a lock-free manner.

        return Steinberg::kResultOk;
    }

    Steinberg::tresult PLUGIN_API process (Vst::ProcessData& data) SMTG_OVERRIDE
    {
        // numInputs, numOutputs and numSamples can be 0
        // which is used to only update parameters without processing audio;

        if (data.numInputs != num_input_buses() or
            data.numOutputs != num_output_buses() or
            data.numSamples < 1)
        {
            return Steinberg::kResultOk;
        }

        process_data = &data;
        current_frame = 0;

        co_call(kernel_routine);

        return Steinberg::kResultOk;
    }

    //Steinberg::tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
    //Steinberg::tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;

    void clock()
    {
        ++current_frame;
        if (current_frame >= process_data->numSamples)
            co_resume();
    }

    // FIXME: Optimize selecting 64bit/32bit processing

    double input(int i)
    {
        auto & bus = process_data->inputs[0];

        if (sample_size_64bit)
            return bus.channelBuffers64[i][current_frame];
        else
            return bus.channelBuffers32[i][current_frame];
    }

    void output(int i, double value)
    {
        auto & bus = process_data->outputs[0];

        if (sample_size_64bit)
            bus.channelBuffers64[i][current_frame] = value;
        else
            bus.channelBuffers32[i][current_frame] = value;
    }

    template <typename T>
    void input_samplerate(T & value)
    {
        value = sample_rate;
    }

protected:
    static void kernel_routine_func(void * data)
    {
        auto * processor = static_cast<ProcessorBase*>(data);
        processor->kernel_process();
    }

    virtual void create_kernel() = 0;
    virtual void kernel_process() = 0;

    const Channels input_channels;
    const Channels output_channels;

    bool has_routine = false;
    coroutine_t kernel_routine;
    vector<char> kernel_stack;

    double sample_rate = 0;
    bool sample_size_64bit = false;

    Vst::ProcessData * process_data = nullptr;
    int current_frame = 0;
};

// To be generated for kernel
ProcessorBase * create_processor();

}
}
