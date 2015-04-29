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

#include "rms-kernel.h"

#include <iostream>
#include <cmath>

using namespace std;

int main()
{
    rms::state s;
    //rms::allocate(&buf);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    rms::initialize(input, &s);

    double expected;
    {
        double sum = 0;
        for (int i = 0; i < 10; ++i)
            sum += input[i] * input[i];
        sum /= 10.0;
        expected = std::sqrt(sum);
    }

    double actual = *rms::get_output(&s);

    cout << "expected: " << expected << endl;
    cout << "actual: " << actual << endl;

    if (actual == expected)
    {
        cout << "OK." << endl;
        return 0;
    }
    else
    {
        cout << "FAILED." << endl;
        return 1;
    }
}
