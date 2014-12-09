//#include "test.hpp"
#include "../../interface/cpp-interface.hpp"

#include <iostream>
#include <array>

using namespace std;
using namespace stream::interface;

extern "C" {
void initialize(double* a, double *b, buffer* buffers);
}

int main()
{
    descriptor d;
    try
    {
        d = descriptor::from_file("matrix-mult.meta");
    }
    catch (descriptor::read_error & e)
    {
        cerr << "** Error reading meta-data: " << e.what << endl;
        return 1;
    }

    process p(d);

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

    p.m_buffers.back().data = out;

    initialize((double*)m1, (double*)m2, p.m_buffers.data());

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

