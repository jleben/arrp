#pragma once

#include <iostream>
#include <iomanip>

class generic_printer
{
    int m_output_count = 10;

public:
    generic_printer(int output_count = -1): m_output_count(output_count) {
        cout << std::fixed << std::setprecision(3);
    }

    void set_remaining_output_count(int count) { m_output_count = count; }
    int remaining_output_count() { return m_output_count; }

    template <typename T, size_t S>
    void output(T (&a)[S])
    {
        if (!check_output_count())
            return;

        using namespace std;
        print(a);
        cout << endl;
    }

    template <typename T>
    void output(T value)
    {
        if (!check_output_count())
            return;

        print(value);
        cout << endl;
    }

    template <typename T, size_t S>
    void print(T (&a)[S])
    {
        using namespace std;
        cout << "(";
        for (int i = 0; i < S; ++i)
        {
            if (i > 0)
                cout << ",";
            print(a[i]);
        }
        cout << ")";
    }

    template <typename T>
    void print(T a)
    {
        using namespace std;
        cout << a;
    }

private:
    bool check_output_count()
    {
        if (m_output_count == 0)
            return false;
        if (m_output_count > 0)
            --m_output_count;
        return true;
    }
};
