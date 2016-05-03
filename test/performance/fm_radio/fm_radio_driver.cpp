#include "fm_radio.cpp"

#include <iostream>
#include <iomanip>

using namespace std;

namespace fm_radio {
void output(float * data)
{
    cout << std::showpoint << std::setprecision(15) << (double) *data << endl;
}
}

int main()
{
    fm_radio::state s;
    fm_radio::initialize(&s);
    for (int i = 0; i < 20; ++i)
        fm_radio::process(&s);
}
