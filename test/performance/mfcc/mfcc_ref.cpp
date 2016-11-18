#pragma once

#include "mel_coefs.hpp"

//#include <buffer.hpp>
#include <cmath>
#include <complex>
#include <fftw3.h>
#include <arrp.hpp>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace test {

using std::complex;

static const int win_size = 1024;
static const int mfcc_size = 15;
static const int fft_size = win_size/2 + 1;
static constexpr size_t fft_size_m16 = fft_size; //(fft_size / 16 + 1) * 16;
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
    alignas(16) double mel_filter_coefs[mfcc_size][fft_size_m16];

    //alignas(16) double x[win_size];
    alignas(16) double x_window[win_size];
    alignas(16) complex<double> spectrum[fft_size];
    alignas(16) double pow_spectrum[fft_size];
    alignas(16) double mel_spectrum[mfcc_size];
    alignas(16) double mfcc[mfcc_size];

    IO * io;

    fftw_plan m_dft_plan;
    fftw_plan m_dct_plan;

    int t = 0;

    static bool is_aligned(void * d, size_t bound)
    {
        return reinterpret_cast<uintptr_t>(d) % bound == 0;
    }

    program()
    {
        using namespace std;
#if 0
        std::vector<void*> ds = {
            mel_filter_coefs,
            x,
            x_window,
            spectrum,
            pow_spectrum,
            mel_spectrum,
            mfcc
        };
        for (auto & d : ds)
        {
            printf("%d\n", (uintptr_t((void*)(d)) % 64));
            //if (!is_aligned(d,64))
                //throw std::runtime_error("Not aligned.");
        }
#endif
    }

    void prelude()
    {
#if 1
        m_dft_plan = fftw_plan_dft_r2c_1d
                (win_size, x_window, reinterpret_cast<fftw_complex*>(spectrum), 0);
#endif
        m_dct_plan = fftw_plan_r2r_1d(mfcc_size, mel_spectrum, mfcc, FFTW_REDFT10, 0);

        mfcc::mel_coefs<mfcc_size,win_size,fft_size_m16>
                (sample_rate, mel_scale_lb, mel_scale_ub, mel_filter_coefs);

        t = 0;
    }

    void period()
    {
        auto & t = this->t;
        //auto & mel_spectrum = this->mel_spectrum;
        //auto spectrum = this->spectrum;
        //auto pow_spectrum = this->pow_spectrum;

#if 0
#pragma ivdep
        for (int i = 0; i < win_size; ++i) // Vectorized (!)
        {
            x[i] = t;
            t = (t + 1) & 127;
        }
#endif
#pragma ivdep
        for (int i = 0; i < win_size; ++i) // Optionally vectorized
        {
            x_window[i] = t;
            t = (t + 1);
        }

        // Spectrum
        fftw_execute_dft_r2c(m_dft_plan, x_window, reinterpret_cast<fftw_complex*>(spectrum));

        // Power spectrum
#pragma ivdep
        for (int i = 0; i < fft_size; ++i) // Not vectorized
        {
            auto & x = spectrum[i];
            pow_spectrum[i] = (x.real()*x.real() + x.imag()*x.imag()) / win_size;
        }

        // Mel spectrum
        for (int m = 0; m < mfcc_size; ++m) // Not vectorized
        {
            alignas(16) double v = 0;
            for(int b = 0; b < fft_size; ++b) // Partially vectorized
            {
                v += pow_spectrum[b] * mel_filter_coefs[m][b];
            }
            mel_spectrum[m] = v;
        }

        // Log mel spectrum
#pragma simd
        for (int m = 0; m < mfcc_size; ++m) // Partially vectorized (!)
        {
            mel_spectrum[m] =
                    //mel_spectrum[m] + 0.0001;
                    std::log(mel_spectrum[m] + 0.0001);
        }

        // MFCC
        fftw_execute_r2r(m_dct_plan, mel_spectrum, mfcc);

        io->output(mfcc);
    }
};

}
