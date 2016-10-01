
#include "test_parser.hpp"
#include "io.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include <iostream>
#include <fstream>

using namespace arrp::testing;
using namespace std;

class program_id;

typedef arrp::testing::io<program_id> io_type;
typedef test::program<io_type> program_type;

bool compare(const vector<element> & actual, const vector<element> & expected)
{
    using namespace std;

    if (actual.size() < expected.size())
    {
        cout << "Too few elements in output." << endl;
        return false;
    }

    bool ok = true;

    for (int i = 0; i < expected.size(); ++i)
    {
        const auto & e = expected[i];
        const auto & v = actual[i];
        if (!compare(e,v))
        {
            ok = false;
#if 1
            cout << "Output " << i << ":"
                 << " expected = " << e
                 << " but actual = " << v
                 << "." << endl;
#endif
        }
    }

    return ok;
}

bool run(program_type * program, const vector<element> & expected)
{
    using namespace std;

    auto io = program->io;

    int max_periods = 100;
    int period = 0;

    program->prelude();

    while(io->data().size() < expected.size() && period++ < max_periods)
        program->period();

    if (io->data().size() < expected.size())
    {
        cout << "Timed out at " << max_periods << " periods." << endl;
        return false;
    }

    return compare(io->data(), expected);
}

class arguments
{
public:
    arguments(int argc, char *argv[])
    {
        for (int i = 0; i < argc; ++i)
        {
            char * a = argv[i];
            if (a[0] == '-')
                m_opts.push_back(a);
            else
                m_data.push_back(a);
        }
    }

    bool has_option(const string & option)
    {
        return std::find(m_opts.begin(), m_opts.end(), option) != m_opts.end();
    }

    const vector<string> & data() const
    {
        return m_data;
    }

private:
    vector<string> m_data;
    vector<string> m_opts;
};

int main(int argc, char *argv[])
{
    arguments args(argc-1, argv+1);

    if (args.data().size() < 1)
    {
        cout << "Required arguments: <input file>" << endl;
        return 1;
    }

    ifstream input(args.data()[0]);
    if (!input.is_open())
    {
        cout << "Failed to open input file." << endl;
        return 1;
    }

    test_parser parser;
    parser.parse(input);

    if (args.has_option("--print-expected"))
    {
        cout << "Expected:" << endl;
        for (auto & d : parser.data())
        {
            cout << d << endl;
        }
    }

    auto program = new program_type;
    program->io = new io_type;

    bool success = run(program, parser.data());
    if (success)
      cout << "OK." << endl;
    else
      cout << "FAILED." << endl;

    delete program->io;
    delete program;

    return success ? 0 : 1;
}
