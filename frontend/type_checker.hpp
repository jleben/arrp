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
#include <sstream>

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

    type_ptr symbol_type( const symbol & sym, const vector<type_ptr> & args );
    type_ptr symbol_type( const symbol & sym );

    pair<type_ptr, func_type_ptr>
    process_function( const func_type_ptr & func,
                      const vector<type_ptr> & args,
                      context_type::scope_iterator scope );
    type_ptr process_block( const sp<ast::node> & );
    void process_stmt_list( const ast::node_ptr & );
    void process_stmt( const sp<ast::node> &, const ast::node_ptr & list );
    type_ptr process_expression( const sp<ast::node> & );
    pair<type_ptr, context_type::scope_iterator>
    process_identifier( const sp<ast::node> & );
    type_ptr process_negate( const sp<ast::node> & );
    type_ptr process_binop( const sp<ast::node> & );
    type_ptr process_range( const sp<ast::node> & );
    type_ptr process_extent( const sp<ast::node> & );
    type_ptr process_transpose( const sp<ast::node> & );
    type_ptr process_slice( const sp<ast::node> & );
    type_ptr process_call( const sp<ast::node> & );
    type_ptr process_conditional( const ast::node_ptr & );
    type_ptr process_iteration( const sp<ast::node> & );
    type_ptr process_iterator( const sp<ast::node> & );
    type_ptr process_reduction( const sp<ast::node> & );

    pair<type_ptr, function_signature>
    process_primitive( const builtin_function_group &,
                       const vector<type_ptr> & args  );

    type_ptr constant_for( const builtin_function & func,
                           const vector<type_ptr> & args );

    string generate_func_name( string name )
    {
        std::ostringstream symbol;
        symbol << name << ":" << m_func_counter;
        ++m_func_counter;
        return symbol.str();
    }

    void report( const std::exception & e )
    {
        m_has_error = true;
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    environment & m_env;
    context_type m_ctx;

    int m_func_counter;
    bool m_has_error;
};

}
}

#endif // STREAM_LANG_TYPE_CHECKER_INCLUDED
