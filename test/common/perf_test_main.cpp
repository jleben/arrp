#include "perf_io.hpp"
#include "perf_test_driver.hpp"
#include "../../compiler/arg_parser.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>

using namespace arrp::testing;
using namespace std;

namespace test { struct traits; }

typedef arrp::testing::perf_io<test::traits> io_type;
typedef test::program<io_type> program_type;

class perf_test
{
public:
    perf_test(int period_count): m_period_count(period_count) {}

    void initialize()
    {
        delete p;
        p = new program_type;
        p->io = new io_type;
        p->prelude();
    }

    void run()
    {
        for (int i = 0; i < m_period_count; ++i)
        {
            p->period();
        }
    }

private:
    int m_period_count = 100;
    program_type * p = nullptr;
};

int main(int argc, char * argv[])
{
    using namespace stream::compiler;

    struct args
    {
        int period_count = 100;
        int ensemble_size = 1000;
        int rep_count = 3;
    };

    args arg;

    stream::compiler::arguments arg_parser(argc-1, argv+1);
    arg_parser.add_option({"periods", "p", "<count>",
                           "Execute <count> periods."},
                          new int_option(&arg.period_count));
    arg_parser.add_option({"ensemble", "e", "<count>",
                           "The program is run <count> times in an ensemble."},
                          new int_option(&arg.ensemble_size));
    arg_parser.add_option({"repetitions", "r", "<count>",
                           "The evaluation is repeated <count> times."},
                          new int_option(&arg.rep_count));

    try {
        arg_parser.parse();
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

    cout << "- Periods: " << arg.period_count << endl;
    cout << "- Ensemble: " << arg.ensemble_size << endl;
    cout << "- Repetitions: " << arg.rep_count << endl;

    perf_test t(arg.period_count);
    test_driver<perf_test> d;
    d.go(t, arg.rep_count, arg.ensemble_size);
}
