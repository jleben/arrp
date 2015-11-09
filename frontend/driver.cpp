#include "driver.hpp"

namespace stream {
namespace parsing {

void driver::error(const parser::location_type& l, const std::string& m)
{
    using namespace std;

    cout << "ERROR [" << l.begin.line << ":" << l.begin.column << "]: "
         << m << endl;

    m_input.seekg(0);

    if (l.end.line == l.begin.line)
    {
        string line_text;

        for(int line = 0; line < l.begin.line; ++line)
        {
            std::getline(m_input, line_text);
        }

        cout << "    " << line_text << endl;
        cout << "    " << string(l.begin.column-1,' ');
        cout << string(l.end.column - l.begin.column, '^');
        cout << endl;
    }
}

}
}
