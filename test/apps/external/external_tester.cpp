#include "external.cpp"
#include "../../common/generic_printer.hpp"
#include "../../common/generic_tester.hpp"

class tester;
typedef test::program<tester> program;

class tester : public generic_printer, public generic_tester<program, int>
{
public:
    tester(): generic_tester(this) {}

    void f(int in[5], int out[6])
    {
      out[0] = 1;
      for (int i = 0; i < 5; ++i)
        out[i+1] = in[i] + 1;
    }

    void g(int & in, int & out)
    {
      out = in * 100;
    }

    void output(int (&data)[6])
    {
      generic_printer::output(data);

      int v = 0;
      for (int i = 0; i < 6; ++i)
        v += data[i];

      record(v);
    }
};

void main()
{
    tester t;
    t.run(9);

    vector<int> expected = {
      16,
      621,
      1226,
      1831,
      2436,
      3041,
      3646,
      4251,
      4856,
      5461
    };

    t.compare(expected);
}
