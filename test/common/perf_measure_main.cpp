#include "perf_io.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include "utils.hpp"
#include "../../compiler/arg_parser.hpp"

#include <papi.h>
#include <fcntl.h>
#include <unistd.h>

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

int main(int argc, char * argv[])
{
    using namespace stream::compiler;

    int warmup_period_count = 2;
    int period_count = 100;
    bool measure = false;
    bool print = false;
    bool prologue = true;
    int rt_priority = -1;
    int max_cpu_latency = -1;
    int cpu = -1;

    stream::compiler::arguments arg_parser;

    arg_parser.add_option({"help", "h", "Print help."}, [](arguments& args){
        args.print_help();
        throw arguments::abortion();
    });
    arg_parser.add_option({"periods", "p", "<count>",
                           "Execute <count> periods."},
                          new int_option(&period_count));
    arg_parser.add_option({"warmup-periods", "w", "<count>",
                           "Execute <count> periods."},
                          new int_option(&warmup_period_count));
    arg_parser.add_option({"measure", "", "", ""},
                          new switch_option(&measure));
    arg_parser.add_option({"print", "", "", ""},
                          new switch_option(&print));
    arg_parser.add_option({"no-prologue", "", "", ""},
                          new switch_option(&prologue, false));
    arg_parser.add_option({"rt-priority", "", "", ""},
                          new int_option(&rt_priority));
    arg_parser.add_option({"cpu-pin", "", "", ""},
                          new int_option(&cpu));
    arg_parser.add_option({"cpu-latency", "", "", ""},
                          new int_option(&max_cpu_latency));

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

    if (rt_priority >= 0)
    {
        arrp::set_rt_scheduling(rt_priority);
    }

    if (cpu >= 0)
    {
        arrp::pin_to_cpu(cpu);
    }

    if (max_cpu_latency >= 0)
    {
        arrp::set_max_cpu_latency(max_cpu_latency);
    }

    auto RESTRICT p = arrp::make_aligned<program_type>();

    p->io = new io_type;
    p->io->set_printing(print);

    p->prelude();

    if (measure)
    {
        if (PAPI_library_init(PAPI_VER_CURRENT) < 0)
        {
            cerr << "ERROR: PAPI_library_init" << endl;
            return 1;
        }

        for(;;)
        {
            for (int i = 0; i < warmup_period_count; ++i)
            {
                p->period();
            }

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

            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    else
    {
        for (int i = 0; i < period_count; ++i)
        {
            p->period();
        }
    }
}

