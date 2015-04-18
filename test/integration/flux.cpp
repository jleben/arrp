/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "flux.h"
#include "test.hpp"

#include <cmath>
#include <iomanip>
#include <thread>
#include <chrono>

using stream::testing::multi_array;
using namespace std;
using namespace std::chrono;

inline double map(double in)
{
#ifndef NO_LOGARITHM
    return std::log(1000 * in + 1);
#else
    return 1000 * in + 1;
#endif
}

template<int T, int N>
void expected_best( const multi_array<double,T,N> & in,
                    multi_array<double,T-1> & out,
                    multi_array<double,N> & buf )
{
    for(int n = 0; n < N; ++n)
    {
        buf(n) = map(in(0,n));
    }

    for(int t = 0; t < T-1; ++t)
    {
        double sum = 0;
        for(int n = 0; n < N; ++n)
        {
            double in2 = map(in(t+1,n));
            sum += std::max(0.0, in2 - buf(n));
            buf(n) = in2;
        }
        out(t) = sum;
    }
}

template<int T, int N>
void expected_typical( const multi_array<double,T,N> & in,
                     multi_array<double,T-1> & out )
{
    for(int t = 0; t < T-1; ++t)
    {
        double sum = 0;
        for(int n = 0; n < N; ++n)
        {
            double in1 = map(in(t,n));
            double in2 = map(in(t+1,n));
            sum += std::max(0.0, in2 - in1);
        }
        out(t) = sum;
    }
}

///

static constexpr int T=1000;
static constexpr int N=1000;

static multi_array<double,T,N> *input = nullptr;
static int t = 0;

static void input_func(int index, double *data)
{
    //cout << "input: " << t << endl;
    for(int n = 0; n < N; ++n)
    {
        data[n] = (*input)(t, n);
    }
    ++t;
}

int main()
{
    multi_array<double,T-1> ex;
    multi_array<double,N> ex_buf;
    multi_array<double,T-1> out;

    std::random_device rand;

    bool all_ok = true;
    for (int rep = 0; rep < 3; ++rep)
    {
        cout << endl;
        cout << "## Run " << rep << " ##" << endl;

        multi_array<double,T,N> in = multi_array<double,T,N>::random(0,100,rand());
        input = &in;
        t = 0;

        auto ex_best_start_time = high_resolution_clock::now();
        expected_best(in, ex, ex_buf);
        auto ex_best_end_time = high_resolution_clock::now();

        auto ex_typical_start_time = high_resolution_clock::now();
        expected_typical(in, ex);
        auto ex_typical_end_time = high_resolution_clock::now();

        flux::buffer buf;
        flux::allocate(&buf);
        buf.input_func = (void*) &input_func;

        auto test_start_time = high_resolution_clock::now();

        flux::initialize(in.data(), &buf);

#ifdef STREAMING
        for(int t = 0; t < T-1; ++t)
        {
            flux::process(nullptr, &buf);
            out(t) = *flux::get_output(&buf);
        }
#endif

        auto test_end_time = high_resolution_clock::now();

        duration<double, std::micro> c_best_time =
                ex_best_end_time - ex_best_start_time;
        duration<double, std::micro> c_typical_time =
                ex_typical_end_time - ex_typical_start_time;
        duration<double, std::micro> stream_time =
                test_end_time - test_start_time;

        cout << "C best time: " << c_best_time.count() << endl;
        cout << "C typical time: " << c_typical_time.count() << endl;
        cout << "lang time: " << stream_time.count() << endl;
        cout << "lang / C best ratio: "
             << (stream_time.count() / c_best_time.count()) << endl;
        cout << "lang / C typical ratio: "
             << (stream_time.count() / c_typical_time.count()) << endl;

#ifndef STREAMING
        out = multi_array<double,T-1>(flux::get_output(&buf));
#endif

#if 0
        cout << "-- in:" << endl;
        cout << in;
        cout << "-- expected:" << endl;
        cout << ex;
        cout << "-- out:" << endl;
        cout << out;
#endif
        bool ok = out == ex;
        stream::testing::outcome(ok);

        all_ok &= ok;
    }

    cout << endl << "## Summary:" << endl;

    return stream::testing::outcome(all_ok);
}
