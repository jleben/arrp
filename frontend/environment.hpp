#ifndef STREAM_LANG_ENVIRONMENT_INCLUDED
#define STREAM_LANG_ENVIRONMENT_INCLUDED

#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cassert>

namespace stream {
namespace semantic {

using std::string;
using std::vector;
using std::unordered_map;

struct symbol
{
    enum symbol_type
    {
        expression,
        function,
        builtin_unary_math,
        builtin_binary_math
    };

    //symbol() {}

    symbol(symbol_type t, string name, const ast::node_ptr & src = ast::node_ptr()):
        type(t),
        name(name),
        source(src)
    {}

    symbol_type type;
    string name;
    vector<string> parameter_names;
    ast::node_ptr source;
};

struct environment : public unordered_map<string, symbol>
{
    environment();
};

std::ostream & operator<<(std::ostream & s, const environment & env);

class environment_builder
{
public:
    environment_builder(environment &env):
        m_env(env),
        m_has_error(false)
    {}

    bool process( const ast::node_ptr & source );

private:
    struct dummy {};
    typedef context<string,dummy> context_type;

    void process_stmt_list( const ast::node_ptr & );
    void process_stmt( const sp<ast::node> & );
    void process_block( const sp<ast::node> & );
    void process_expr( const sp<ast::node> & );
    environment & m_env;
    context<string,dummy> m_ctx;

    bool can_bind( const string & name )
    {
        if (m_ctx.level() > 0)
            return !m_ctx.current_scope_has_bound(name);
        else
            return m_env.find(name) == m_env.end();
    }

    bool is_bound( const string & name )
    {
        bool bound = m_ctx.has_bound(name);
        if (!bound)
            bound = m_env.find(name) != m_env.end();
        return bound;
    }

    void report( const std::exception & e )
    {
        m_has_error = true;
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    bool m_has_error;
};

}
}

#endif // STREAM_LANG_ENVIRONMENT_INCLUDED
