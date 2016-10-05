
#include "test_parser.hpp"
#include "io.hpp"

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
                parse_option(a);
            else
                m_data.push_back(a);
        }
    }

    bool has_option(const string & name) const
    {
        return m_opts.find(name) != m_opts.end();
    }

    string option(const string & name) const
    {
        auto val = m_opts.find(name);
        if (val == m_opts.end())
            return string();
        else
            return val->second;
    }

    const vector<string> & data() const
    {
        return m_data;
    }

private:
    void parse_option(const string & text)
    {
        string name, value;
        auto eq_pos = text.find('=');
        if (eq_pos != string::npos)
        {
            name = text.substr(0,eq_pos);
            value = text.substr(eq_pos+1);
        }
        else
        {
            name = text;
        }
        m_opts[name] = value;
    }

    vector<string> m_data;
    unordered_map<string,string> m_opts;
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

struct print_size
{
    const vector<int> & v;
    print_size(const vector<int> & v): v(v) {}
    friend ostream & operator<< (ostream & s, const print_size & p)
    {
        const auto & v = p.v;
        s << '[';
        for (int i = 0; i < (int)v.size(); ++i)
        {
            if (i > 0)
                s << ",";
            s << v[i];
        }
        s << ']';
        return s;
    }
};

int main(int argc, char *argv[])
{
    arguments args(argc-1, argv+1);

    bool do_compare = false;
    elem_type expected_type;
    vector<int> expected_size;
    arrp::testing::array expected_data;

    if (args.has_option("--compare"))
    {
        do_compare = true;

        string filename = args.option("--compare");

        ifstream input(filename);
        if (!input.is_open())
        {
            cout << "Failed to open input file: " << filename << endl;
            return 1;
        }

        test_parser parser;
        parser.parse(input);

        expected_data = parser.data();
        expected_size = parser.size();
        expected_type = parser.type();

        do_compare = !expected_data.is_empty();
    }

    if (!expected_data.is_empty() && args.has_option("--print-expected"))
    {
        cout << "Expected:" << endl;
        print(expected_data);
    }

    vector<int> actual_size;
    array_size<test::traits::output_type>::get_size(actual_size);

    int actual_count = actual_size.empty() ? 1 : actual_size[0];

    if (do_compare)
    {
        if (actual_size != expected_size)
        {
            cout << "Actual and expected output size differ: "
                 << print_size(actual_size) << " != " << print_size(expected_size)
                 << endl;
            return 1;
        }
    }

    int out_count = 10;

    if (args.has_option("--count"))
    {
        auto value = args.option("--count");
        try {
            out_count = stoi(value);
        } catch (...) {
            cerr << "Invalid argument for --count: " << value << endl;
            return 1;
        }

        if (actual_count >= 0 && out_count > actual_count)
        {
            cerr << "Requested output count is larger than available." << endl;
            return 1;
        }

        cout << "Producing requested output count: "
             << out_count << endl;
    }
    else if (!expected_data.is_empty())
    {
        out_count = expected_data.count();
        cout << "Producing output count equal to test data count: "
             << out_count << endl;
        assert(actual_count < 0 || out_count <= actual_count);
    }
    else if (actual_count >= 0)
    {
        out_count = actual_count;
        cout << "Producing entire output count: " << out_count << endl;
    }
    else
    {
        cout << "Producing default output count: " << out_count << endl;
    }

    auto program = new program_type;
    program->io = new io_type;

    bool ok;

    ok = run(program, out_count, 100);

    if (args.has_option("--print-produced"))
    {
        cout << "Produced:" << endl;
        print(program->io->data());
    }

    if (ok && !expected_data.is_empty())
        ok = compare(program->io->data(), expected_data);

    if (ok)
      cout << "OK." << endl;
    else
      cout << "FAILED." << endl;

    delete program->io;
    delete program;

    return ok ? 0 : 1;
}
