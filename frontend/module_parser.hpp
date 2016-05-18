#ifndef STREAM_LANG_MODULE_PARSER_INCLUDED
#define STREAM_LANG_MODULE_PARSER_INCLUDED

#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <deque>
#include <unordered_map>
#include <utility>

#include "../common/ast.hpp"
#include "../common/module.hpp"

namespace stream {

using std::string;
using std::istream;
using std::unordered_map;
using std::list;
using std::deque;
using std::pair;

class module_parser
{
public:
    void set_import_dirs(const vector<string> & dirs) { m_import_dirs = dirs; }

    module * parse(const module_source &, istream & text);

    vector<module*> modules() const
    {
        return vector<module*>(m_ordered_modules.begin(), m_ordered_modules.end());
    }

private:
    pair<string, module*> parse_import
    (const module_source &, const ast::node_ptr & import);

    vector<string> m_import_dirs;
    list<module*> m_ordered_modules;
    unordered_map<string, module*> m_named_modules;
};

}

#endif // STREAM_LANG_MODULE_PARSER_INCLUDED
