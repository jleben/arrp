#include "test.hpp"

#include <iostream>
#include <array>

using namespace std;
using namespace stream;

extern "C" {
void initialize(double** inputs, testing::buffer* buffers);
void process(double** inputs, testing::buffer* buffers);
}

int main()
{
    double m1[5][2][3];
    double m2[5][3][2];
    double out[5][2][2];

    for (int x=0; x<5; ++x)
    {
        m1[x][0][0] = 1 + x;
        m1[x][0][1] = 2 + x;
        m1[x][0][2] = 3 + x;
        m1[x][1][0] = 4 + x;
        m1[x][1][1] = 5 + x;
        m1[x][1][2] = 6 + x;

        m2[x][0][0] = 1 + x;
        m2[x][0][1] = 2 + x;
        m2[x][1][0] = 3 + x;
        m2[x][1][1] = 4 + x;
        m2[x][2][0] = 5 + x;
        m2[x][2][1] = 6 + x;
    }

/*
S_0: 5 2 3 [30]
S_1: 5 3 2 [30]
S_2: 5 2 2 3 [60]
S_3: 5 2 2 [20]
S_4: 5 2 2 2 [40]
S_5: 5 2 2 [20]
*/

    testing::buffer buffers[] =
    {
        testing::alloc_buffer(30),
        testing::alloc_buffer(30),
        testing::alloc_buffer(60),
        testing::alloc_buffer(20),
        testing::alloc_buffer(40),
        testing::init_buffer((double*)out)
    };

    double *inputs[] = { (double*)m1, (double*)m2 };

    initialize(inputs, buffers);

    for (int x=0; x<5; ++x)
    {
        cout << out[x][0][0] << "  " << out[x][0][1] << endl;
        cout << out[x][1][0] << "  " << out[x][1][1] << endl;
        cout << "---" << endl;
    }

    int correct_count = 0;
    for (int x=0; x<5; ++x)
    {
        for (int r = 0; r < 2; ++r)
        {
            for (int c = 0; c < 2; ++c)
            {
                double sum = 0;
                for(int i = 0; i < 3; ++i)
                {
                    sum += m1[x][r][i] * m2[x][i][c];
                }
                if (out[x][r][c] == sum)
                    ++correct_count;
            }
        }
    }

    cout << ">> " << ((double) correct_count * 100 / (5*2*2)) << "% CORRECT" << endl;

    return 0;
}

