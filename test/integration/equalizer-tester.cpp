#include "equalizer-kernel.h"
#include "equalizer.hpp"
#include <cmath>
#include <iostream>
#include <random>

using namespace std;

const double pi = 3.14159265358979323846;

class biquad
{
public:
    biquad(const vector<double> & k): k(k), w1(0), w2(0) {}

    double operator()(double x)
    {
        double w = x - w1 * k[0] - w2 * k[1];
        double y = w * k[2] + w1 * k[3] + w2 * k[4];

        w2 = w1;
        w1 = w;

        return y;
    }

private:
    vector<double> k;
    double w1, w2;
};

void reference_process(const double *x, double *y, unsigned int N)
{
    biquad lp(equalizer::low_pass_coef);
    biquad hp(equalizer::high_pass_coef);
    biquad bp(equalizer::band_stop_coef);

    for(int i = 0; i < N; ++i)
    {
        double z = x[i];
        z = lp(z);
        z = hp(z);
        z = bp(z);
        y[i] = z;
    }
}

static double *input = nullptr;
static int input_index = 0;

static void input_func(int index, double *data)
{
    //cout << "input: " << input_index << endl;
    *data = input[input_index];
    ++input_index;
}

int main()
{
    const int N = 1000;

    double x[N];
    double y[N];

    input = x;
    input_index = 0;

    {
        //const double freq = 0.03;

        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_real_distribution<double> uniform_dist(-1, 1);

        for(int i = 0; i < N; ++i)
            x[i] = uniform_dist(gen);
            //x[i] = std::sin(i * freq * pi);
    }

    equalizer::buffer buf;
    equalizer::allocate(&buf);
    buf.input_func = (void*) &input_func;

    for(int i = 0; i < N; ++i)
    {
        equalizer::process(nullptr, &buf);
        y[i] = equalizer::get_output(&buf)[0];
    }

    double y_ref[N];
    reference_process(x, y_ref, N);

    unsigned int ok_count = 0;
    for(int i = 0; i < N; ++i)
    {
        cout << y[i] << endl;
        if(y[i] == y_ref[i])
            ++ok_count;
    }

    if (ok_count == N)
    {
        //cout << "OK" << endl;
        return 0;
    }
    else
    {
        cout << "FAIL: " << ok_count << "/" << N << " correct." << endl;
        return 1;
    }
}
