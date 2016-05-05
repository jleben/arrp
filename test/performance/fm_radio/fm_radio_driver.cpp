#include "fm_radio.cpp"
#include "../drivers/test_driver.hpp"

#include <iostream>
#include <iomanip>

#define PRINT 0

using namespace std;

namespace fm_radio {
static volatile float v;
inline void output(float * data)
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
    fm_radio::state s;
    void initialize()
    {
        fm_radio::initialize(&s);
    }
    void run()
    {
        for (int i = 0; i < 100; ++i)
            fm_radio::process(&s);
    }
};

int main()
{
    fm_radio_test test;

    test_driver<fm_radio_test> driver;
    driver.go(test, 3, 1000);
}
