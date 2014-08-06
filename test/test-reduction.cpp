#include "test.hpp"

#include <iostream>
using namespace std;
using namespace stream::testing;

extern "C" {
double f_0(double *);
}
int main()
{
    const unsigned int in_count = 10;
    //vector<int> out_count = {1};

    double in[in_count];
    for (int i = 0; i < in_count; ++i)
        in[i] = 2;

    double f_out = f_0(in);
    cout << "result = " << f_out << endl;

    return outcome(f_out == 1024.0);
}
