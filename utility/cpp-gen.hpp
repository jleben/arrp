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

#ifndef STREAM_LANG_CPP_GENERATION_INCLUDED
#define STREAM_LANG_CPP_GENERATION_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <memory>
#include <initializer_list>

namespace stream {
namespace cpp_gen {

using std::vector;
using std::string;
using std::ostream;

enum indentation_type
{
    tab_indentation,
    space_indentation
};

class options
{
public:
    options():
        indentation(space_indentation),
        indentation_size(4)
    {}

    indentation_type indentation;
    int indentation_size;
};

class state
{
public:
    state(const options & opt = options()): opt(opt), indentation_level(0) {}

    options opt;

    int indentation_level;

    void new_line(ostream & stream)
    {
        stream << std::endl;
        switch(opt.indentation)
        {
        case tab_indentation:
            stream << string(indentation_level,'\t');
            break;
        case space_indentation:
            stream << string(indentation_level * opt.indentation_size, ' ');
            break;
        }
    }
    void increase_indentation()
    {
        ++indentation_level;
    }
    void decrease_indentation()
    {
        assert(indentation_level > 0);
        --indentation_level;
    }
};

enum access_type
{
    default_access,
    private_access,
    public_access
};

enum class_key
{
    class_class,
    struct_class
};

class program_member
{
public:
    virtual ~program_member() {}
    virtual void generate(state &, ostream &) = 0;
};

class program
{
public:
    ~program()
    {
        for (auto member : members)
            delete member;
    }
    vector<program_member*> members;
    void generate(state &, ostream &);
};

class namespace_member_node : public program_member
{};

class namespace_node : public namespace_member_node
{
public:
    ~namespace_node()
    {
        for (auto member : members)
            delete member;
    }
    string name;
    vector<namespace_member_node*> members;
    void generate(state &, ostream &);
};

class extern_c_node : public namespace_member_node
{
public:
    ~extern_c_node()
    {
        for (auto member : members)
            delete member;
    }
    vector<namespace_member_node*> members;
    void generate(state &, ostream &);
};

class class_member_node
{
public:
    virtual ~class_member_node() {}
    virtual void generate(state &, ostream &) = 0;
};

class class_section_node
{
public:
    class_section_node(access_type access = default_access): access(access) {}
    ~class_section_node()
    {
        for(auto member : members)
            delete member;
    }
    access_type access;
    vector<class_member_node*> members;

    void generate(state &, ostream &);
};

class class_node : public namespace_member_node, public class_member_node
{
public:
    class_node(class_key key, const string & name = string()):
        key(key),
        name(name)
    {}

    ~class_node()
    {
        for (auto section : sections)
            delete section;
    }

    class_key key;
    string name;
    vector<class_section_node*> sections;

    void generate(state &, ostream &);
};

class type_node
{
public:
    virtual ~type_node() {}
    virtual void generate(state &, ostream &) = 0;
};

class base_type_node : public type_node
{
};

class basic_type_node : public base_type_node
{
public:
    basic_type_node(const string & name):
        name(name),
        is_const(false) {}
    string name;
    bool is_const;
    void generate(state &, ostream &);
};

class pointer_type_node : public base_type_node
{
public:
    pointer_type_node(base_type_node *base):
        base_type(base),
        is_const(false)
    {}
    ~pointer_type_node()
    {
        delete base_type;
    }
    base_type_node * base_type;
    bool is_const;
    void generate(state &, ostream &);
};

class reference_type_node : public type_node
{
public:
    reference_type_node(base_type_node *base):
        base_type(base)
    {}
    ~reference_type_node()
    {
        delete base_type;
    }
    base_type_node * base_type;
    void generate(state &, ostream &);
};

class variable_decl_node
{
public:
    variable_decl_node(type_node *t, const string & name = string()):
        type(t),
        name(name)
    {}
    virtual ~variable_decl_node()
    {
        delete type;
    }
    type_node *type;
    string name;
    virtual void generate(state &, ostream &);
};

class data_field_decl_node : public variable_decl_node, public class_member_node
{
public:
    data_field_decl_node(type_node *t, const string & name):
        variable_decl_node(t, name)
    {}
    virtual void generate(state &state, ostream &stream)
    {
        variable_decl_node::generate(state, stream);
        stream << ";";
    }
};

class func_signature_node
{
public:
    ~func_signature_node()
    {
        delete type;
        for(auto param: parameters)
            delete param;
    }
    string name;
    type_node *type;
    vector<variable_decl_node*> parameters;
    void generate(state &, ostream &);
};

typedef std::shared_ptr<func_signature_node> func_sig_ptr;

class func_decl_node : public namespace_member_node, public class_member_node
{
public:
    func_decl_node(func_signature_node *sig): signature(sig) {}
    ~func_decl_node()
    {
        delete signature;
    }
    func_signature_node *signature;
    void generate(state &, ostream &);
};

class using_decl : public namespace_member_node, public class_member_node
{
public:
    using_decl(const string & name):
        name(name)
    {}
    const string name;
    void generate(state &, ostream &stream)
    {
        stream << "using " << name << ';';
    }
};

// Expressions

class expression
{
public:
    virtual ~expression(){}
    virtual void generate(state &, ostream &) = 0;
};

typedef std::shared_ptr<expression> expression_ptr;

template <typename T>
class literal_expression : public expression
{
public:
    literal_expression(T v = T()): value(v) {}

    T value;

    void generate(state &, ostream & stream)
    {
        stream << value;
    }
};

class id_expression : public expression
{
public:
    string name;

    id_expression() {}
    id_expression(string name): name(name) {}
    void generate(state &, ostream &);
};

class un_op_expression : public expression
{
public:
    string op;
    expression_ptr rhs;

    un_op_expression() {}
    un_op_expression(string op, expression_ptr r):
        op(op), rhs(r)
    {}
    void generate(state &, ostream &);
};

class bin_op_expression : public expression
{
public:
    string op;
    expression_ptr lhs;
    expression_ptr rhs;

    bin_op_expression() {}
    bin_op_expression(string op): op(op) {}
    bin_op_expression(string op, expression_ptr l, expression_ptr r):
        op(op), lhs(l), rhs(r)
    {}
    void generate(state &, ostream &);
};

class call_expression : public expression
{
public:
    string func_name;
    vector<expression_ptr> args;

    call_expression(string f, std::initializer_list<expression_ptr> a):
        func_name(f), args(a)
    {}
    call_expression(string f, const vector<expression_ptr> & a):
        func_name(f), args(a)
    {}
    void generate(state &, ostream &);
};

// Statements

class statement
{
public:
    virtual ~statement(){}
    virtual void generate(state &, ostream &) = 0;
};

typedef std::shared_ptr<statement> statement_ptr;

class block_statement : public statement
{
public:
    vector<statement_ptr> statements;

    void generate(state &, ostream &);
};

class expr_statement : public statement
{
public:
    expression_ptr expr;

    expr_statement(): expr(nullptr) {}
    expr_statement(expression_ptr e): expr(e) {}
    void generate(state &, ostream &);
};

class if_statement : public statement
{
public:
    expression_ptr condition;
    statement_ptr body;
    void generate(state &, ostream &);
};

class for_statement : public statement
{
public:
    expression_ptr initialization;
    expression_ptr condition;
    expression_ptr update;
    statement_ptr body;
    void generate(state &, ostream &);
};

class comment_statement : public statement
{
public:
    string text;

    comment_statement(const string & t): text(t) {}
    void generate(state &, ostream & stream)
    {
        stream << "// " << text;
    }
};

// Function

class func_def_node :  public namespace_member_node, public class_member_node
{
public:
    func_def_node(func_sig_ptr sig): signature(sig) {}

    func_sig_ptr signature;
    block_statement body;

    void generate(state &, ostream &);
};


} // namespace cpp_gen
} // namespace stream


#endif // STREAM_LANG_CPP_GENERATION_INCLUDED
