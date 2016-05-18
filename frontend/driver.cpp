#include "driver.hpp"
#include "../common/module.hpp"

using namespace std;

namespace stream {
namespace parsing {

void driver::error(const parser::location_type& loc, const std::string& m)
{
    code_range range;
    range.start.line = loc.begin.line;
    range.start.column = loc.begin.column;
    range.end.line = loc.end.line;
    range.end.column = loc.end.column;

    cerr << "** ERROR at " << m_path << ":" << range.start << ": "
         << m << endl;
    print_code_range(cerr, m_path, range);
}

}
}
