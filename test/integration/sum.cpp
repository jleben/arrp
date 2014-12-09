#include "sum.h"

#include <iostream>

using namespace std;

int main()
{
    sum::buffer b[4];

    for(int i = 0; i < 4; ++i)
    {
        b[i].data = new double;
        b[i].phase = 0;
    }

    double *input = new double[10];
    for (int i = 0; i < 10; ++i)
    {
        input[i] = i;
    }

    sum::initialize(input, b);

    double result = *(double*)b[3].data;

    cout << "result = " << result << endl;

    if (result == 45)
        return 0;
    else
        return 1;
}
