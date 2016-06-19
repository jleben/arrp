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
    {}

    void print( node * n )
    {
        using namespace std;

        cout << indent();

        if (!n)
        {
            cout << "null";
            return;
        }

        switch (n->type)
        {
        case constant:
        {
            if (auto b = dynamic_cast<leaf_node<bool>*>(n))
                cout << (b->value ? "true" : "false");
            else if(auto i = dynamic_cast<leaf_node<int>*>(n))
                cout << i->value;
            else if(auto d = dynamic_cast<leaf_node<double>*>(n))
                cout << d->value;
            else if(auto s = dynamic_cast<leaf_node<string>*>(n))
                cout << s->value;
            else if(auto op = dynamic_cast<leaf_node<primitive_op>*>(n))
                cout << name_of_primitive(op->value);
            else
                cout << "?";
            break;
        }
        case identifier:
        {
            cout << "id: " << '"' << n->as_leaf<string>()->value << '"';
            break;
        }
        default:
        {
            switch(n->type)
            {
            case anonymous:
                break;
            case program:
                cout << "program"; break;
            case array_def:
                cout << "array-def"; break;
            case array_params:
                cout << "array-params"; break;
            case array_param:
                cout << "array-param"; break;
            case array_apply:
                cout << "array-apply"; break;
            case lambda:
                cout << "lambda"; break;
            case binding:
                cout << "bind"; break;
            case local_binding:
                cout << "scope"; break;
            case func_apply:
                cout << "func-apply"; break;
            case primitive:
                cout << "primitive"; break;
            default:
                cout << "?";
            }

            list_node *parent = dynamic_cast<list_node*>(n);
            if (!parent)
                break;

            if (n->type != anonymous)
                cout << ": ";

            if (!parent->elements.empty())
            {
                cout << "{ " << endl;
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
                cout << "{ }";
            }
            break;
        }
        }
    }

private:
    string indent()
    {
        return string(m_level * 2, ' ');
    }

    int m_level;
};

} // namespace ast
} // namespace stream
