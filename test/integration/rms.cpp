#include "../../interface/cpp-interface.hpp"

#include <iostream>
#include <cmath>

using buffer = stream::interface::buffer;
using namespace std;

extern "C" {
void initialize(double* input, buffer* buffers);
}

int main()
{
    using namespace stream::interface;

    descriptor d;
    try
    {
        d = descriptor::from_file("rms.meta");
    }
    catch (descriptor::read_error & e)
    {
        cerr << "** Error reading meta-data: " << e.what << endl;
        return 1;
    }

    process p(d);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    initialize(input, p.m_buffers.data());

    double expected;

    {
        double sum = 0;
        for (int i = 0; i < 10; ++i)
            sum += input[i] * input[i];
        sum /= 10.0;
        expected = std::sqrt(sum);
    }

    double actual = *p.output<real>();

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
