 
#pragma once
#include "mel_coefs.hpp"
#include <io.hpp>
#include <complex>
#include <fftw3.h>
#include <iostream>

class program_id;

namespace arrp {
namespace testing {

template<>
class io<test::traits>
{
public:
    static const int win_size = 1024;
    static const int fft_size = win_size/2 + 1;
    static const int mfcc_size = 15;

    fftw_plan m_dft_plan;
    fftw_plan m_dct_plan;

    double sample_rate;
    double freq_lower_bound;
    double freq_upper_bound;

    int t = 0;

    io(): io(20000, 100, 10000)
    {

    }

    io(double sr, double fl, double fh):
        sample_rate(sr),
        freq_lower_bound(fl),
        freq_upper_bound(fh)
    {
        {
            auto in = fftw_alloc_real(win_size);
            auto out = fftw_alloc_complex(fft_size);
            m_dft_plan = fftw_plan_dft_r2c_1d(win_size, in, out, 0);
            fftw_free(in);
            fftw_free(out);
        }
        {
            auto in = fftw_alloc_real(mfcc_size);
            auto out = fftw_alloc_real(mfcc_size);
            m_dct_plan = fftw_plan_r2r_1d(mfcc_size, in, out, FFTW_REDFT10, 0);
        }
    }

    void fft(double in[win_size], complex<double> out[fft_size])
    {
        fftw_execute_dft_r2c(m_dft_plan, in, reinterpret_cast<fftw_complex*>(out));
    }

    void dct(double in[mfcc_size], double out[mfcc_size])
    {
        fftw_execute_r2r(m_dct_plan, in, out);
    }

    void input_x(double & x)
    {
        x = t;
        t = (t + 1);
    }

    void input_mel_coefs(double out[mfcc_size][fft_size])
    {
        using namespace std;

        mfcc::mel_coefs<mfcc_size, win_size>(sample_rate, freq_lower_bound, freq_upper_bound, out);
#if 0
        for (int i = 0; i < mfcc_size; ++i)
        {
            cout << "(";
            for (int j = 0; j < fft_size; ++j)
            {
                cout << out[i][j] << ",";
            }
            cout << ")" << endl;
        }
#endif
    }
};

}
}
