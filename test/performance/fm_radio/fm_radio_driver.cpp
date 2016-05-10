#include "fm_radio_s_kernel.cpp"
#include "../drivers/test_driver.hpp"
#include <valgrind/callgrind.h>
#include <iostream>
#include <iomanip>

#define PRINT 1

using namespace std;

namespace fm_radio_s_kernel {
static volatile float v;
inline void state::output(float * data)
{
#if PRINT
    cout << std::showpoint << std::setprecision(15) << (double) *data << endl;
#else
    v = *data;
#endif
}
}

struct fm_radio_test
{
    fm_radio_s_kernel::state s;
    void initialize()
    {
        s = fm_radio_s_kernel::state();
        s.initialize();
    }
    void run()
    {
        CALLGRIND_TOGGLE_COLLECT;
        for (int i = 0; i < 500; ++i)
            s.process();
        CALLGRIND_TOGGLE_COLLECT;
    }
};

int main()
{
#if PRINT

    fm_radio_s_kernel::state s;

    s.initialize();

    for (int i = 0; i < 20; ++i)
        s.process();

#else // PRINT

    fm_radio_test test;

#if 1
    for (int i = 0; i < 1000; ++i)
    {
        test.initialize();
        test.run();
    }
#else
    test_driver<fm_radio_test> driver;
    driver.go(test, 3, 1000);
#endif

#endif // PRINT
}
