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

#include "../common/primitives.hpp"
#include "../common/error.hpp"

#include <cassert>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <initializer_list>
#include <complex>
#include <deque>

namespace stream {
namespace cpp_gen {

using std::vector;
using std::string;
using std::ostream;
using std::unordered_map;
using std::ostringstream;
using std::stack;
using std::deque;

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

enum inline_mode
{
    default_inline,
    explicit_inline
};

class module_member;
typedef std::shared_ptr<module_member> module_member_ptr;

class expression;
typedef std::shared_ptr<expression> expression_ptr;

class module
{
public:
    string next_id(const string & prefix)
    {
        ostringstream name;
        name << prefix;
        int index = ++m_identifiers[prefix];
        name << index;
        return name.str();
    }

    vector<module_member_ptr> members;

    void generate(state &, ostream &);

private:
    unordered_map<string,int> m_identifiers;
};

class module_member
{
public:
    virtual ~module_member() {}
    virtual void generate(state &, ostream &) = 0;
};

class include_dir : public module_member
{
public:
    enum location
    {
        local,
        global
    };

    location loc = global;
    string filename;

    include_dir() {}
    include_dir(const string & f, location loc = global): loc(loc), filename(f) {}
    void generate(state &, ostream &stream)
    {
        stream << "#include ";
        if (loc == local)
            stream << "\"";
        else
            stream << "<";

        stream << filename;

        if (loc == local)
            stream << "\"";
        else
            stream << ">";
    }
};

class namespace_member : public module_member {};
typedef std::shared_ptr<namespace_member> namespace_member_ptr;

class namespace_node : public namespace_member
{
public:
    string name;
    vector<namespace_member_ptr> members;
    void generate(state &, ostream &);
};

class extern_c_node : public namespace_member
{
public:
    vector<namespace_member_ptr> members;
    void generate(state &, ostream &);
};

class class_member
{
public:
    virtual ~class_member() {}
    virtual void generate(state &, ostream &) = 0;
};
typedef std::shared_ptr<class_member> class_member_ptr;

class class_section
{
public:
    access_type access;
    vector<class_member_ptr> members;

    class_section(access_type access = default_access): access(access) {}
    void generate(state &, ostream &);
};

class class_node : public namespace_member, public class_member
{
public:
    class_key key;
    vector<string> template_parameters;
    int alignment = 0;
    string name;
    vector<class_section> sections;

    class_node(class_key key, const string & name = string()):
        key(key),
        name(name)
    {}

    void generate(state &, ostream &);
};

// Types

class type
{
public:
    virtual ~type() {}
    virtual void generate(state &, ostream &) = 0;
};

typedef std::shared_ptr<type> type_ptr;

class base_type : public type
{
};

typedef std::shared_ptr<base_type> base_type_ptr;

class basic_type : public base_type
{
public:
    basic_type(const string & name):
        name(name),
        is_const(false) {}
    string name;
    bool is_const;
    void generate(state &, ostream &);
};

typedef std::shared_ptr<basic_type> basic_type_ptr;

class pointer_type : public base_type
{
public:
    base_type_ptr base;
    bool is_const;

    pointer_type(base_type_ptr base):
        base(base),
        is_const(false)
    {}
    void generate(state &, ostream &);
};

class array_type : public base_type
{
public:
    base_type_ptr base;
    int size;

    array_type(base_type_ptr base):
        base(base)
    {}

    void generate(state &, ostream &);
};

class reference_type_node : public type
{
public:
    reference_type_node(base_type_ptr base):
        base(base)
    {}
    base_type_ptr base;
    void generate(state &, ostream &);
};

// Declarations

class variable_decl
{
public:
    // FIXME: alignment shouldn't apply to function params
    int alignment=0;
    type_ptr type;
    string name;
    expression_ptr value;

    virtual void generate(state &, ostream &);

    variable_decl() {}
    variable_decl(type_ptr t, const string & name,
                  expression_ptr val = expression_ptr()):
        type(t), name(name), value(val) {}
};

typedef std::shared_ptr<variable_decl> variable_decl_ptr;

class array_decl : public variable_decl
{
public:
    vector<int> size;

    array_decl(type_ptr t, const string & name, const vector<int> & size):
        variable_decl(t, name),
        size(size)
    {}
    virtual void generate(state &, ostream &);
};

class custom_decl : public namespace_member, public class_member
{
public:
    string text;
    virtual void generate(state &, ostream &);
};

class data_field : public class_member
{
public:
    variable_decl_ptr var;

    data_field() {}
    data_field(variable_decl_ptr var): var(var) {}

    virtual void generate(state &state, ostream &stream)
    {
        var->generate(state, stream);
        stream << ";";
    }
};

class func_signature
{
public:
    func_signature(const string & name, inline_mode in = default_inline):
        inlining(in),
        name(name),
        type(std::make_shared<basic_type>("void"))
    {}
    func_signature(const string & name,
                   const vector<variable_decl_ptr> & p,
                   inline_mode in = default_inline):
        inlining(in),
        name(name),
        type(std::make_shared<basic_type>("void")),
        parameters(p)
    {}
    func_signature(const string & name,
                   const vector<variable_decl_ptr> & p,
                   type_ptr r,
                   inline_mode in = default_inline):
        inlining(in),
        name(name),
        type(r),
        parameters(p)
    {}

    inline_mode inlining = default_inline;
    string name;
    type_ptr type;
    vector<string> template_parameters;
    vector<variable_decl_ptr> parameters;
    void generate(state &, ostream &);
};

typedef std::shared_ptr<func_signature> func_sig_ptr;

class func_decl : public namespace_member, public class_member
{
public:
    func_sig_ptr signature;
    void generate(state &, ostream &);

    func_decl() {}
    func_decl(func_sig_ptr sig): signature(sig) {}
};

class using_decl : public namespace_member, public class_member
{
public:
    const string name;

    using_decl(const string & name): name(name) {}
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

template<typename X> inline
void print_literal(ostream & stream, const X & v)
{
    stream << v;
}

inline void print_literal(ostream & stream, double v)
{
    using namespace std;
    //cout << "printing double literal" << endl;
#if 1
    char buf[128];
    snprintf(buf, 128, "%a", v);
    stream << buf;
#endif
#if 0
    stream.unsetf(std::ios_base::floatfield);
    stream << std::showpoint << std::setprecision(12);
    stream << v;
#endif
}

inline void print_literal(ostream & stream, float v)
{
    using namespace std;
    //cout << "printing float literal" << endl;
    char buf[128];
    snprintf(buf, 128, "%af", v);
    stream << buf;
}

template <typename T>
class literal_expression : public expression
{
public:
    literal_expression(T v = T()): value(v) {}

    T value;

    void generate(state &, ostream & stream)
    {
        print_literal(stream, value);
    }
};

template <typename T> string complex_type_name();

template <> inline
string complex_type_name<double>() { return "complex<double>"; }

template <> inline
string complex_type_name<float>() { return "complex<float>"; }

template <typename T>
class literal_expression<std::complex<T>> : public expression
{
public:
    typedef std::complex<T> value_type;

    literal_expression(value_type v = value_type()): value(v) {}

    value_type value;

    void generate(state &, ostream & stream)
    {
        // FIXME:
        // Generate complex literals?
        stream << complex_type_name<T>();
        stream << '(';
        print_literal(stream, value.real());
        stream << ',';
        print_literal(stream, value.imag());
        stream << ')';
    }
};

template <typename T>
inline expression_ptr literal(T v)
{
    return std::make_shared<literal_expression<T>>(v);
}

class id_expression : public expression
{
public:
    string name;

    id_expression() {}
    id_expression(string name): name(name) {}
    void generate(state &, ostream &);
};

enum struct op
{
    scope_resolution,
    function_call,
    array_subscript,
    cast,
    member_of_reference,
    member_of_pointer,
    dereference,
    address,
    logic_neg,
    logic_and,
    logic_or,
    bit_neg,
    bit_and,
    bit_or,
    bit_xor,
    bit_left,
    bit_right,
    lesser,
    lesser_or_equal,
    greater,
    greater_or_equal,
    equal,
    not_equal,
    mult,
    div,
    rem,
    add,
    sub,
    u_plus,
    u_minus,
    post_incr,
    post_decr,
    pre_incr,
    pre_decr,
    assign,
    assign_add,
    assign_sub,
    assign_mult,
    assign_div,
    assign_rem,
    conditional
};

class un_op_expression : public expression
{
public:
    cpp_gen::op op;
    expression_ptr rhs;

    un_op_expression() {}
    un_op_expression(cpp_gen::op op, expression_ptr r):
        op(op), rhs(r)
    {}
    void generate(state &, ostream &);
};

class bin_op_expression : public expression
{
public:
    cpp_gen::op op;
    expression_ptr lhs;
    expression_ptr rhs;

    bin_op_expression() {}
    bin_op_expression(cpp_gen::op op): op(op) {}
    bin_op_expression(cpp_gen::op op, expression_ptr l, expression_ptr r):
        op(op), lhs(l), rhs(r)
    {}
    void generate(state &, ostream &);
};

class if_expression : public expression
{
public:
    expression_ptr condition;
    expression_ptr true_expr;
    expression_ptr false_expr;

    if_expression() {}
    if_expression(expression_ptr c, expression_ptr t, expression_ptr f):
        condition(c), true_expr(t), false_expr(f)
    {}
    void generate(state &, ostream &);
};

class call_expression : public expression
{
public:
    expression_ptr callee;
    vector<expression_ptr> args;

    call_expression(expression_ptr c, const vector<expression_ptr> & a):
        callee(c), args(a)
    {}
    template <typename ...T>
    call_expression(string name, T ... a ):
        callee(std::make_shared<id_expression>(name)),
        args {a...}
    {}
    template <typename ...T>
    call_expression(expression_ptr c, T ... a ):
        callee(c),
        args {a...}
    {}

    void generate(state &, ostream &);
};

class var_decl_expression : public expression
{
public:
    variable_decl_ptr decl;
    var_decl_expression() {}
    var_decl_expression(variable_decl_ptr decl): decl(decl) {}
    void generate(cpp_gen::state & state, ostream & stream)
    {
        decl->generate(state, stream);
    }
};

class cast_expression : public expression
{
public:
    type_ptr type;
    expression_ptr expr;

    cast_expression() {}
    cast_expression(type_ptr t, expression_ptr e): type(t), expr(e) {}
    void generate(cpp_gen::state & state, ostream & stream);
};

class array_access_expression : public expression
{
public:
    expression_ptr id;
    vector<expression_ptr> index;

    array_access_expression() {}
    array_access_expression(expression_ptr id, const vector<expression_ptr> & index):
        id(id), index(index)
    {}
    void generate(cpp_gen::state & state, ostream & stream);
};

// Statements

class statement
{
public:
    virtual ~statement(){}
    virtual void generate(state &, ostream &) = 0;
    virtual void generate_nested(state &, ostream &);
};

typedef std::shared_ptr<statement> statement_ptr;

class block_statement : public statement
{
public:
    vector<statement_ptr> statements;

    block_statement() {}
    block_statement(const vector<statement_ptr> & stmts): statements(stmts) {}

    void generate(state &, ostream &) override;
    virtual void generate_nested(state &, ostream &) override;
};

class expr_statement : public statement
{
public:
    expression_ptr expr;

    expr_statement(): expr(nullptr) {}
    expr_statement(expression_ptr e): expr(e) {}
    void generate(state &, ostream &);
};

class complex_statement : public statement
{
public:
    virtual void generate_nested(state &, ostream &) override;
};

class if_statement : public complex_statement
{
public:
    if_statement() {}
    if_statement(expression_ptr c, statement_ptr t, statement_ptr f):
        condition(c),
        true_part(t),
        false_part(f)
    {}
    expression_ptr condition;
    statement_ptr true_part;
    statement_ptr false_part;
    void generate(state &, ostream &);
};

class for_statement : public complex_statement
{
public:
    expression_ptr initialization;
    expression_ptr condition;
    expression_ptr update;
    statement_ptr body;
    bool is_parallel = false;
    bool is_vector = false;
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

class return_statement : public statement
{
public:
    expression_ptr value;

    return_statement() {}
    return_statement(expression_ptr v): value(v) {}
    void generate(state &, ostream &);
};

// Function

class func_def :  public namespace_member, public class_member
{
public:
    func_def(func_sig_ptr sig): signature(sig) {}

    func_sig_ptr signature;
    block_statement body;
    bool is_inline = false;

    void generate(state &, ostream &);
};

// Constructor

class builder
{
public:
    using block_ptr = vector<statement_ptr>*;

    struct block_info
    {
        block_info(vector<statement_ptr>* stmts): stmts(stmts) {}

        vector<statement_ptr> * stmts;
        string induction_var;
    };

    builder(module *m): m_module(m) {}

    void set_current_function(func_signature *f) { m_func = f; }
    func_signature *current_function() { return m_func; }

    void set_current_function(func_def *f)
    {
        m_func = f->signature.get();
        m_blocks.clear();
        m_blocks.push_back(&f->body.statements);
    }

    void push(block_statement & block)
    {
        m_blocks.push_back(&block.statements);
    }

    void push(vector<statement_ptr> * statements)
    {
        m_blocks.push_back(statements);
    }

    void pop()
    {
        m_blocks.pop_back();
    }

    int block_count() { return m_blocks.size(); }

    int current_block_level() { return int(m_blocks.size()) - 1; }

    block_info & current_block() { return m_blocks.back(); }

    block_info & block(int level) { return m_blocks[level]; }

    void add(statement_ptr stmt) { current_block().stmts->push_back(stmt); }

    void add(expression_ptr expr) { add(std::make_shared<expr_statement>(expr)); }

    string new_var_id() { return m_module->next_id("v"); }

    expression_ptr new_var(type_ptr t, string & id)
    {
        id = new_var_id();
        auto decl = std::make_shared<variable_decl>(t, id);
        return std::make_shared<var_decl_expression>(decl);
    }

private:

    cpp_gen::module * m_module;
    func_signature *m_func;
    deque<block_info> m_blocks;
};

// Helpers


inline expression_ptr call(expression_ptr f, const vector<expression_ptr> & a)
{
    return std::make_shared<call_expression>(f, a);
}

inline expression_ptr binop(op o, expression_ptr l, expression_ptr r)
{
    return std::make_shared<bin_op_expression>(o, l, r);
}

inline expression_ptr unop(op o, expression_ptr l)
{
    return std::make_shared<un_op_expression>(o, l);
}

inline expression_ptr assign(expression_ptr lhs, expression_ptr rhs)
{
    return binop(op::assign, lhs, rhs);
}

inline std::shared_ptr<id_expression> make_id(const string & name)
{
    return std::make_shared<id_expression>(name);
}

inline variable_decl_ptr decl(type_ptr t, const string & id,
                              expression_ptr val = expression_ptr())
{
    return std::make_shared<variable_decl>(t, id, val);
}

inline variable_decl_ptr decl(type_ptr t, const id_expression & id,
                              expression_ptr val = expression_ptr())
{
    return std::make_shared<variable_decl>(t, id.name, val);
}

inline expression_ptr decl_expr(type_ptr t, const string & id,
                                expression_ptr val = expression_ptr())
{
    return std::make_shared<var_decl_expression>(decl(t,id,val));
}

inline expression_ptr decl_expr(type_ptr t, const id_expression & id,
                                expression_ptr val = expression_ptr())
{
    return std::make_shared<var_decl_expression>(decl(t,id.name,val));
}

inline base_type_ptr pointer(base_type_ptr t)
{
    return std::make_shared<pointer_type>(t);
}

inline basic_type_ptr auto_type()
{
    return std::make_shared<basic_type>("auto");
}

inline basic_type_ptr float_type()
{
    return std::make_shared<basic_type>("float");
}
inline basic_type_ptr double_type()
{
    return std::make_shared<basic_type>("double");
}
inline basic_type_ptr int_type()
{
    return std::make_shared<basic_type>("int");
}
inline basic_type_ptr bool_type()
{
    return std::make_shared<basic_type>("bool");
}

inline expression_ptr cast(type_ptr t, expression_ptr e)
{
    return std::make_shared<cast_expression>(t, e);
}

inline type_ptr reference(base_type_ptr t)
{
    return std::make_shared<reference_type_node>(t);
}

inline statement_ptr block(const vector<statement_ptr> & stmts)
{
    return std::make_shared<block_statement>(stmts);
}

inline statement_ptr stmt(expression_ptr e)
{
    return std::make_shared<expr_statement>(e);
}

} // namespace cpp_gen
} // namespace stream


#endif // STREAM_LANG_CPP_GENERATION_INCLUDED
