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
