#include "module.hpp"
#include <fstream>

using namespace std;

namespace stream {

void print_code_range(ostream & out, const string & path, const code_range & range)
{
    if (path.empty())
        return;

    if (range.end.line != range.start.line)
        return;

    ifstream file(path);
    if (!file.is_open())
        return;

    string line_text;

    for(int line = 0; line < range.start.line; ++line)
    {
        std::getline(file, line_text);
    }

    cout << "    " << line_text << endl;
    cout << "    " << string(range.start.column-1,' ');
    cout << string(range.end.column - range.start.column, '^');
    cout << endl;
}

ostream & operator<< (ostream & s, const code_point & p)
{
    s << p.line << '.' << p.column;
    return s;
}

ostream & operator<< (ostream & s, const code_range & r)
{
    s << r.start;
    if (r.end.line != r.start.line || r.end.column != r.start.column)
        s << '-' << r.end;
    return s;
}

ostream & operator<< (ostream & s, const code_location & l)
{
    if (l.module)
    {
        s << "in module '" << l.module->name << "'";
        if (!l.module->source.path.empty())
            s << " at " << l.module->source.path;
        s << ':';
    }
    s << l.range;
    return s;
}

}
