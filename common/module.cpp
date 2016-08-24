#include "module.hpp"
#include <fstream>

using namespace std;

namespace stream {

void print_code_range(ostream & out, const module_source & source, const code_range & range)
{
    if (source.path.empty())
    {
        if (source.text.empty())
            return;

        istringstream in(source.text);
        print_code_range(out, in, range);
    }
    else
    {
        ifstream file(source.path);
        if (!file.is_open())
            return;
        print_code_range(out, file, range);
    }
}

void print_code_range(ostream & out, istream & in, const code_range & range)
{
    if (range.is_empty())
        return;

    if (range.end.line != range.start.line)
        return;

    string line_text;

    for(int line = 0; line < range.start.line; ++line)
    {
        std::getline(in, line_text);
    }

    out << "    " << line_text << endl;
    out << "    " << string(range.start.column-1,' ');
    out << string(range.end.column - range.start.column, '^');
    out << endl;
}

ostream & operator<< (ostream & s, const code_point & p)
{
    s << p.line << '.' << p.column;
    return s;
}

ostream & operator<< (ostream & s, const code_range & r)
{
    if (r.is_empty())
        return s;

    s << r.start << '-' << r.end;
    return s;
}

ostream & operator<< (ostream & s, const code_location & l)
{
    if (l.module)
    {
        s << "module '" << l.module->name << "'";
        if (!l.module->source.path.empty())
            s << ": " << l.module->source.path;
        if (!l.range.is_empty())
            s << ':';
    }
    s << l.range;
    return s;
}

}
