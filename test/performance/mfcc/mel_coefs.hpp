#pragma once

#include <cmath>
#include <vector>

namespace mfcc {

using std::vector;

double freq_to_mel(double freq)
{
    return 2595.0 * std::log10(1.0 + freq/700.0);
}

double mel_to_freq(double mel)
{
    return 700.0 * (std::pow(10.0, mel/2595.0) - 1.0);
}

int freq_to_bin(double sr, int n, double f) { return n*f/sr; }

double bin_to_freq(double sr, int n, int b) { return b*sr/n; }

double coef(double l, double c, double h, double x)
{
    if (x < l || x > h)
        return 0;
    else if (x <= c)
        return (x-l)/(c-l);
    else
        return (h-x)/(h-c);
}

vector<double> mel_freqs(double lf, double hf, int n)
{
    double lm = freq_to_mel(lf);
    double hm = freq_to_mel(hf);
    vector<double> m;
    for (int i = 0; i < n; ++i)
    {
        m.push_back(mel_to_freq(i/double(n-1) * (hm - lm) + lm));
    }
    return m;
}

template <int N, int WN>
void mel_coefs(double sr, double fl, double fh, double coefs[N][WN/2+1])
{
    auto freq_grid = mel_freqs(fl, fh, N+2);
#if 0
    for (auto & f : freq_grid)
    {
        cout << f << ", " << endl;
    }
#endif
    for (int n = 0; n < N; ++n)
    {
        for (int k = 0; k < WN/2+1; ++k)
        {
            coefs[n][k] = coef
                    (freq_grid[n], freq_grid[n+1], freq_grid[n+2], bin_to_freq(sr,WN,k));
        }
    }
}

}
