#include "fir.cpp"

#include <iostream>

using namespace std;

class fir_tester
{
  typedef fir::program<fir_tester> program_t;

  program_t * program;

public:

  fir_tester()
  {
      program = new program_t;
      program->io = this;
  }

  void input_coefs(double k[3])
  {
    for (int i = 0; i < 3; ++i)
      k[i] = i+1;
  }

  void input_in1(double &v)
  {
    static int t = 0;
    v = t;
    t += 1;
  }

  void output(double v)
  {
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
  vector<double> expected =
  { 8, 14, 20, 26, 32, 38, 44, 50, 56, 62 };

  fir_tester t;
  t.run(10);

  bool ok = true;
  for (int i = 0; i < (int) expected.size(); ++i)
  {
      if (t.out[i] != expected[i])
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
}
