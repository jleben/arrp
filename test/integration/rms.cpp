#include "rms.h"

#include <iostream>
#include <cmath>

using namespace std;

int main()
{
    rms::buffer buf;
    rms::allocate(&buf);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    rms::initialize(input, &buf);

    double expected;
    {
        double sum = 0;
        for (int i = 0; i < 10; ++i)
            sum += input[i] * input[i];
        sum /= 10.0;
        expected = std::sqrt(sum);
    }

    double actual = *rms::get_output(&buf);

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
