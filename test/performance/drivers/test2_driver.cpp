#include "test2_opt.cpp"
#include "test2_raw.cpp"

#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;
using test_clock = chrono::high_resolution_clock;

typedef double fp_type;


fp_type dummy;

#define DEBUG_OUTPUT 0

namespace test2_opt {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

namespace test2_raw {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

namespace test2_ref {
inline void output(fp_type * data)
{
#if DEBUG_OUTPUT
    cout << *data << endl;
#else
    dummy = *data;
#endif
}
}

int main()
{
#if DEBUG_OUTPUT
    const int reps = 1;
    const int iter = 10;
#else
    const int reps = 3;
    const int iter = 1000 * 1000;
#endif

    for (int r = 0; r < reps; ++r)
    {
        double raw_ms, opt_ms;

        {
            test2_raw::state s;
            test2_raw::initialize(&s);

            auto start = test_clock::now();
            for (int i = 0; i < iter; ++i)
            {
                test2_raw::process(&s);
            }
            auto end = test_clock::now();
            raw_ms = duration<double,milli>(end - start).count();
        }

        {
            test2_opt::state s;
            test2_opt::initialize(&s);

            auto start = test_clock::now();
            for (int i = 0; i < iter; ++i)
            {
                test2_opt::process(&s);
            }
            auto end = test_clock::now();
            opt_ms = duration<double,milli>(end - start).count();
        }

        cout << "Raw = " << raw_ms << " ms" << endl;
        cout << "Optimized = " << opt_ms << " ms" << endl;
        cout << "Raw/Optimized = " << raw_ms/opt_ms << " x" << endl;
    }
}
