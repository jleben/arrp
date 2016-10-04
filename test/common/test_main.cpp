
#include "test_parser.hpp"
#include "io.hpp"

#include PROGRAM_SOURCE

#ifdef TEST_HEADER
#include TEST_HEADER
#endif

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace arrp::testing;
using namespace std;

namespace test { struct traits; }

typedef arrp::testing::io<test::traits> io_type;
typedef test::program<io_type> program_type;

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

    bool has_option(const string & option) const
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

bool compare(const arrp::testing::array & actual, const arrp::testing::array & expected)
{
    using namespace std;

    if (actual.size().size() != expected.size().size())
    {
        cout << "Different number of dimensions." << endl;
        return false;
    }

    for (int i = 1; i < actual.size().size(); ++i)
    {
        if (actual.size()[i] != expected.size()[i])
        {
            cout << "Different element sizes." << endl;
            return false;
        }
    }

    if (actual.count() < expected.count())
    {
        cout << "Too few elements in output." << endl;
        return false;
    }

    bool ok = true;

    for (int i = 0; i < expected.total_count(); ++i)
    {
        const auto & e = expected(i);
        const auto & v = actual(i);
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

bool run(program_type * program, int out_count, int max_periods = 100)
{
    cout << "Running until " << out_count << " outputs." << endl;

    using namespace std;

    auto io = program->io;

    int period = 0;

    program->prelude();

    while(io->data().count() < out_count && period++ < max_periods)
        program->period();

    if (io->data().count() < out_count)
    {
        cout << "Timed out at " << max_periods << " periods." << endl;
        return false;
    }

    return true;
}

void print(const arrp::testing::array & data, int dim, int & index)
{
    for (int i = 0; i < data.size()[dim]; ++i)
    {
        if (dim > 0 && i > 0)
            cout << ",";
        if (dim == data.size().size() - 1)
        {
            cout << data(index);
            ++index;
        }
        else
        {
            cout << '(';
            print(data, dim+1, index);
            cout << ')';
        }
        if (dim == 0)
            cout << endl;
    }
}

void print(const arrp::testing::array & data)
{
    cout << std::fixed << std::setprecision(3);

    if (data.size().empty())
    {
        cout << data(0) << endl;
    }
    else
    {
        int index = 0;
        print(data, 0, index);
    }
}

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
        print(parser.data());
    }

    auto program = new program_type;
    program->io = new io_type;

    bool ok;

    ok = run(program, parser.data().count(), 100);

    if (args.has_option("--print-produced"))
    {
        cout << "Produced:" << endl;
        print(program->io->data());
    }

    if (ok)
        ok = compare(program->io->data(), parser.data());

    if (ok)
      cout << "OK." << endl;
    else
      cout << "FAILED." << endl;

    delete program->io;
    delete program;

    return ok ? 0 : 1;
}
