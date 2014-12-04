#include "../../interface/cpp-interface.hpp"

#include <iostream>

using buffer = stream::interface::buffer;
using namespace std;

extern "C" {
void initialize(double** inputs, buffer* buffers);
//void process(double** inputs, buffer* buffers);
}

int main()
{
    using namespace stream::interface;

    descriptor d = descriptor::from_file("sum.meta");

    process p(d, &::initialize, nullptr);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    double *inputs[] = {input};

    p.run(inputs);

    double result = p.output()[0];

    cout << "result = " << result << endl;

    if (result == 45)
        return 0;
    else
        return 1;
}
