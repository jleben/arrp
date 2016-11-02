#pragma once

#include "mel_coefs.hpp"

//#include <buffer.hpp>
#include <cmath>
#include <complex>
#include <fftw3.h>
#include <arrp.hpp>

namespace test {

using std::complex;

static const int win_size = 1024;
static const int mfcc_size = 15;
static const int fft_size = win_size/2 + 1;
static const double sample_rate = 20000;
static const double mel_scale_lb = 100;
static const double mel_scale_ub = 10000;
static const double freq_hz = 500;
static const double freq = freq_hz/sample_rate;
static const double pi = std::atan(1.0) * 4.0;

struct traits {
    using output_type = arrp::stream_type<double[mfcc_size]>;
    using unit_type = double[mfcc_size];
};

//using buf = f64buf;

#if 0
class sine
{
public:
    sine(double freq, buf & out): out(out) {}

    void run()
    {
        out[0] = std::sin(freq*t*2*pi);
        t += 1;
    }

private:
    double freq;
    buf & out;
    int t = 0;
};
#endif

template<typename IO>
struct alignas(16) program
{
    IO * io;

    alignas(16) double mel_filter_coefs[mfcc_size][fft_size];

    alignas(16) double x[win_size];
    alignas(16) complex<double> spectrum[fft_size];
    alignas(16) double pow_spectrum[fft_size];
    alignas(16) double mel_spectrum[mfcc_size];
    alignas(16) double mfcc[mfcc_size];

    fftw_plan m_dft_plan;
    fftw_plan m_dct_plan;

    int t = 0;

    program()
    {
    }

    void prelude()
    {
        m_dft_plan = fftw_plan_dft_r2c_1d
                (win_size, x, reinterpret_cast<fftw_complex*>(spectrum), 0);

        m_dct_plan = fftw_plan_r2r_1d(mfcc_size, mel_spectrum, mfcc, FFTW_REDFT10, 0);

        mfcc::mel_coefs<mfcc_size,win_size>(sample_rate, mel_scale_lb, mel_scale_ub,
                                            mel_filter_coefs);

        t = 0;
    }

    void period()
    {
        // Sine source
        for (int i = 0; i < win_size; ++i)
        {
            x[i] = std::sin(freq*t*2*pi);
            t += 1;
        }

        // Spectrum
        fftw_execute_dft_r2c(m_dft_plan, x, reinterpret_cast<fftw_complex*>(spectrum));

        // Power spectrum
        for (int i = 0; i < fft_size; ++i)
        {
            auto & x = spectrum[i];
            pow_spectrum[i] = (x.real()*x.real() + x.imag()*x.imag()) / win_size;
        }

        // Mel spectrum
        for (int m = 0; m < mfcc_size; ++m)
        {
            double v = 0;
            for(int b = 0; b < fft_size; ++b)
            {
                v += pow_spectrum[b] * mel_filter_coefs[m][b];
            }
            mel_spectrum[m] = v;
        }

        // Log mel spectrum
        for (int m = 0; m < mfcc_size; ++m)
        {
            mel_spectrum[m] = std::log(mel_spectrum[m] + 0.0001);
        }

        // MFCC
        fftw_execute_r2r(m_dct_plan, mel_spectrum, mfcc);

        io->output(mfcc);
    }
};

}
