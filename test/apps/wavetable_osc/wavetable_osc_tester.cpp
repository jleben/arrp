#include "wavetable_osc.cpp"
#include "../../common/generic_printer.hpp"
#include "../../common/generic_tester.hpp"

class tester;
typedef wavetable_osc::program<tester> program;

class tester : public generic_printer, public generic_tester<program>
{
public:
    tester(): generic_tester(this) {}
};

void main()
{
    tester t;
    t.run(15);
}
