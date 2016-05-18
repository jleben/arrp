#ifndef STREAM_LANG_MODULE_INCLUDED
#define STREAM_LANG_MODULE_INCLUDED

#include <string>
#include <iostream>
#include <unordered_map>

#include "ast.hpp"
#include "error.hpp"

namespace stream {

using std::string;
using std::istream;
using std::unordered_map;

struct module_source
{
    string dir;
    string path;
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
};

struct code_location
{
    stream::module * module = nullptr;
    code_range range;

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

void print_code_range(ostream &, const string & path, const code_range &);

ostream & operator<< (ostream & s, const code_point & p);
ostream & operator<< (ostream & s, const code_range & r);
ostream & operator<< (ostream & s, const code_location & l);

}

#endif // STREAM_LANG_MODULE_INCLUDED
