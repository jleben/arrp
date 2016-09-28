#include "wavetable_osc.cpp"
#include "../../common/generic_printer.hpp"
#include "../../common/generic_tester.hpp"

class tester;
typedef wavetable_osc::program<tester> program;

class tester : public generic_printer, public generic_tester<program, double>
{
public:
    tester(): generic_tester(this) {}

    void output(double (&v)[2])
    {
      record(v[0]);
      generic_printer::output(v);
    }
};

void main()
{
    tester t;
    t.run(15);

    vector<double> expected = {
      0.000,
      0.637,
      0.973,
      0.856,
      0.340,
      -0.340,
      -0.856,
      -0.973,
      -0.637,
      0.000,
      0.637,
      0.973,
      0.856,
      0.340,
      -0.340,
      -0.856
    };

    t.compare(expected);
}
