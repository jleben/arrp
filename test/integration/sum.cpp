#include "../../interface/cpp-interface.hpp"

#include <iostream>

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
        d = descriptor::from_file("sum.meta");
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

    double result = *p.output<real>();

    cout << "result = " << result << endl;

    if (result == 45)
        return 0;
    else
        return 1;
}
