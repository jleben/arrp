#pragma once

#include <arrp.hpp>
#include <sndfile.hh>
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

template <typename traits>
class wav_interface
{
private:
    SndfileHandle m_out_file;
    int m_num_channels = 0;
    int64_t m_output_count = 0;

public:
    wav_interface(int sample_rate, const string & out_file_name)
    {
        using namespace std;

        vector<int> out_size;
        arrp::data_traits<typename traits::output_type>::size(out_size);

        if (out_size.size() < 1)
            throw std::runtime_error("Program output is not a stream.");

        int num_channels = 1;
        for(int i = 1; i < out_size.size(); ++i)
            num_channels *= out_size[i];

        m_num_channels = num_channels;

        cerr << "Creating sound file with " << m_num_channels << " channels." << endl;

        m_out_file = SndfileHandle(out_file_name, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16,
                                    num_channels, sample_rate);

        if (!m_out_file || m_out_file.error())
        {
            throw std::runtime_error("Failed to open output file.");
        }
    }

    int64_t output_count() const { return m_output_count; }

    template <typename T>
    void output(T value)
    {
        m_out_file.write(&value, 1);
        ++m_output_count;
    }

    template <typename T>
    void output(T * values)
    {
        m_out_file.write(values, m_num_channels);
        ++m_output_count;
    }
};
