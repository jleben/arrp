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
        case integer_num:
            cout << ": " << static_cast<leaf_node<int>*>(n)->value;
            break;
        case real_num:
            cout << ": " << static_cast<leaf_node<double>*>(n)->value;
            break;
        case identifier:
            cout << ": \"" << static_cast<leaf_node<string>*>(n)->value << "\"";
            break;
        case negate:
        case add:
        case subtract:
        case multiply:
        case divide:
        case lesser:
        case greater:
        case lesser_or_equal:
        case greater_or_equal:
        case equal:
        case not_equal:
        case range:
        case call_expression:
        case for_expression:
        case for_iteration:
        case for_iteration_list:
        case reduce_expression:
        case hash_expression:
        case transpose_expression:
        case slice_expression:
        case expression_block:
        case statement:
        case id_list:
        case int_list:
        case expression_list:
        case statement_list:
        case program:
        {
            list_node *parent = static_cast<list_node*>(n);
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
        m_type_names[negate] = "-";
        m_type_names[add] = "+";
        m_type_names[subtract] = "-";
        m_type_names[multiply] = "*";
        m_type_names[divide] = "/";
        m_type_names[lesser] = "<";
        m_type_names[greater] = ">";
        m_type_names[lesser_or_equal] = "<=";
        m_type_names[greater_or_equal] = ">=";
        m_type_names[equal] = "==";
        m_type_names[not_equal] = "!=";
        m_type_names[integer_num] = "int";
        m_type_names[real_num] = "real";
        m_type_names[identifier] = "id";
        m_type_names[range] = "range";
        m_type_names[transpose_expression] = "transpose";
        m_type_names[slice_expression] = "slice";
        m_type_names[call_expression] = "call";
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
