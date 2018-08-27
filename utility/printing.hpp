#include <iostream>
#include <vector>

namespace arrp {

using std::vector;
using std::string;
using std::ostream;

template <typename seq_type>
struct printable
{
    const seq_type & seq;
    string sep;

    printable(const seq_type & seq, const string & sep): seq(seq), sep(sep) {}
};

template <typename seq_type>
ostream & operator<<(ostream & s, const printable<seq_type> & p)
{
    auto i = p.seq.begin();
    if (i == p.seq.end())
        return s;
    s << *i;
    ++i;
    for(; i != p.seq.end(); ++i)
        s << p.sep << *i;
    return s;
}

}
