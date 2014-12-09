#include "matrix-mult.h"

#include <iostream>
#include <array>

using namespace std;

template<typename T, int ...S>
struct multi_array
{
    multi_array(T* d): data(d) {}

    template<int size_head, int ...size_tail>
    struct indexer
    {
        template<typename ...int_type>
        static
        int index(int base, int idx_head, int_type ... idx_tail)
        {
            return indexer<size_tail...>::index(idx_head + base * size_head, idx_tail...);
        }
    };
    template<int size_head>
    struct indexer<size_head>
    {
        static
        int index(int base, int idx_head)
        {
            return idx_head + base * size_head;
        }
    };

    template<typename ...int_type>
    T operator()(int_type ...idxs)
    {
        int idx = indexer<S...>::index(0,idxs...);
        return data[idx];
    }

    T * data;
};

int main()
{
    matrix_multiply::buffer buf;
    matrix_multiply::allocate(&buf);

    double m1[5][2][3];
    double m2[5][3][2];

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

    matrix_multiply::initialize((double*)m1, (double*)m2, &buf);

    multi_array<double,5,2,2> output(matrix_multiply::get_output(&buf));

    for (int x=0; x<5; ++x)
    {
        cout << output(x,0,0) << "  " << output(x,0,1) << endl;
        cout << output(x,1,0) << "  " << output(x,1,1) << endl;
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
                if (output(x,r,c) == sum)
                    ++correct_count;
            }
        }
    }

    cout << ">> " << ((double) correct_count * 100 / (5*2*2)) << "% CORRECT" << endl;

    return 0;
}

