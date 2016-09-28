#pragma once

#include <iostream>

template <typename P, typename V>
class generic_tester
{
public:
    generic_tester(P * program)
    {
        m_program = program;
    }

    template <typename IO>
    generic_tester(IO * io)
    {
        m_program = new P;
        m_program->io = io;
    }

    void run(int count)
    {
        m_program->prelude();
        while(count--)
            m_program->period();
    }

    void record(const V & value)
    {
        m_records.push_back(value);
    }

    bool compare(const vector<V> & expected)
    {
        using namespace std;

        if (expected.size() != m_records.size())
        {
          cout << "Expected " << expected.size() << " values"
               << ", but got " << m_records.size() << "." << endl;
          cout << "FAILED." << endl;
          return false;
        }

        bool ok = true;

        for (int i = 0; i < expected.size(); ++i)
        {
            const auto & e = expected[i];
            const auto & v = m_records[i];
            if (!compare(v,e))
            {
                ok = false;
                cout << "Output " << i << ":"
                     << " expected = " << e
                     << " but actual = " << v
                     << "." << endl;
            }
        }

        if (ok)
          cout << "OK." << endl;
        else
          cout << "FAILED." << endl;

        return ok;
    }

    template <typename V>
    static bool compare(const V & lhs, const V & rhs)
    {
      return lhs == rhs;
    }

    static bool compare(const double & lhs, const double & rhs)
    {
      return std::abs(lhs - rhs) < 0.001;
    }

private:
    P * m_program;
    vector<V> m_records;
};
