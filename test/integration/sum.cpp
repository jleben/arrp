#include "sum.h"

#include <iostream>

using namespace std;

int main()
{
    sum::buffer buf;
    sum::allocate(&buf);

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    sum::initialize(input, &buf);

    double result = *sum::get_output(&buf);

    cout << "result = " << result << endl;

    if (result == 45)
        return 0;
    else
        return 1;
}
