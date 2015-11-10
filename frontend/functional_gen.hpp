/*
Compiler for language for stream processing

Copyright (C) 2015  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_FUNCTIONAL_MODEL_GENERATOR_INCLUDED
#define STREAM_FUNCTIONAL_MODEL_GENERATOR_INCLUDED

#include "../common/ast.hpp"
#include "../common/functional_model.hpp"
#include "../utility/context.hpp"

namespace stream {
namespace functional {

class generator
{
public:
    vector<func_def_ptr> generate(ast::node_ptr ast);

private:
    using context_type = context<string,var_ptr>;

    context_type m_context;

private:
    func_def_ptr do_stmt(ast::node_ptr);
    expr_ptr do_block(ast::node_ptr, vector<func_def_ptr> & defs);
    expr_ptr do_expr(ast::node_ptr);
    expr_ptr do_primitive(ast::node_ptr);
    expr_ptr do_array_def(ast::node_ptr);
    expr_ptr do_array_apply(ast::node_ptr);
    expr_ptr do_func_apply(ast::node_ptr);
};

}
}

#endif // STREAM_FUNCTIONAL_MODEL_GENERATOR_INCLUDED
