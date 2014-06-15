#ifndef STREAM_LANG_AST_INCLUDED
#define STREAM_LANG_AST_INCLUDED

#include <vector>
#include <memory>
#include <string>

namespace stream
{

using std::vector;
using std::string;
template <typename T> using sp = std::shared_ptr<T>;

template <typename T>
class visitable : public T
{
    template <typename V>
    void accept( const V & visitor )
    {
        visitor.visit( *static_cast<T*>(this) );
    }
};


namespace ast {

struct node
{
    node(): line(0) {}
    node( int line ): line(line) {}
    int line;
};

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

}

}

#endif // STREAM_LANG_AST_INCLUDED
