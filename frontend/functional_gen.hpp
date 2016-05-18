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
#include "../common/module.hpp"
#include "../common/primitives.hpp"
#include "../common/functional_model.hpp"
#include "../utility/context.hpp"
#include "../utility/stacker.hpp"
#include "error.hpp"

#include <stack>
#include <unordered_map>
#include <string>

namespace stream {
namespace functional {

using std::string;
using std::stack;
using std::unordered_map;

class generator
{
public:
    vector<id_ptr> generate(const vector<module*> modules);
    vector<id_ptr> generate(module*);

private:
    using context_type = context<string,var_ptr>;
    context_type m_context;

private:
    id_ptr do_stmt(ast::node_ptr);
    expr_ptr do_block(ast::node_ptr);
    expr_ptr do_expr(ast::node_ptr);
    expr_ptr do_primitive(ast::node_ptr);
    expr_ptr do_case_expr(ast::node_ptr);
    expr_ptr do_array_def(ast::node_ptr);
    expr_ptr do_array_apply(ast::node_ptr);
    expr_ptr do_array_enum(ast::node_ptr);
    expr_ptr do_array_concat(ast::node_ptr);
    expr_ptr do_func_apply(ast::node_ptr);

    string qualified_name(const string & name);

    code_location location_in_module(const parsing::location & ploc)
    {
        code_location cloc;
        cloc.module = m_current_module;
        cloc.range.start.line = ploc.begin.line;
        cloc.range.start.column = ploc.begin.column;
        cloc.range.end.line = ploc.end.line;
        cloc.range.end.column = ploc.end.column;
        return cloc;
    }

    source_error module_error(const string & what, const parsing::location & loc);

    static unordered_map<string, primitive_op> m_prim_ops;

    unordered_map<string, id_ptr> m_final_ids;

    module * m_current_module = nullptr;

    stack<array_ptr> m_array_stack;
    stack<scope*> m_func_scope_stack;
    typedef stack_adapter<deque<string>> name_stack_t;
    name_stack_t m_name_stack;
};

}
}

#endif // STREAM_FUNCTIONAL_MODEL_GENERATOR_INCLUDED
