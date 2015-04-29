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

#ifndef STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
#define STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED

#include "../common/polyhedral_model.hpp"
#include "../common/ast.hpp"
#include "../common/types.hpp"
#include "../common/environment.hpp"
#include "../utility/context.hpp"
#include "../utility/matrix.hpp"
#include "../utility/debug.hpp"

#include <deque>
#include <vector>
#include <stack>

namespace stream {
namespace polyhedral {

using std::deque;
using std::vector;
using std::stack;
using utility::matrix;

class translator
{
public:
    struct debug : public stream::debug::topic<debug, polyhedral::debug>
    { static string id() { return "model"; } };
    struct debug_transform : public stream::debug::topic
            <debug_transform, debug, stream::debug::disabled>
    { static string id() { return "transform"; } };

    translator(const semantic::environment &);

    void translate(const semantic::symbol &,
                   const vector<semantic::type_ptr> & args);

    vector<statement*> & statements() { return m_statements; }
    vector<array_ptr> & arrays() { return m_arrays; }

private:
    class symbol
    {
    public:
        symbol(expression *expr): source(expr) {}
        expression * source;
    };

    class array_view : public expression
    {
    public:
        array_view(array_ptr target):
            expression(target->type),
            target(target)
        {}
        array_ptr target;
        mapping pattern;
        int current_iteration;
    };

    class range : public expression
    {
    public:
        int start;
        int end;

        range(): expression(primitive_type::integer) {}
    };

    typedef stream::context<string,symbol> context;

    const semantic::environment &m_env;
    context m_context;
    vector<int> m_domain;
    vector<statement*> m_statements;
    vector<array_ptr> m_arrays;

    void do_statement_list(const ast::node_ptr &node);
    expression * do_statement(const ast::node_ptr &node);
    expression * do_block(const ast::node_ptr &node);
    expression * do_expression(const ast::node_ptr &node);
    expression * do_identifier(const ast::node_ptr &node);
    expression * do_call(const ast::node_ptr &node);
    expression * do_unary_op(const ast::node_ptr &node);
    expression * do_binary_op(const ast::node_ptr &node);
    expression * do_transpose(const ast::node_ptr &node);
    expression * do_slicing(const  ast::node_ptr &node);
    expression * do_conditional(const  ast::node_ptr &node);
    expression * do_mapping(const  ast::node_ptr &node);
    expression * do_reduction(const  ast::node_ptr &node);

    int current_dimension() const
    {
        return m_domain.size();
    }

    expression * translate_input(const semantic::type_ptr & type, int index);

    mapping access(array_view *source, int padding = 0);

    expression *iterate (expression *, const semantic::type_ptr & result_type);
    iterator_access * iterate( range * );
    array_access * complete_access( array_view *, const semantic::type_ptr & result_type );

    array_ptr make_array(primitive_type);

    statement * make_statement();
    statement * make_statement( expression *, const vector<int> & domain );
    array_view * make_current_view( array_ptr );
    array_view * make_current_view( statement * );

    expression * update_accesses(expression *, const mapping & map);
};

}
}

#endif // STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
