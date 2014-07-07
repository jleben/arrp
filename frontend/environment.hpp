#ifndef STREAM_LANG_ENVIRONMENT_INCLUDED
#define STREAM_LANG_ENVIRONMENT_INCLUDED

#include "types.hpp"
#include "ast.hpp"

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <memory>

namespace stream {
namespace semantic {

using std::string;
using std::vector;
using std::deque;
using std::unordered_map;
template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using up = std::unique_ptr<T>;

struct environment;

class symbol
{
public:
    symbol( const string & name ): m_name(name) {}
    const string & name() const { return m_name; }
    virtual sp<type> evaluate( environment & envir,
                               const vector<sp<type>> & = vector<sp<type>>() ) = 0;
    virtual void print_on( ostream & s ) const { s << m_name; }
private:
    string m_name;
};

class constant_symbol : public symbol
{
public:
    constant_symbol( const string & name, const sp<ast::node> & code ):
        symbol(name),
        m_code(code)
    {}
    constant_symbol( const string & name, const sp<type> & value ):
        symbol(name),
        m_value(value)
    {}
    sp<type> evaluate( environment &, const vector<sp<type>> & );
private:
    sp<ast::node> m_code;
    sp<type> m_value;
};

class function_symbol : public symbol
{
public:
    function_symbol( const string & name,
                     const vector<string> & params,
                     const sp<ast::node> & code ):
        symbol(name),
        m_parameters(params),
        m_code(code)
    {}
    sp<type> evaluate( environment &, const vector<sp<type>> & );
private:
    void print_on( ostream & s ) const
    {
        s << name();
        s << '(';
        if (!m_parameters.empty())
            s << m_parameters.front();
        for (int i = 1; i < m_parameters.size(); ++i)
            s << ", " << m_parameters[i];
        s << ')';
    }
    vector<string> m_parameters;
    sp<ast::node> m_code;
};

class elemwise_func_symbol : public symbol
{
public:
    elemwise_func_symbol( const string & name ):
        symbol(name)
    {}
    sp<type> evaluate( environment &, const vector<sp<type>> & );
};

inline ostream & operator<<(ostream & stream, const symbol & sym)
{
    sym.print_on(stream);
    return stream;
}

typedef std::unordered_map<string, up<symbol>> symbol_map;

class environment : private deque<symbol_map>
{
public:
    environment();

    void bind( symbol * sym )
    {
        back().emplace( sym->name(), up<symbol>(sym) );
    }

    void bind( const string & name, const sp<type> & val )
    {
        back().emplace( name, up<symbol>(new constant_symbol(name, val)) );
    }

    void enter_scope()
    {
        emplace_back();
    }

    void exit_scope()
    {
        pop_back();
    }

    symbol * operator[]( const string & key )
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

environment top_environment( ast::node * program, bool print_symbols );

}
}

#endif // STREAM_LANG_ENVIRONMENT_INCLUDED
