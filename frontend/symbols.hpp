#ifndef STREAM_LANG_SYMBOLIC_INCLUDED
#define STREAM_LANG_SYMBOLIC_INCLUDED

#include "ast.hpp"
//#include "types.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace stream {
namespace semantic {

using std::string;
using std::unordered_map;
using std::vector;
using std::deque;
using std::ostream;
template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using up = std::unique_ptr<T>;

struct type
{
    enum tag
    {
        integer_num,
        real_num,
        range,
        stream
        //function
    };

    type(tag t): m_tag(t) {}
    virtual ~type() {}

    tag get_tag() const { return m_tag; }

    virtual void print_on( ostream & s ) const
    {
        s << "<tag = " << m_tag << ">";
    }

private:
    tag m_tag;
};

inline ostream & operator<<( ostream & s, const type & t )
{
    t.print_on(s);
    return s;
}

template <type::tag TAG>
class tagged_type : public type
{
public:
    tagged_type(): type(TAG)
    {}
};

template<typename T>
struct scalar
{
    scalar():
        m_is_constant(false)
    {}

    scalar( const T & value ):
        m_is_constant(true),
        m_value(value)
    {}

    bool is_constant() const { return m_is_constant; }

    T constant_value() const
    {
        if (!m_is_constant)
            throw std::runtime_error("Not a constant");
        return m_value;
    }

    void set_constant( const T & value )
    {
        m_value = value;
        m_is_constant = true;
    }

private:
    bool m_is_constant;
    T m_value;
};

struct integer_num : public scalar<int>, public tagged_type<type::integer_num>
{
    integer_num() {}
    integer_num(int v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<int: " << constant_value() << ">";
        else
            s << "<int>";
    }
};

struct real_num : public scalar<double>, public tagged_type<type::real_num>
{
    real_num() {}
    real_num(double v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<real: " << constant_value() << ">";
        else
            s << "<real>";
    }
};

struct range : public tagged_type<type::range>
{
    range() {}

    sp<type> start;
    sp<type> end;

    virtual void print_on( ostream & s ) const
    {
        s << "<range: ";
        if (start)
            s << *start;
        s << "...";
        if (end)
            s << *end;
        s << ">";
    }
};

struct stream : public tagged_type<type::stream>
{
    stream( const vector<int> & s ) : size(s) {}
    int dimensionality() const { return size.size(); }
    vector<int> size;


    virtual void print_on( ostream & s ) const
    {
        s << "<stream: [";
        for (int i : size)
        {
            s << i;
            if (i != size.back())
                s << ",";
        }
        s << "]>";
    }
};

#if 0
struct function : public tagged_type<type::function>
{
    sp<ast::node> body;
    string name;
    vector<string> parameters;

    function( const sp<ast::node> & def );
};
#endif

struct environment_item;

struct environment;

// TODO:
// Have one single environment item that optionally has parameters.
// It is created with an unevaluated reference to ast::node.
// It is evaluated upon request.
// If it has no parameters, the first value is cached.
// Thus we can use evaluate_stmt_list for top-level environment creation.

class environment_item
{
public:
    environment_item( const string & name, const vector<string> & params,
                      const sp<ast::node> & code ):
        m_name(name),
        m_parameters(params),
        m_code(code)
    {}

    environment_item( const sp<type> & val ):
        m_value(val)
    {}

    const string & name() { return m_name; }
    const vector<string> & parameters() { return m_parameters; }
    sp<type> evaluate( environment & envir, const vector<sp<type>> & = vector<sp<type>>() );

private:
    string m_name;
    vector<string> m_parameters;
    sp<ast::node> m_code;
    sp<type> m_value;
};

#if 0
struct environment_item
{
    virtual ~environment_item(){}
    virtual sp<type> evaluate_type( environment & envir, const vector<sp<type>> & ) = 0;
    //virtual vector<string> parameters() = 0;
};

struct simple_environment_item : public environment_item
{
    sp<type> data;

    simple_environment_item( const sp<type> & d): data(d) {}

#if 0
    vector<string> parameters()
    {
        return vector<string>();
    }
#endif
    sp<type> evaluate_type( environment &, const vector<sp<type>> &)
    {
        return data;
    }
};

struct func_environment_item : public environment_item
{
    sp<function> func;

    func_environment_item( const sp<function> & f ): func(f) {}
#if 0
    vector<string> parameters()
    {
        return func->parameters;
    }
#endif
    sp<type> evaluate_type( environment & envir, const vector<sp<type>> & );
};
#endif

typedef std::unordered_map<string, up<environment_item>> symbol_map;

class environment : private deque<symbol_map>
{
public:
    environment()
    {
        enter_scope();
    }

    void bind( const string & symbol, environment_item * item )
    {
        back().emplace( symbol, up<environment_item>(item) );
    }

    void bind( const string & symbol, const sp<type> & val )
    {
        back().emplace(symbol, up<environment_item>(new environment_item(val)));
    }

#if 0
    void bind( const string & symbol, const sp<function> & f )
    {
        back().emplace(symbol, up<environment_item>(new func_environment_item(f)));
    }
#endif
    void enter_scope()
    {
        emplace_back();
    }

    void exit_scope()
    {
        pop_back();
    }

    environment_item * operator[]( const string & key )
    {
        reverse_iterator it;
        for (it = rbegin(); it != rend(); ++it)
        {
            auto entry = it->find(key);
            if (entry != it->end())
              return entry->second.get();
        }

        return nullptr;
    }
};



environment top_environment( ast::node * program );

//sp<type> evaluate( environment & env, sp<function> & f, const vector<sp<type>> & a );
sp<type> evaluate_expr_block( environment & env, const sp<ast::node> & root );
void evaluate_stmt_list( environment & env, const sp<ast::node> & root );
environment_item * evaluate_statement( environment & env, const sp<ast::node> & root );
sp<type> evaluate_expression( environment & env, const sp<ast::node> & root );
sp<type> evaluate_binop( environment & env, const sp<ast::node> & root );
sp<type> evaluate_range( environment & env, const sp<ast::node> & root );
sp<type> evaluate_hash( environment & env, const sp<ast::node> & root );
sp<type> evaluate_call( environment & env, const sp<ast::node> & root );

struct semantic_error : public std::runtime_error
{
    semantic_error(const string & what, int line = 0):
        runtime_error(what),
        m_line(line)
    {}
    int line() const { return m_line; }
    void report();
private:
    int m_line;
};

} // namespace semantic
} // namespace stream

#endif //  STREAM_LANG_SYMBOLIC_INCLUDED
