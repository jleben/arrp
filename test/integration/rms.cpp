#include "../../interface/cpp-interface.hpp"

#include <iostream>
#include <cmath>

using buffer = stream::interface::buffer;
using namespace std;

extern "C" {
void initialize(double** inputs, buffer* buffers);
//void process(double** inputs, buffer* buffers);
}

int main()
{
    using namespace stream::interface;

    descriptor d = descriptor::from_file("rms.meta");

    process p(d, &::initialize, nullptr);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    double *inputs[] = {input};

    p.run(inputs);

    double expected;

    {
        double sum = 0;
        for (int i = 0; i < 10; ++i)
            sum += input[i] * input[i];
        sum /= 10.0;
        expected = std::sqrt(sum);
    }

    double actual = p.output()[0];

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
