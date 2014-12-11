#include "flux.h"
#include "test.hpp"

#include <cmath>
#include <iomanip>

using stream::testing::multi_array;
using namespace std;

template<int T, int N>
multi_array<double,T,N> input()
{
    multi_array<double,T,N> in;
    for(int t = 0; t < T; ++t)
        for(int n = 0; n < N; ++n)
            in(t,n) = t * 10 + n;
    return in;
}

template<int T, int N>
multi_array<double,T-1> expected( const multi_array<double,T,N> & in )
{
    multi_array<double,T-1> out;

    auto map = [](double in) -> double
    {
        return std::log(1000 * in + 1);
    };

    for(int t = 0; t < T-1; ++t)
    {
        double sum = 0;
        for(int n = 0; n < N; ++n)
            sum += std::max(0.0, map(in(t+1,n)) - map(in(t,n)));
        out(t) = sum;
    }

    return out;
}

template<int T>
void print( const multi_array<double,T> & a )
{
    for (int t = 0; t < T; ++t)
        cout << a(t) << endl;
}

template<int T, int N>
void print( const multi_array<double,T,N> & a )
{
    for (int t = 0; t < T; ++t)
    {
        for (int n = 0; n < N; ++n)
            cout << std::setw(4) << a(t,n) << ' ';
        cout << endl;
    }
}

int main()
{
    flux::buffer buf;
    flux::allocate(&buf);

    constexpr int T=10;
    constexpr int N=10;
    multi_array<double,T,N> in = input<T,N>();
    multi_array<double,T-1> ex = expected(in);
    multi_array<double,T-1> out;

    cout << "in:" << endl;
    print(in);
    cout << "expected:" << endl;
    print(ex);

    flux::initialize(in.data(), &buf);

    for(int t = 0; t < T-1; ++t)
    {
        flux::process(in.data() + (t+1)*N, &buf);
        out(t) = *flux::get_output(&buf);
    }

    cout << "out:" << endl;
    print(out);


    return stream::testing::outcome(out == ex);
}
