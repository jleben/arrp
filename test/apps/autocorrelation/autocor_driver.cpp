#include "autocor_kernel.cpp"

#include <iostream>

using namespace std;

static const int out_size = 19;

class autocor_printer : public autocorrelation::state<autocor_printer>
{
public:
    void output(double * data)
    {
        for (int i = 0; i < out_size; ++i)
        {
            cout << i << ":" << data[i] << " ";
        }
        cout << endl;
    }
};

int main()
{
    auto printer = new autocor_printer;
    printer->initialize();
    for (int i = 0; i < 10; ++i)
    {
        printer->process();
    }
}
