#ifndef STREAM_LANG_AST_INCLUDED
#define STREAM_LANG_AST_INCLUDED

#include <vector>
#include <memory>
#include <string>
#include <cassert>
#include <iostream>

namespace stream
{

using std::vector;
using std::string;
template <typename T> using sp = std::shared_ptr<T>;

namespace semantic {
struct type;
using type_ptr = std::shared_ptr<type>;
}

namespace ast {

struct node;

using node_ptr = std::shared_ptr<node>;

struct semantic_value : public node_ptr
{
    semantic_value & operator=(node * n)
    {
        reset(n);
        return *this;
    }

    template <typename T>
    T * as()
    {
        return static_cast<T*>( get() );
    }
};

enum node_type
{
    kwd_let,
    kwd_for,
    kwd_reduce,

    add,
    subtract,
    multiply,
    divide,
    raise,
    lesser,
    greater,
    lesser_or_equal,
    greater_or_equal,
    equal,
    not_equal,

    integer_num,
    real_num,
    identifier,
    range,

    //binary_op_expression,
    call_expression,
    transpose_expression,
    slice_expression,
    hash_expression,

    for_expression,
    for_iteration,
    for_iteration_list,
    reduce_expression,

    expression_block,
    statement,

    id_list,
    int_list,
    expression_list,
    statement_list,

    program,

    node_type_count
};

struct list_node;
template <typename T> struct leaf_node;

struct node
{
    node_type type;
    int line;
    semantic::type_ptr semantic_type;

    node( const node & other ):
        type(other.type),
        line(other.line),
        semantic_type(other.semantic_type)
    {}

    node( node_type type, int line = 0 ): type(type), line(line) {}
    virtual ~node() {}

    virtual bool is_list() { return false; }
    virtual bool is_leaf() { return false; }

    virtual node_ptr clone()
    {
        node *n = new node(*this);
        return node_ptr(n);
    }

    inline list_node *as_list();

    template <typename T>
    inline leaf_node<T> *as_leaf();
};

struct list_node : public node
{
    vector<sp<node>> elements;

    list_node( node_type type, int line ):
        node(type, line)
    {}

    list_node( node_type type, int line,
               std::initializer_list<sp<node>> elements ):
        node(type, line),
        elements(elements)
    {}

    list_node( const list_node & other ):
        node(other)
    {
        elements.reserve(other.elements.size());
        for(const node_ptr & e : other.elements)
        {
            if (e)
                elements.push_back(e->clone());
            else
                elements.push_back(node_ptr());
        }
    }

    bool is_list() { return true; }

    virtual node_ptr clone()
    {
        list_node *n = new list_node(*this);
        return node_ptr(n);
    }

    void append( const sp<node> & element )
    {
        elements.push_back(element);
    }

    void append( const vector<sp<node>> & other_elements )
    {
        elements.insert( elements.end(),
                         other_elements.begin(), other_elements.end() );
    }
};

template<typename T>
struct leaf_node : public node
{
    T value;

    leaf_node (node_type type, int line, const T & v):
        node(type, line),
        value(v)
    {}

    leaf_node(const leaf_node<T> & other):
        node(other),
        value(other.value)
    {}

    virtual node_ptr clone()
    {
        leaf_node<T> *n = new leaf_node<T>(*this);
        return node_ptr(n);
    }

    bool is_leaf() { return true; }
};

inline list_node *node::as_list()
{
    assert(is_list());
    return static_cast<list_node*>(this);
}

template <typename T>
inline leaf_node<T> *node::as_leaf()
{
    assert(is_leaf());
    return static_cast<leaf_node<T>*>(this);
}

struct binary_op_expression : list_node
{
    binary_op_expression( const sp<node> & lhs,
                          node_type op,
                          const sp<node> & rhs ):
        list_node( op, lhs->line, { lhs, rhs } )
    {}
};

#if 0
struct statement;

template <typename T>
struct list_node : public node, public vector<T>
{
    using vector<T>::vector;
    list_node() {}
    list_node( int line ): node(line) {}
};

typedef list_node<int> int_list;

struct identifier : public node
{
    using node::node;
    string name;
};

struct expression : public node
{
protected:
    expression() {}
    expression(int line): node(line) {}
};

typedef list_node<sp<expression>> expression_list;

struct range : public node
{
    range() {}
    range( const sp<expression> & start,
           const sp<expression> & end):
        node(start ? start->line : end->line),
        start(start),
        end(end)
    {}
    sp<expression> start;
    sp<expression> end;
};

typedef list_node<range> range_list;

struct call : public expression
{
    identifier id;
    expression_list args;
    int_list dimensions;
    expression_list range;
};

typedef list_node<identifier> id_list;

typedef list_node<statement> statement_list;

struct expression_block : public node
{
    statement_list environment;
    sp<expression> expr;
};

struct statement : public node
{
    identifier id;
    id_list params;
    expression_block body;
};

struct for_iteration : public node
{
    identifier iterator;
    sp<expression> size;
    sp<expression> hop;
    sp<expression> domain;
};

typedef list_node<for_iteration> for_iteration_list;

struct for_expression : public expression
{
    for_iteration_list iterations;
    expression_block body;
};

struct reduce_expression : public expression
{
    identifier id1;
    identifier id2;
    sp<expression> domain;
    expression_block body;
};

enum binary_operator
{
    add,
    subtract,
    multiply,
    divide,
    equals,
    not_equals,
    less_than,
    more_than
};

struct binop_expression : public expression
{
    binop_expression( const sp<expression> & lhs,
                      binary_operator op,
                      const sp<expression> & rhs ):
        expression(lhs->line),
        op(op), lhs(lhs), rhs(rhs)
    {}
    binary_operator op;
    sp<expression> lhs;
    sp<expression> rhs;
};

struct range_expression : public expression
{
    range_expression( const range & r ): expression(r.line), r(r) {}
    range r;
};

enum numerical_type
{
    int_literal,
    real_literal
};

struct numerical_expression : public expression
{
    numerical_expression( int value, int line = 0 ):
        expression(line),
        type(int_literal),
        i(value)
    {}

    numerical_expression( double value, int line = 0 ):
        expression(line),
        type(real_literal),
        d(value)
    {}

    numerical_type type;
    union {
        int i;
        double d;
    };
};

struct hash_expression : public expression
{
    hash_expression() {}
    hash_expression( const identifier & id, const sp<expression> & dim = nullptr ):
        expression(id.line),
        id(id),
        dim(dim)
    {}
    identifier id;
    sp<expression> dim;
};

struct program : public node
{
    statement_list statements;
};

#endif

}

}

#endif // STREAM_LANG_AST_INCLUDED
