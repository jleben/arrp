#include "perf_io.hpp"
#include "../../compiler/arg_parser.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef __GNUC__
#define RESTRICT
#else
#define RESTRICT restrict
#endif

using namespace arrp::testing;
using namespace std;

namespace test { struct traits; }

typedef arrp::testing::perf_io<test::traits> io_type;
typedef test::program<io_type> program_type;

#if 0
class perf_test
{
public:
    perf_test(int period_count): m_period_count(period_count) {}

    void initialize()
    {
        auto space = sizeof(program_type) + 64;
        void * d = malloc(space);
        d = align(64, sizeof(program_type), d, space);
        if (d == nullptr || space < sizeof(program_type))
        {
            throw runtime_error("Could not align storage.");
        }

        prog = new(d) program_type;
        prog->io = new io_type;
        prog->prelude();

        //this_thread::sleep_for(chrono::milliseconds(500));
        //cout << "blah   blaab alsjd aldkj aldkjalsdkj" << p << endl;

        static volatile void* x;
        //x = &prog;
#if 0
        cout << "Pointer aligned to: "
             << (uintptr_t(&p) % 128)
             << " -> "
             << (uintptr_t(p) % 128) << endl;
#endif
#if 0
        delete p;


        p = new program_type;
        p->io = new io_type;
        p->prelude();
#endif
    }

    void run()
    {
        for (int i = 0; i < m_period_count; ++i)
        {
            prog->period();
        }
    }

private:
    int m_period_count = 100;
    program_type * restrict prog = nullptr;
};
#endif
int main(int argc, char * argv[])
{
    using namespace stream::compiler;

    int period_count = 100;
    bool print = false;

    if (argc >= 2)
        period_count = std::atoi(argv[1]);

    if (argc >= 3)
        print = std::strcmp(argv[2], "print") == 0;

#if 1
    auto space = sizeof(program_type) + 64;
    void * d = malloc(space);
    d = align(64, sizeof(program_type), d, space);
    if (d == nullptr || space < sizeof(program_type))
    {
        cerr << "Could not align storage." << endl;
        return 1;
    }

    auto RESTRICT p = new(d) program_type;
    //cout << "Pointer alignment: " << ((uintptr_t)&p % 64) << endl;
    //static volatile void * pp = &p;

    p->io = new io_type;
    p->io->set_printing(print);

    p->prelude();

    for (int i = 0; i < period_count; ++i)
    {
        p->period();
    }
#else
    perf_test t(period_count);
    t.initialize();
    t.run();
#endif
}
