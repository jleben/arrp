#include "fm_radio_s_kernel.cpp"
#include "../drivers/test_driver.hpp"
#include <valgrind/callgrind.h>
#include <iostream>
#include <iomanip>

#define PRINT 1

using namespace std;

struct fm_radio_printer : public fm_radio_s_kernel::state<fm_radio_printer>
{
    fm_radio_printer() { io = this; }

    void output(float * data)
    {
        cout << std::showpoint << std::setprecision(15) << (double) *data << endl;
    }
};

struct fm_radio_test
{
    typedef fm_radio_s_kernel::state<fm_radio_test> kernel_t;

    kernel_t kernel;
    volatile float dummy;

    void initialize()
    {
        kernel = kernel_t();
        kernel.io = this;
        kernel.initialize();
    }
    void run()
    {
        CALLGRIND_TOGGLE_COLLECT;
        for (int i = 0; i < 500; ++i)
            kernel.process();
        CALLGRIND_TOGGLE_COLLECT;
    }
    void output(float * data)
    {
        dummy = *data;
    }
};

int main()
{
#if PRINT

    fm_radio_printer p;
    p.initialize();
    for (int i = 0; i < 20; ++i)
        p.process();

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
