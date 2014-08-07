#include "test.hpp"

#include <iostream>

using namespace std;
using namespace stream;

extern "C" { void process(void**); }

int main()
{
    double m1[5][2][2];
    double m2[5][2][2];

    for (int x=0; x<5; ++x)
    {
        m1[x][0][0] = 1 + x;
        m1[x][0][1] = 2 + x;
        m1[x][1][0] = 3 + x;
        m1[x][1][1] = 4 + x;

        m2[x][0][0] = 1 + x;
        m2[x][0][1] = 2 + x;
        m2[x][1][0] = 3 + x;
        m2[x][1][1] = 4 + x;
    }

    double out[5][2][2];

    double buf[16]; // Required buffer pool.

    void *args[] = {m1, m2, out, buf};

    process(args);

    for (int x=0; x<5; ++x)
    {
        cout << out[x][0][0] << "  " << out[x][0][1] << endl;
        cout << out[x][1][0] << "  " << out[x][1][1] << endl;
        cout << "---" << endl;
    }
}

