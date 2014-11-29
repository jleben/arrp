#ifndef STREAM_LANG_TESTING_INCLUDED
#define STREAM_LANG_TESTING_INCLUDED

#include <iostream>
#include <vector>
#include <string>

namespace stream {
namespace testing {

using std::vector;
using std::string;

struct buffer
{
    double *data;
    std::uint32_t phase;
};

buffer alloc_buffer( int size, int phase = 0 )
{
    buffer b;
    b.data = new double[size];
    b.phase = phase;
    for (int i = 0; i < size; ++i)
        b.data[i] = 0;
    return b;
}

buffer init_buffer( double *data, int phase = 0 )
{
    buffer b;
    b.data = data;
    b.phase = 0;
    return b;
}

#if 0
void print( double *d,
            const vector<int> & size,
            const vector<int> & index = vector<int>() )
{
    using namespace std;

    if (index.size() == size.size())
    {
        int factor = 1;
        int flat_index = 0;
        int dim = size.size();
        while(dim--)
        {
            flat_index += index[dim] * factor;
            factor *= size[dim];
        }
        cout << d[flat_index] << endl;
        //printf("%f\n", d[flat_index]);
    }
    else
    {
        int dim = index.size();
        vector<int> next_index = index;
        next_index.push_back(1);

        int level = dim+1;
        cout << string(level * 2, '*') << level << endl;
        //for (int d = 0; d < dim; ++d)
            //printf("-");
        //printf("\n");

        for (int i = 0; i < size[dim]; ++i)
        {
            next_index[dim] = i;
            print(d, size, next_index);
        }

        cout << string(level * 2, '*') << level << endl;
    }
}
#endif
int outcome(bool correct)
{
    using namespace std;
    cout << (correct ? "CORRECT" : "INCORRECT") << endl;
    return correct ? 0 : 1;
}

}
}

#endif // STREAM_LANG_TESTING_INCLUDED
