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

#include "llvm_ir_from_isl.hpp"
#include LLVM_VERIFIER_HEADER
#include <llvm/Support/raw_os_ostream.h>

#include <iostream>

using namespace std;

namespace stream {
namespace llvm_gen {

using value_type = llvm::Value*;
using type_type = llvm::Type*;

llvm_from_isl::llvm_from_isl(const string & module_name):
    m_module(module_name, llvm::getGlobalContext()),
    m_builder(llvm::getGlobalContext())
{

}

void llvm_from_isl::generate
( isl_ast_node *ast, const unordered_map<string, statement*> & source )
{
    m_statements = &source;

    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    false);


    llvm::Function *func =
            llvm::Function::Create(func_type,
                                   llvm::Function::ExternalLinkage,
                                   "process",
                                   &m_module);

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm_context(), "", func);
    m_builder.SetInsertPoint(bb);

    try
    {
        process_node(ast);
    }
    catch (std::exception & e)
    {
        m_statements = nullptr;
        cout << "ERROR: " << e.what() << endl;
        return;
    }

    m_builder.CreateRetVoid();
}

void llvm_from_isl::process_node(isl_ast_node * node)
{
    isl_ast_node_type node_type = isl_ast_node_get_type(node);

    switch(node_type)
    {
    case isl_ast_node_block:
        process_block(node);
        break;
    case isl_ast_node_if:
        process_if(node);
        break;
    case isl_ast_node_for:
        process_for(node);
        break;
    case isl_ast_node_user:
        process_statement(node);
        break;
    default:
        assert(false);
    }
}

int llvm_from_isl::process_block_element(isl_ast_node * node, void * data)
{
    llvm_from_isl *generator = (llvm_from_isl*) data;
    generator->process_node(node);
    return 0;
}

void llvm_from_isl::process_block(isl_ast_node* node)
{
    isl_ast_node_list *children = isl_ast_node_block_get_children(node);
    isl_ast_node_list_foreach(children,
                              &llvm_from_isl::process_block_element, this);
}

void llvm_from_isl::process_if(isl_ast_node* node)
{
    bool has_else = isl_ast_node_if_has_else(node);
    auto cond_expr = isl_ast_node_if_get_cond(node);

    block_type true_block = add_block("if.true");
    block_type false_block = has_else ? add_block("if.false") : nullptr;
    block_type after_block = add_block("if.after");

    if (false_block)
        process_conditional(cond_expr, true_block, false_block);
    else
        process_conditional(cond_expr, true_block, after_block);

    m_builder.SetInsertPoint(true_block);
    auto then_node = isl_ast_node_if_get_then(node);
    process_node(then_node);
    m_builder.CreateBr(after_block);

    if (false_block)
    {
        m_builder.SetInsertPoint(false_block);
        auto else_node = isl_ast_node_if_get_else(node);
        process_node(else_node);
        m_builder.CreateBr(after_block);
    }

    m_builder.SetInsertPoint(after_block);
}

void llvm_from_isl::process_for(isl_ast_node* node)
{
    //isl_ast_node_for_is_degenerate

    auto iterator = isl_ast_node_for_get_iterator(node);
    auto init_expr = isl_ast_node_for_get_init(node);
    auto cond_expr = isl_ast_node_for_get_cond(node);
    auto inc_expr = isl_ast_node_for_get_inc(node);
    auto body_node = isl_ast_node_for_get_body(node);

    assert(isl_ast_expr_get_type(iterator) == isl_ast_expr_id);
    isl_id *iter_id = isl_ast_expr_get_id(iterator);
    const char *iter_name = isl_id_get_name(iter_id);

    auto before_block = m_builder.GetInsertBlock();
    auto cond_block = add_block("for.cond");
    auto body_block = add_block("for.body");
    auto end_block = add_block("for.end");

    auto init_val = process_expression( init_expr );

    context::scope_holder for_init_scope(m_ctx);
    m_ctx.bind(iter_name, init_val);

    m_builder.CreateBr(cond_block);

    // Condition

    m_builder.SetInsertPoint(cond_block);

    auto iter_val = m_builder.CreatePHI(int32_type(), 2);
    iter_val->addIncoming(init_val, before_block);

    context::scope_holder for_body_scope(m_ctx);
    m_ctx.bind(iter_name, iter_val);

    process_conditional(cond_expr, body_block, end_block);

    // Body

    m_builder.SetInsertPoint(body_block);

    process_node(body_node);

    auto iter_val_incremented = process_expression(inc_expr);
    iter_val->addIncoming(iter_val_incremented, m_builder.GetInsertBlock());

    m_builder.CreateBr(cond_block);

    // End

    m_builder.SetInsertPoint(end_block);
}

value_type llvm_from_isl::process_expression(isl_ast_expr* expr)
{
    isl_ast_expr_type expr_type = isl_ast_expr_get_type(expr);
    switch(expr_type)
    {
    case isl_ast_expr_id:
    {
        isl_id *id = isl_ast_expr_get_id(expr);
        const char *name = isl_id_get_name(id);
        return m_ctx.find(name).value();
    }
    case isl_ast_expr_int:
    {
        isl_val *val = isl_ast_expr_get_val(expr);
        int int_val = isl_val_get_num_si(val);
        return int32_value(int_val);
    }
    case isl_ast_expr_op:
    {
        return process_op(expr);
    }
    default:
        throw std::runtime_error("Unexpected expression type.");
    }
}

value_type llvm_from_isl::process_op(isl_ast_expr* expr)
{
    isl_ast_op_type op_type = isl_ast_expr_get_op_type(expr);
    int arg_count = isl_ast_expr_get_op_n_arg(expr);

    switch(op_type)
    {
    case isl_ast_op_cond:
    {
        assert(arg_count == 3);
        auto true_block = add_block("cond.true");
        auto false_block = add_block("cond.false");
        auto after_block = add_block("cond.after");

        auto cond_expr = isl_ast_expr_get_op_arg(expr, 0);
        auto true_expr = isl_ast_expr_get_op_arg(expr, 1);
        auto false_expr = isl_ast_expr_get_op_arg(expr, 2);

        process_conditional(true_expr, true_block, false_block);

        m_builder.SetInsertPoint(true_block);
        auto true_val = process_expression(true_expr);
        true_block = m_builder.GetInsertBlock();
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(false_block);
        auto false_val = process_expression(false_expr);
        false_block = m_builder.GetInsertBlock();
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(after_block);
        llvm::PHINode *phi = m_builder.CreatePHI(int32_type(), 2);
        phi->addIncoming(true_val, true_block);
        phi->addIncoming(false_val, false_block);

        return phi;
    }
    case isl_ast_op_and:
    case isl_ast_op_and_then:
    case isl_ast_op_or:
    case isl_ast_op_or_else:
    {
        assert(arg_count == 2);
        auto before_block = m_builder.GetInsertBlock();
        auto rhs_block = add_block("boolean_op.other");
        auto after_block = add_block("boolean_op.after");

        auto lhs_expr = isl_ast_expr_get_op_arg(expr,0);
        auto rhs_expr = isl_ast_expr_get_op_arg(expr,1);

        auto lhs = process_expression(lhs_expr);
        if (op_type == isl_ast_op_and || op_type == isl_ast_op_and_then)
            m_builder.CreateCondBr(lhs, rhs_block, after_block);
        else
            m_builder.CreateCondBr(lhs, after_block, rhs_block);
        before_block = m_builder.GetInsertBlock();

        m_builder.SetInsertPoint(rhs_block);
        auto rhs = process_expression(rhs_expr);
        rhs_block = m_builder.GetInsertBlock();
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(after_block);
        llvm::PHINode *phi = m_builder.CreatePHI(int32_type(), 2);
        phi->addIncoming(lhs, before_block);
        phi->addIncoming(rhs, rhs_block);

        return phi;
    }
    }

    vector<value_type> args;
    args.reserve(arg_count);
    for (int arg_idx = 0; arg_idx < arg_count; ++arg_idx)
    {
        isl_ast_expr *arg_expr = isl_ast_expr_get_op_arg(expr, arg_idx);
        value_type arg_val = process_expression(arg_expr);
        args.push_back(arg_val);
    }

    switch(op_type)
    {
    case isl_ast_op_minus:
        assert(arg_count == 1);
        return m_builder.CreateNeg(args[0]);
    case isl_ast_op_add:
        assert(arg_count == 2);
        return m_builder.CreateAdd(args[0], args[1]);
    case isl_ast_op_sub:
        assert(arg_count == 2);
        return m_builder.CreateSub(args[0], args[1]);
    case isl_ast_op_mul:
        assert(arg_count == 2);
        return m_builder.CreateMul(args[0], args[1]);
    case isl_ast_op_div:
        assert(arg_count == 2);
        return m_builder.CreateExactSDiv(args[0], args[1]);
    case isl_ast_op_pdiv_q:
        assert(arg_count == 2);
        return m_builder.CreateSDiv(args[0], args[1]);
    case isl_ast_op_fdiv_q:
        // FIXME:
        throw std::runtime_error("Division rounded towards negative infinity"
                                 " not implemented.");
    case isl_ast_op_pdiv_r:
        assert(arg_count == 2);
        return m_builder.CreateSRem(args[0], args[1]);

    case isl_ast_op_max:
    {
        assert(arg_count == 2);
        value_type cond = m_builder.CreateICmpSGT(args[0], args[1]);
        return m_builder.CreateSelect(cond, args[0], args[1]);
    }
    case isl_ast_op_min:
    {
        assert(arg_count == 2);
        value_type cond = m_builder.CreateICmpSLT(args[0], args[1]);
        return m_builder.CreateSelect(cond, args[0], args[1]);
    }
    case isl_ast_op_select:
        assert(arg_count == 3);
        return m_builder.CreateSelect(args[0], args[1], args[2]);
    case isl_ast_op_eq:
        assert(arg_count == 2);
        return m_builder.CreateICmpEQ(args[0], args[1]);
    case isl_ast_op_le:
        assert(arg_count == 2);
        return m_builder.CreateICmpSLE(args[0], args[1]);
    case isl_ast_op_lt:
        assert(arg_count == 2);
        return m_builder.CreateICmpSLT(args[0], args[1]);
    case isl_ast_op_ge:
        assert(arg_count == 2);
        return m_builder.CreateICmpSGE(args[0], args[1]);
    case isl_ast_op_gt:
        assert(arg_count == 2);
        return m_builder.CreateICmpSGT(args[0], args[1]);

    default:
        assert(false);
    }

    return nullptr;
}

void llvm_from_isl::process_conditional
(isl_ast_expr * expr, block_type true_block, block_type false_block )
{
    assert(isl_ast_expr_get_type(expr) == isl_ast_expr_op);

    isl_ast_op_type op_type = isl_ast_expr_get_op_type(expr);
    int arg_count = isl_ast_expr_get_op_n_arg(expr);
    assert(arg_count == 2);

    switch(op_type)
    {
    case isl_ast_op_and:
    case isl_ast_op_and_then:
    {
        assert(arg_count == 2);
        auto rhs_block = add_block("and.other");
        auto lhs_expr = isl_ast_expr_get_op_arg(expr,0);
        auto rhs_expr = isl_ast_expr_get_op_arg(expr,1);
        process_conditional(lhs_expr, rhs_block, false_block);
        m_builder.SetInsertPoint(rhs_block);
        process_conditional(rhs_expr, true_block, false_block);
        return;
    }
    case isl_ast_op_or:
    case isl_ast_op_or_else:
    {
        assert(arg_count == 2);
        auto rhs_block = add_block("or.other");
        auto lhs_expr = isl_ast_expr_get_op_arg(expr,0);
        auto rhs_expr = isl_ast_expr_get_op_arg(expr,1);
        process_conditional(lhs_expr, true_block, rhs_block);
        m_builder.SetInsertPoint(rhs_block);
        process_conditional(rhs_expr, true_block, false_block);
        return;
    }
    default:
        auto cond = process_expression(expr);
        m_builder.CreateCondBr(cond, true_block, false_block);
    }
}

void llvm_from_isl::process_statement(isl_ast_node* node)
{
    auto expr = isl_ast_node_user_get_expr(node);
    assert(isl_ast_expr_get_type(expr) == isl_ast_expr_op);
    assert(isl_ast_expr_get_op_type(expr) == isl_ast_op_call);

    int arg_count = isl_ast_expr_get_op_n_arg(expr);
    assert(arg_count >= 1);

    auto stmt_id_expr = isl_ast_expr_get_op_arg(expr, 0);
    assert(isl_ast_expr_get_type(stmt_id_expr) == isl_ast_expr_id);
    isl_id *stmt_id = isl_ast_expr_get_id(stmt_id_expr);
    const char *stmt_name = isl_id_get_name(stmt_id);

    vector<value_type> index;
    if (arg_count > 1)
    {
        index.reserve(arg_count - 1);
        for (int arg_idx = 1; arg_idx < arg_count; ++arg_idx)
        {
            auto arg_expr = isl_ast_expr_get_op_arg(expr, arg_idx);
            value_type index_val = process_expression(arg_expr);
            index.push_back(index_val);
        }
    }

    auto stmt_info = m_statements->find(stmt_name);
    assert(stmt_info != m_statements->end());

    m_builder.CreateAlloca(double_type(), nullptr, stmt_info->first);
}

bool llvm_from_isl::verify()
{
    string verifier_msg;
    bool failure = llvm::verifyModule(m_module, llvm::ReturnStatusAction,
                                      &verifier_msg);
    if (failure)
    {
        cerr << "ERROR: Failed to verify generated IR code:" << endl;
        cerr << verifier_msg;
    }
    return !failure;
}

void llvm_from_isl::output( std::ostream & out )
{
    llvm::raw_os_ostream llvm_ostream(out);
    m_module.print( llvm_ostream, nullptr );
}


}
}
