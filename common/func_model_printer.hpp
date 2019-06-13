/*
Compiler for language for stream processing

Copyright (C) 2015  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef STREAM_FUNCTIONAL_MODEL_PRINTER_INCLUDED
#define STREAM_FUNCTIONAL_MODEL_PRINTER_INCLUDED

#include "functional_model.hpp"
#include "linear_algebra.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace stream {
namespace functional {

using std::ostream;
using std::ostringstream;
using std::string;
using std::unordered_map;

class printer
{
public:
    printer();
    void print(const scope & s, ostream & out);
    void print(id_ptr id, ostream &);
    void print(expr_ptr expr, ostream &);
    void print(const linexpr & expr, ostream &);
    void indent() { ++level; }
    void unindent() { --level; }
    void set_print_scopes(bool enabled) { m_print_scopes = enabled; }
    void set_print_var_address(bool enabled) { m_print_var_address = enabled; }

private:
    string name(const var_ptr & var)
    {
        return var->name;
    }
#if 0
    string name(const array_var_ptr & var)
    {
        string & name = array_var_names[var];
        if (name.empty())
        {
            ostringstream text;
            text << 'i' << array_var_names.size();
            name = text.str();
        }
        return name;
    }

    string name(const func_var_ptr & var)
    {
        string & name = func_var_names[var];
        if (name.empty())
        {
            ostringstream text;
            text << 'v' << func_var_names.size();
            name = text.str();
        }
        return name;
    }
#endif
    string indentation() { return string(level * 2, ' '); }
    int level = 0;
    unordered_map<func_var_ptr, string> func_var_names;
    unordered_map<array_var_ptr, string> array_var_names;

    bool m_print_scopes = true;
    bool m_print_var_address = false;
};

}
}
#endif // STREAM_FUNCTIONAL_MODEL_PRINTER_INCLUDED

