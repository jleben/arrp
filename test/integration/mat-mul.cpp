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

#include "matrix-mult.h"
#include "test.hpp"

#include <iostream>
#include <array>
#include <chrono>

using namespace std;
using namespace std::chrono;

using stream::testing::multi_array;

constexpr int T = 10;
constexpr int A = 128;
constexpr int B = 256;
constexpr int C = 128;

using in1_type = multi_array<double,T,A,B>;
using in2_type = multi_array<double,T,B,C>;
using out_type = multi_array<double,T,A,C>;

void compute_expected(const in1_type & in1,
                      const in2_type & in2,
                      out_type & out)
{
    for (int t=0; t<T; ++t)
    {
        for (int a = 0; a < A; ++a)
        {
            for (int c = 0; c < C; ++c)
            {
                double sum = 0;
                for(int b = 0; b < B; ++b)
                {
                    sum += in1(t,a,b) * in2(t,b,c);
                }
                out(t,a,c) = sum;
            }
        }
    }
}

int main()
{
    std::random_device rand;

    bool all_ok = true;

    for(int rep = 0; rep < 3; ++rep)
    {
        cout << endl;
        cout << "## Run " << rep << " ##" << endl;

       in1_type in1 = in1_type::random(-10,10,rand());
       in2_type in2 = in2_type::random(-10,10,rand());
       out_type expected;

       // Run expected

       auto c_start_time = high_resolution_clock::now();
       compute_expected(in1, in2, expected);
       auto c_end_time = high_resolution_clock::now();

       // Run lang

       matrix_multiply::buffer state;
       matrix_multiply::allocate(&state);

       auto lang_start_time = high_resolution_clock::now();
       matrix_multiply::initialize(in1.data(), in2.data(), &state);
       auto lang_end_time = high_resolution_clock::now();

       // Compare

       out_type actual(matrix_multiply::get_output(&state));

       bool ok = actual == expected;
       stream::testing::outcome(ok);

       all_ok &= ok;

       // Report

       duration<double, std::micro> c_time =
               c_end_time - c_start_time;
       duration<double, std::micro> lang_time =
               lang_end_time - lang_start_time;

       cout << "C time: " << c_time.count() << endl;
       cout << "lang time: " << lang_time.count() << endl;
       cout << "lang / C ratio: "
            << (lang_time.count() / c_time.count()) << endl;
       cout << "lang / C ratio: "
            << (lang_time.count() / c_time.count()) << endl;
    }

    cout << endl << "## Summary:" << endl;
    return stream::testing::outcome(all_ok);
}

