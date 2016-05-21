#include "fft_kernel.cpp"

#include <iostream>

using namespace std;

int N = 8;

class fft_printer : public fft::state<fft_printer>
{
public:
    fft_printer()
    {
        io = this;
    }

    void output(float * data)
    {
        cout << *data << endl;
    }

    void output(complex<float> * data)
    {
        cout << norm(*data) / N << endl;
#if 0
        for (int i = 0; i < N; ++i)
        {
            cout << norm(data[i]) / N << endl;
            //cout << data[i].real() << "\t" << data[i].imag() << endl;
        }
#endif
    }
};

int main()
{
    auto p = new fft_printer;
    p->initialize();
}
