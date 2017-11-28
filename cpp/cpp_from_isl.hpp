/*
Compiler for language for stream processing

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_LANG_CPP_FROM_ISL_INCLUDED
#define STREAM_LANG_CPP_FROM_ISL_INCLUDED

#include "../utility/context.hpp"
#include "../utility/cpp-gen.hpp"

#include <isl/ast.h>

#include <functional>

namespace stream {
namespace cpp_gen {

class cpp_from_isl
{
public:
    cpp_from_isl(builder *ctx);

    void generate( isl_ast_node *ast );

    template<typename F>
    void set_stmt_func(F f)
    {
        m_stmt_func = f;
    }

    template<typename F>
    void set_id_func(F f)
    {
        m_id_func = f;
    }

private:
    void process_node(isl_ast_node *node);
    void process_block(isl_ast_node *node);
    void process_for(isl_ast_node *node);
    void process_if(isl_ast_node *node);
    void process_user(isl_ast_node *node);

    expression_ptr process_expr(isl_ast_expr * expr);
    expression_ptr process_op(isl_ast_expr * expr);

    std::function<void(const string &,
                       const vector<expression_ptr> &,
                       builder *)>
    m_stmt_func;

    std::function<expression_ptr(const string &)>
    m_id_func;

    bool m_is_user_stmt = false;
    builder *m_ctx;
};


}
}

#endif // STREAM_LANG_CPP_FROM_ISL_INCLUDED
