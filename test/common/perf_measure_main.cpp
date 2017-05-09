#include "perf_io.hpp"
#include "../../compiler/arg_parser.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include <papi.h>

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

template <typename T>
T * make_aligned()
{
    auto space = sizeof(T) + 64;
    void * d = malloc(space);
    d = align(64, sizeof(T), d, space);
    if (d == nullptr || space < sizeof(T))
    {
        throw std::runtime_error("Could not allocate aligned storage.");
    }

    return new(d) T;
}

int main(int argc, char * argv[])
{
    using namespace stream::compiler;

    int period_count = 100;
    bool print = false;
    bool prologue = true;

    stream::compiler::arguments arg_parser;

    arg_parser.add_option({"help", "h", "Print help."}, [](arguments& args){
        args.print_help();
        throw arguments::abortion();
    });
    arg_parser.add_option({"periods", "p", "<count>",
                           "Execute <count> periods."},
                          new int_option(&period_count));
    arg_parser.add_option({"print", "", "", ""},
                          new switch_option(&print));
    arg_parser.add_option({"no-prologue", "", "", ""},
                          new switch_option(&prologue, false));

    try {
        arg_parser.parse(argc-1, argv+1);
    }
    catch (stream::compiler::arguments::abortion &)
    {
        return 1;
    }
    catch (stream::compiler::arguments::error & e)
    {
        cerr << e.msg() << endl;
        return 0;
    }

    if (PAPI_library_init(PAPI_VER_CURRENT) < 0)
    {
        cerr << "ERROR: PAPI_library_init" << endl;
        return 1;
    }

    auto RESTRICT p = make_aligned<program_type>();

    p->io = new io_type;
    p->io->set_printing(print);

    p->prelude();

    auto start = PAPI_get_virt_usec();

    for (int i = 0; i < period_count; ++i)
    {
        p->period();
    }

    auto end = PAPI_get_virt_usec();

    auto duration = end - start;

    int64_t count = period_count * test::traits::output_period_size;

    double speed = double(count) / duration;

    cout << "Speed = " << speed << " elems/us" << endl;
}

