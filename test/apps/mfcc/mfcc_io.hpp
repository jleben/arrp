 
#pragma once
#include <io.hpp>
#include <complex>
#include <fftw3.h>

class program_id;

namespace arrp {
namespace testing {

template<>
class io<test::traits> : public io_base<test::traits>
{
public:
    static const int win_size = 64;
    static const int fft_size = win_size/2 + 1;
    static const int mfcc_size = 10;

    fftw_plan m_dft_plan;
    fftw_plan m_dct_plan;

    io()
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
};

}
}
