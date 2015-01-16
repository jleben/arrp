/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

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

#include "ast.hpp"

#include <string>
#include <iostream>

namespace stream {
namespace ast {

using std::string;

class printer
{
public:
    printer():
        m_level(0)
    {
        init_type_names();
    }

    void print( node * n )
    {
        using namespace std;

        cout << indent();

        if (!n)
        {
            cout << "null";
            return;
        }

        string type_name = m_type_names[n->type];

        if (type_name.empty())
            cout << '<' << n->type << '>';
        else
            cout << type_name;

        switch (n->type)
        {
        case boolean:
            cout << ": " << (static_cast<leaf_node<bool>*>(n)->value ? "true" : "false");
            break;
        case integer_num:
            cout << ": " << static_cast<leaf_node<int>*>(n)->value;
            break;
        case real_num:
            cout << ": " << static_cast<leaf_node<double>*>(n)->value;
            break;
        case identifier:
            cout << ": \"" << static_cast<leaf_node<string>*>(n)->value << "\"";
            break;
        default:
        {
            list_node *parent = dynamic_cast<list_node*>(n);
            if (!parent)
                break;

            if (!parent->elements.empty())
            {
                cout << ": { " << endl;
                m_level++;
                for (sp<node> & child : parent->elements)
                {
                    print(child.get());
                    cout << endl;
                }
                m_level--;
                cout << indent() << "}";
            }
            else
            {
                cout << ": { }";
            }
            break;
        }
        }
    }

private:
    void init_type_names()
    {
        m_type_names[kwd_let] = "'let'";
        m_type_names[kwd_for] = "'for'";
        m_type_names[kwd_reduce] = "'reduce'";
        m_type_names[oppose] = "not";
        m_type_names[logic_or] = "or";
        m_type_names[logic_and] = "and";
        m_type_names[negate] = "-";
        m_type_names[add] = "+";
        m_type_names[subtract] = "-";
        m_type_names[multiply] = "*";
        m_type_names[divide] = "/";
        m_type_names[divide_integer] = ":";
        m_type_names[lesser] = "<";
        m_type_names[greater] = ">";
        m_type_names[lesser_or_equal] = "<=";
        m_type_names[greater_or_equal] = ">=";
        m_type_names[equal] = "==";
        m_type_names[not_equal] = "!=";
        m_type_names[integer_num] = "int";
        m_type_names[real_num] = "real";
        m_type_names[boolean] = "bool";
        m_type_names[identifier] = "id";
        m_type_names[range] = "range";
        m_type_names[transpose_expression] = "transpose";
        m_type_names[slice_expression] = "slice";
        m_type_names[call_expression] = "call";
        m_type_names[if_expression] = "if";
        m_type_names[for_expression] = "for";
        m_type_names[for_iteration] = "for-iter";
        m_type_names[for_iteration_list] = "for-iter-list";
        m_type_names[reduce_expression] = "reduce";
        m_type_names[hash_expression] = "hash";
        m_type_names[expression_block] = "expr-block";
        m_type_names[statement] = "statement";
        m_type_names[id_list] = "id-list";
        m_type_names[int_list] = "int-list";
        m_type_names[expression_list] = "expr-list";
        m_type_names[statement_list] = "stmt-list";
        m_type_names[program] = "program";
    }

    string indent()
    {
        return string(m_level * 2, ' ');
    }

    int m_level;
    string m_type_names[ast::node_type_count];
};

} // namespace ast
} // namespace stream
