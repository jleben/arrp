#ifndef STREAM_LANG_MODULE_INCLUDED
#define STREAM_LANG_MODULE_INCLUDED

#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "ast.hpp"
#include "error.hpp"

namespace stream {

using std::string;
using std::istream;
using std::unordered_map;
using std::vector;

struct module_source
{
    string dir;
    string path;
    string text;
};

struct module
{
    module_source source;
    string name;
    ast::node_ptr ast;
    unordered_map<string, module*> imports;
};

struct code_point
{
    unsigned int line = 0;
    unsigned int column = 0;
};

struct code_range
{
    code_point start;
    code_point end;

    code_range() {}

    code_range(const parsing::location & loc)
    {
        start.line = loc.begin.line;
        start.column = loc.begin.column;
        end.line = loc.end.line;
        end.column = loc.end.column;
    }

    bool is_empty() const
    {
        return start.line == end.line && start.column == end.column;
    }
};

struct code_location
{
    const stream::module * module = nullptr;
    code_range range;

    code_location() {}
    code_location(const stream::module *m, const code_range & r = code_range()):
        module(m), range(r) {}

    string path()
    {
        if (module)
            return module->source.path;
        else
            return string();
    }
};

struct parser_error : error {};

struct io_error : error { using error::error; };

void print_code_range(ostream &, const module_source & source, const code_range &);
void print_code_range(ostream &, istream &, const code_range &);

ostream & operator<< (ostream & s, const code_point & p);
ostream & operator<< (ostream & s, const code_range & r);
ostream & operator<< (ostream & s, const code_location & l);

}

#endif // STREAM_LANG_MODULE_INCLUDED
