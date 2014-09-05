#include "llvm_ir_generator.hpp"

namespace stream {
namespace polyhedral {

using value_type = llvm::Value*;
using type_type = llvm::Type*;

llvm_ir_generator::llvm_ir_generator(const string & module_name):
    m_module(module_name, llvm::getGlobalContext()),
    m_builder(llvm::getGlobalContext())
{

}

void llvm_ir_generator::generate
( isl_ast_node *ast, const unordered_map<string, statement*> & source )
{

}

value_type llvm_ir_generator::process_expression(isl_ast_expr* expr)
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
    }
}

value_type llvm_ir_generator::process_op(isl_ast_expr* expr)
{
    isl_ast_op_type op_type = isl_ast_expr_get_op_type(expr);
    int arg_count = isl_ast_expr_get_op_n_arg(expr);
    auto parent_func = m_builder.GetInsertBlock()->getParent();

    switch(op_type)
    {
    case isl_ast_op_cond:
    {
        assert(arg_count == 3);
        auto true_block = llvm::BasicBlock::Create(llvm_context(), "cond.true", parent_func);
        auto false_block = llvm::BasicBlock::Create(llvm_context(), "cond.false", parent_func);
        auto after_block = llvm::BasicBlock::Create(llvm_context(), "cond.after", parent_func);

        auto cond_expr = isl_ast_expr_get_op_arg(expr, 0);
        auto true_expr = isl_ast_expr_get_op_arg(expr, 1);
        auto false_expr = isl_ast_expr_get_op_arg(expr, 2);

        process_conditional(true_expr, true_block, false_block);

        m_builder.SetInsertPoint(true_block);
        auto true_val = process_expression(true_expr);
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(false_block);
        auto false_val = process_expression(false_expr);
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
        auto rhs_block = llvm::BasicBlock::Create(llvm_context(), "cond.other", parent_func);
        auto after_block = llvm::BasicBlock::Create(llvm_context(), "cond.after", parent_func);

        auto lhs_expr = isl_ast_expr_get_op_arg(expr,0);
        auto rhs_expr = isl_ast_expr_get_op_arg(expr,1);

        auto lhs = process_expression(lhs_expr);
        if (op_type == isl_ast_op_and || op_type == isl_ast_op_and_then)
            m_builder.CreateCondBr(lhs, rhs_block, after_block);
        else
            m_builder.CreateCondBr(lhs, after_block, rhs_block);

        m_builder.SetInsertPoint(rhs_block);
        auto rhs = process_expression(rhs_expr);
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

void llvm_ir_generator::process_conditional
(isl_ast_expr * expr, block_type true_block, block_type false_block )
{
    assert(isl_ast_expr_get_type(expr) == isl_ast_expr_op);

    isl_ast_op_type op_type = isl_ast_expr_get_op_type(expr);
    int arg_count = isl_ast_expr_get_op_n_arg(expr);
    assert(arg_count == 2);

    auto parent_func = m_builder.GetInsertBlock()->getParent();

    switch(op_type)
    {
    case isl_ast_op_and:
    case isl_ast_op_and_then:
    {
        assert(arg_count == 2);
        auto rhs_block = llvm::BasicBlock::Create(llvm_context(), "and", parent_func);
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
        auto rhs_block = llvm::BasicBlock::Create(llvm_context(), "or", parent_func);
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

}
}
