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

#include "centroid.h"
#include "test.hpp"

constexpr int in_size = 100;

using stream::testing::multi_array;
using in_type = multi_array<double,in_size>;
using namespace std;

double expected( const in_type & in )
{
    double num = 0;
    for (int i = 0; i < in_size; ++i)
        num += i * in(i);

    double denom = 0;
    for (int i = 0; i < in_size; ++i)
        denom += in(i);

    return num / denom;
}

int main()
{
    centroid::buffer state;
    centroid::allocate(&state);

    bool ok = true;

    std::uint32_t seeds[] = {71837490, 64738298, 15640987};
    int rep = 0;
    for (auto seed : seeds)
    {
        cout << "## run " << ++rep << " ##" << endl;

        in_type in = in_type::random(0,10,seed);
        double ex = expected(in);

        centroid::initialize(in.data(), & state);

        double result = *centroid::get_output(&state);

        cout << "-- input:" << endl;
        cout << in;
        cout << "-- expected:" << endl;
        cout << ex << endl;
        cout << "-- actual:" << endl;
        cout << result << endl;

        cout << endl;

        ok &= result == ex;
    }

    return stream::testing::outcome(ok);
}
