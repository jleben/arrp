#ifndef STREAM_LANG_TYPE_CHECKER_INCLUDED
#define STREAM_LANG_TYPE_CHECKER_INCLUDED

#include "ast.hpp"
#include "environment.hpp"
#include "error.hpp"
#include "context.hpp"
#include "types.hpp"

#include <unordered_map>
#include <string>
#include <utility>
#include <exception>

namespace stream {
namespace semantic {

using std::string;
using std::unordered_map;
using std::pair;

class type_checker
{
public:
    type_checker(environment &env );

    type_ptr check( const symbol & sym, const vector<type_ptr> & args );

    bool has_error() { return m_has_error; }

private:
#if 0
    struct context_item
    {
        type_ptr type;
        vector<string> parameters;
        ast::node_ptr expression;
    };
#endif
    using context_type = context<string, type_ptr>;

    struct iterator
    {
        iterator(): hop(1), size(1), count(1) {}
        string id;
        int hop;
        int size;
        int count;
        type_ptr domain;
        type_ptr value;
    };

    type_ptr symbol_type( const symbol & sym, const vector<type_ptr> & args );
    type_ptr symbol_type( const symbol & sym );
    type_ptr builtin_unary_func_type(const type_ptr & arg );
    type_ptr builtin_binary_func_type(const type_ptr & arg1, const type_ptr & arg2 );

    type_ptr process_function( const type_ptr & func,
                               const vector<type_ptr> & args,
                               context_type::scope_iterator scope );
    type_ptr process_block( const sp<ast::node> & );
    void process_stmt_list( const ast::node_ptr & );
    void process_stmt( const sp<ast::node> & );
    type_ptr process_expression( const sp<ast::node> & );
    pair<type_ptr, context_type::scope_iterator>
    process_identifier( const sp<ast::node> & );
    type_ptr process_binop( const sp<ast::node> & );
    type_ptr process_range( const sp<ast::node> & );
    type_ptr process_extent( const sp<ast::node> & );
    type_ptr process_transpose( const sp<ast::node> & );
    type_ptr process_slice( const sp<ast::node> & );
    type_ptr process_call( const sp<ast::node> & );
    type_ptr process_iteration( const sp<ast::node> & );
    iterator process_iterator( const sp<ast::node> & );
    type_ptr process_reduction( const sp<ast::node> & );

    //type_ptr process_context_item( const context_item & item );

    void report( const std::exception & e )
    {
        m_has_error = true;
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    environment & m_env;
    context_type m_ctx;
    context_type::scope_iterator m_root_scope;

    bool m_has_error;
};

}
}

#endif // STREAM_LANG_TYPE_CHECKER_INCLUDED
