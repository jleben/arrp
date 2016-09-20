#include "osc.cpp"

#include <iostream>
#include <iomanip>

using namespace std;

class osc_tester
{
  typedef osc::program<osc_tester> program_t;

  program_t * program;

public:

  osc_tester()
  {
      program = new program_t;
      program->io = this;
  }

  void output(double v)
  {
      //cout << std::fixed << std::setprecision(6) << v << endl;
      out.push_back(v);
  }

  void run(int n)
  {
    program->prelude();
    while(n--)
      program->period();
  }

  vector<double> out;
};


int main()
{
    osc_tester t;
    t.run(15);

    vector<double> expected =
    {
        0.000000,
        0.587785,
        0.951057,
        0.951057,
        0.587785,
        0.000000,
        -0.587785,
        -0.951057,
        -0.951057,
        -0.587785,
        -0.000000,
        0.587785,
        0.951057,
        0.951057,
        0.587785,
        0.000000
    };

#if 1
  bool ok = true;
  for (int i = 0; i < (int) expected.size(); ++i)
  {
      if (std::abs(t.out[i] - expected[i]) > 0.000001)
      {
          ok = false;
          cout << "out[" << i << "] = " << (t.out[i])
               << " but expected[" << i << "] = " << (expected[i])
               << endl;
      }
  }

  if (ok)
      cout << "OK." << endl;
  else
      cout << "FAILED." << endl;
#endif
}
