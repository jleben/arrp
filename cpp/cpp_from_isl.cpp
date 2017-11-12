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

#include "cpp_from_isl.hpp"
#include "../common/ph_model.hpp"
#include "../common/error.hpp"

#include <iostream>
#include <sstream>
#include <isl/ast.h>

using namespace std;

namespace stream {
namespace cpp_gen {

cpp_from_isl::cpp_from_isl(builder *ctx):
    m_ctx(ctx)
{
}

void cpp_from_isl::generate( isl_ast_node * ast )
{
    m_is_user_stmt = false;
    process_node(ast);
}

void cpp_from_isl::process_node(isl_ast_node *node)
{
    auto type = isl_ast_node_get_type(node);
    switch(type)
    {
    case isl_ast_node_for:
        process_for(node); break;
    case isl_ast_node_if:
        process_if(node); break;
    case isl_ast_node_block:
        process_block(node); break;
    case isl_ast_node_user:
        process_user(node); break;
    case isl_ast_node_mark:
    {
        // TODO: label the stmt?
        auto marked_node = isl_ast_node_mark_get_node(node);
        process_node(marked_node);
        isl_ast_node_free(marked_node);
        break;
    }
    default:
        throw error("Unexpected AST node type.");
    }
}

void cpp_from_isl::process_block(isl_ast_node *node)
{
    // The generate AST is weird:
    // Blocks have at most 2 statements.
    // If there are more consecutive statements,
    // all but the last are grouped into a nested block,
    // and so on recursively.

    //auto block = make_shared<block_statement>();

    //m_ctx->push(&block->statements);

    auto list = isl_ast_node_block_get_children(node);
    int n_children = isl_ast_node_list_n_ast_node(list);

    for(int i = 0; i < n_children; ++i)
    {
        auto child = isl_ast_node_list_get_ast_node(list, i);
        process_node(child);
        isl_ast_node_free(child);
    }

    isl_ast_node_list_free(list);

    //m_ctx->pop();

    //m_ctx->add(block);
}

void cpp_from_isl::process_if(isl_ast_node *node)
{
    auto cond_expr = isl_ast_node_if_get_cond(node);
    auto true_node = isl_ast_node_if_get_then(node);
    auto false_node = isl_ast_node_if_get_else(node);

    auto if_stmt = make_shared<if_statement>();

    if_stmt->condition = process_expr(cond_expr);

    {
        vector<statement_ptr> stmts;

        m_ctx->push(&stmts);
        process_node(true_node);
        m_ctx->pop();

        if (stmts.size() == 1)
            if_stmt->true_part = stmts.front();
        else
            if_stmt->true_part = block(stmts);
    }

    if (false_node)
    {
        vector<statement_ptr> stmts;

        m_ctx->push(&stmts);
        process_node(false_node);
        m_ctx->pop();

        if (stmts.size() == 1)
            if_stmt->false_part = stmts.front();
        else
            if_stmt->false_part = block(stmts);
    }

    m_ctx->add(if_stmt);

    isl_ast_expr_free(cond_expr);
    isl_ast_node_free(true_node);
    isl_ast_node_free(false_node);
}

void cpp_from_isl::process_for(isl_ast_node *node)
{
    auto iter_expr = isl_ast_node_for_get_iterator(node);
    auto init_expr = isl_ast_node_for_get_init(node);
    auto cond_expr = isl_ast_node_for_get_cond(node);
    auto inc_expr = isl_ast_node_for_get_inc(node);
    auto body_node = isl_ast_node_for_get_body(node);

    polyhedral::ast_node_info * info = nullptr;
    {
        auto id = isl_ast_node_get_annotation(node);
        if (id)
        {
            info = polyhedral::ast_node_info::get_from_id(id);
            id = isl_id_free(id);
        }
    }

    auto iter = process_expr(iter_expr);
    auto init = process_expr(init_expr);
    auto cond = process_expr(cond_expr);
    auto inc = process_expr(inc_expr);

    auto iter_id = dynamic_pointer_cast<id_expression>(iter);
    if (!iter_id)
        throw error("Iterator expression is not an identifier.");
    auto iter_decl = decl_expr(make_shared<basic_type>("int"),
                               *iter_id, init);

    auto for_stmt = make_shared<for_statement>();

    for_stmt->initialization = iter_decl;

    for_stmt->condition = cond;

    for_stmt->update = binop(op::assign_add, iter, inc);

    {
        vector<statement_ptr> stmts;

        m_ctx->push(&stmts);
        m_ctx->block(0).induction_var = iter_id->name;

        process_node(body_node);

        m_ctx->pop();

        if (stmts.size() == 1)
            for_stmt->body = stmts.front();
        else
            for_stmt->body = block(stmts);
    }

    if (info)
    {
        for_stmt->is_parallel = info->is_parallel_for;
    }

    m_ctx->add(for_stmt);

    isl_ast_expr_free(iter_expr);
    isl_ast_expr_free(init_expr);
    isl_ast_expr_free(cond_expr);
    isl_ast_expr_free(inc_expr);
    isl_ast_node_free(body_node);
}

void cpp_from_isl::process_user(isl_ast_node *node)
{
    auto ast_expr = isl_ast_node_user_get_expr(node);

    m_is_user_stmt = true;
    auto expr = process_expr(ast_expr);
    m_is_user_stmt = false;

    if (expr)
        m_ctx->add(expr);

    isl_ast_expr_free(ast_expr);
}

expression_ptr cpp_from_isl::process_expr(isl_ast_expr * ast_expr)
{
    expression_ptr expr;

    auto type = isl_ast_expr_get_type(ast_expr);

    switch(type)
    {
    case isl_ast_expr_op:
    {
        expr = process_op(ast_expr);
        break;
    }
    case isl_ast_expr_id:
    {
        auto id = isl_ast_expr_get_id(ast_expr);
        string name(isl_id_get_name(id));
        isl_id_free(id);

        if (m_id_func)
            expr = m_id_func(name);
        if (!expr)
            expr = make_shared<id_expression>(name);
        break;
    }
    case isl_ast_expr_int:
    {
        auto val = isl_ast_expr_get_val(ast_expr);
        if (isl_val_is_int(val) != isl_bool_true)
            throw error("Value is not an integer.");
        int ival = isl_val_get_num_si(val);
        isl_val_free(val);
        expr = literal(ival);
        break;
    }
    default:
        throw error("Unexpected AST expression type.");
    }

    return expr;
}

expression_ptr cpp_from_isl::process_op(isl_ast_expr * ast_op)
{
    int arg_count = isl_ast_expr_get_op_n_arg(ast_op);
    vector<expression_ptr> args;
    args.reserve(arg_count);
    for(int i = 0; i < arg_count; ++i)
    {
        auto ast_arg = isl_ast_expr_get_op_arg(ast_op, i);
        auto arg = process_expr(ast_arg);
        isl_ast_expr_free(ast_arg);
        args.push_back(arg);
    }

    expression_ptr expr;

    auto type = isl_ast_expr_get_op_type(ast_op);

    switch(type)
    {
    case isl_ast_op_and:
        assert_or_throw(args.size() == 2);
        expr = binop(op::logic_and, args[0], args[1]);
        break;
    case isl_ast_op_or:
        assert_or_throw(args.size() == 2);
        expr = binop(op::logic_or, args[0], args[1]);
        break;
    case isl_ast_op_max:
    {
        assert_or_throw(args.size() >= 2);
        expr = make_shared<call_expression>("max", args[0], args[1]);
        for(int i = 2; i < args.size(); ++i)
            expr = make_shared<call_expression>("max", expr, args[i]);
        break;
    }
    case isl_ast_op_min:
    {
        assert_or_throw(args.size() >= 2);
        expr = make_shared<call_expression>("min", args[0], args[1]);
        for(int i = 2; i < args.size(); ++i)
            expr = make_shared<call_expression>("min", expr, args[i]);
        break;
    }
    case isl_ast_op_minus:
        assert_or_throw(args.size() == 1);
        expr = unop(op::u_minus, args[0]);
        break;
    case isl_ast_op_add:
        assert_or_throw(args.size() == 2);
        expr = binop(op::add, args[0], args[1]);
        break;
    case isl_ast_op_sub:
        assert_or_throw(args.size() == 2);
        expr = binop(op::sub, args[0], args[1]);
        break;
    case isl_ast_op_mul:
        assert_or_throw(args.size() == 2);
        expr = binop(op::mult, args[0], args[1]);
        break;
    case isl_ast_op_div:
        assert_or_throw(args.size() == 2);
        expr = binop(op::div, args[0], args[1]);
        break;
    case isl_ast_op_eq:
        assert_or_throw(args.size() == 2);
        expr = binop(op::equal, args[0], args[1]);
        break;
    case isl_ast_op_le:
        assert_or_throw(args.size() == 2);
        expr = binop(op::lesser_or_equal, args[0], args[1]);
        break;
    case isl_ast_op_lt:
        assert_or_throw(args.size() == 2);
        expr = binop(op::lesser, args[0], args[1]);
        break;
    case isl_ast_op_ge:
        assert_or_throw(args.size() == 2);
        expr = binop(op::greater_or_equal, args[0], args[1]);
        break;
    case isl_ast_op_gt:
        assert_or_throw(args.size() == 2);
        expr = binop(op::greater, args[0], args[1]);
        break;
    case isl_ast_op_call:
    {
        auto id = dynamic_pointer_cast<id_expression>(args[0]);
        if (!id)
            throw error("Function identifier expression is not an identifier.");

        vector<expression_ptr> func_args(++args.begin(), args.end());

        if (m_is_user_stmt && m_stmt_func)
            m_stmt_func(id->name, func_args, m_ctx);
        else
            expr = make_shared<call_expression>(id->name, func_args);

        break;
    }
    case isl_ast_op_zdiv_r:
    {
        assert_or_throw(args.size() == 2);
        // "Equal to zero iff the remainder on integer division is zero."
        expr = binop(op::rem, args[0], args[1]);
        break;
    }
    case isl_ast_op_pdiv_r:
    {
        assert_or_throw(args.size() == 2);
        //Remainder of integer division, where dividend is known to be non-negative.
        expr = binop(op::rem, args[0], args[1]);
        break;
    }
    case isl_ast_op_pdiv_q:
    {
        assert_or_throw(args.size() == 2);
        // Result of integer division, where dividend is known to be non-negative.
        expr = binop(op::div, args[0], args[1]);
        break;
    }
    case isl_ast_op_or_else:
        // not implemented
    case isl_ast_op_and_then:
        // not implemented
    case isl_ast_op_fdiv_q:
        // Not implemented
        // Result of integer division, rounded towards negative infinity.
    case isl_ast_op_cond:
        // Not implemented.
    case isl_ast_op_select:
        // Not implemented.
    case isl_ast_op_access:
        // Not implemented
    case isl_ast_op_member:
        // Not implemented
    default:
        throw error("Unsupported AST expression type.");
    }

    return expr;
}

}
}
