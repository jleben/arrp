#include "llvm_ir_from_cloog.hpp"

#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/Intrinsics.h>

#include <iostream>

using namespace std;

namespace stream {
namespace polyhedral {

llvm_from_cloog::llvm_from_cloog(const string & module_name):
    m_module(module_name, llvm::getGlobalContext()),
    m_builder(llvm::getGlobalContext())
{
}

void llvm_from_cloog::generate
( clast_stmt *root )
{
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
        process(root);
    }
    catch (std::exception & e)
    {
        cout << "ERROR: " << e.what() << endl;
        return;
    }

    m_builder.CreateRetVoid();
}

void llvm_from_cloog::process_list(clast_stmt *stmt )
{
    while(stmt)
    {
        process(stmt);
        stmt = stmt->next;
    }
}

void llvm_from_cloog::process( clast_stmt *stmt )
{
    if (CLAST_STMT_IS_A(stmt, stmt_root))
    {
        process( reinterpret_cast<clast_root*>(stmt) );
    }
    else if (CLAST_STMT_IS_A(stmt, stmt_block))
    {
        process( reinterpret_cast<clast_block*>(stmt) );
    }
    else if (CLAST_STMT_IS_A(stmt, stmt_ass))
    {
        process( reinterpret_cast<clast_assignment*>(stmt) );
    }
    else if (CLAST_STMT_IS_A(stmt, stmt_guard))
    {
        process( reinterpret_cast<clast_guard*>(stmt) );
    }
    else if (CLAST_STMT_IS_A(stmt, stmt_for))
    {
        process( reinterpret_cast<clast_for*>(stmt) );
    }
    else if (CLAST_STMT_IS_A(stmt, stmt_user))
    {
        process( reinterpret_cast<clast_user_stmt*>(stmt) );
    }
}

void llvm_from_cloog::process( clast_root* root )
{
    process_list(root->stmt.next);
}

void llvm_from_cloog::process( clast_block* block )
{
    process_list(block->body);
}

void llvm_from_cloog::process( clast_assignment* asgn )
{
    value_type val = process(asgn->RHS);
    m_ctx.bind(asgn->LHS, val);
}

void llvm_from_cloog::process( clast_guard* guard )
{
    //block_type body_block = add_block("if.body");
    block_type after_block = add_block("if.after");

    for(int equation_idx = 0; equation_idx < guard->n; ++equation_idx)
    {
        clast_equation *eq = guard->eq + equation_idx;
        block_type true_block = add_block("if.cond");
        block_type false_block = after_block;

        process(eq, true_block, false_block);

        m_builder.SetInsertPoint(true_block);
    }

    process(guard->then);

    m_builder.CreateBr(after_block);

    m_builder.SetInsertPoint(after_block);
}

void llvm_from_cloog::process(clast_equation* eq,
                              block_type true_block,
                              block_type false_block )
{
    value_type lhs = process(eq->LHS);
    value_type rhs = process(eq->RHS);
    value_type result;
    if (eq->sign < 0)
        result = m_builder.CreateICmpSLE(lhs, rhs);
    else if (eq->sign == 0)
        result = m_builder.CreateICmpEQ(lhs, rhs);
    else if (eq->sign > 0)
        result = m_builder.CreateICmpSGE(lhs, rhs);
    m_builder.CreateCondBr(result, true_block, false_block);
}

void llvm_from_cloog::process( clast_for* loop )
{
    const char *iter_name = loop->iterator;

    auto before_block = m_builder.GetInsertBlock();
    auto cond_block = add_block("for.cond");
    auto body_block = add_block("for.body");
    auto end_block = add_block("for.end");

    auto init_val = process(loop->LB);
    auto step_val = value(loop->stride);

    //context::scope_holder for_init_scope(m_ctx);
    //m_ctx.bind(iter_name, init_val);

    m_builder.CreateBr(cond_block);

    // Condition

    m_builder.SetInsertPoint(cond_block);

    auto iter_val = m_builder.CreatePHI(int64_type(), 2);
    iter_val->addIncoming(init_val, before_block);

    context::scope_holder for_body_scope(m_ctx);
    m_ctx.bind(iter_name, iter_val);

    auto max_val = process(loop->UB);
    auto condition = m_builder.CreateICmpSLE(iter_val, max_val);
    m_builder.CreateCondBr(condition, body_block, end_block);

    // Body

    m_builder.SetInsertPoint(body_block);

    process_list(loop->body);

    auto iter_val_incremented = m_builder.CreateAdd(iter_val, step_val);
    iter_val->addIncoming(iter_val_incremented, m_builder.GetInsertBlock());

    m_builder.CreateBr(cond_block);

    // End

    m_builder.SetInsertPoint(end_block);
}

llvm_from_cloog::value_type
llvm_from_cloog::process( clast_expr* expr )
{
    switch(expr->type)
    {
    case clast_expr_name:
    {
        const char *name = reinterpret_cast<clast_name*>(expr)->name;
        return m_ctx.find(name).value();
    }
    case clast_expr_term:
    {
        auto term = reinterpret_cast<clast_term*>(expr);
        value_type val = value(term->val);
        if (term->var)
        {
            value_type var = process(term->var);
            val = m_builder.CreateMul(val, var);
        }
        return val;
    }
    case clast_expr_bin:
    {
        auto operation = reinterpret_cast<clast_binary*>(expr);
        value_type lhs = process(operation->LHS);
        value_type rhs = value(operation->RHS);
        if (operation->type == clast_bin_div)
        {
            return m_builder.CreateExactSDiv(lhs, rhs);
        }
        lhs = m_builder.CreateSIToFP(lhs, double_type());
        rhs = m_builder.CreateSIToFP(rhs, double_type());
        switch(operation->type)
        {
        case clast_bin_fdiv:
            throw std::runtime_error("Floor-of-division not implemented.");
        case clast_bin_cdiv:
            throw std::runtime_error("Ceiling-of-division not implemented.");
        case clast_bin_div:
            return m_builder.CreateExactSDiv(lhs, rhs);
        case clast_bin_mod:
            return m_builder.CreateSRem(lhs, rhs);
        default:
            throw std::runtime_error("Unexpected binary operation type.");
        }
    }
    case clast_expr_red:
    {
        auto reduction = reinterpret_cast<clast_reduction*>(expr);
        if (reduction->n < 1)
            return value((int64_t) 0);
        value_type lhs = process( reduction->elts[0] );
        for (int i = 1; i < reduction->n; ++i)
        {
            value_type rhs = process( reduction->elts[i] );
            switch(reduction->type)
            {
            case clast_red_sum:
                lhs = m_builder.CreateAdd(lhs, rhs);
                break;
            case clast_red_min:
                lhs = m_builder.CreateSelect( m_builder.CreateICmpSLE(lhs, rhs), lhs, rhs );
                break;
            case clast_red_max:
                lhs = m_builder.CreateSelect( m_builder.CreateICmpSGE(lhs, rhs), lhs, rhs );
                break;
            default:
                throw std::runtime_error("Unexpected reduction type.");
            }
        }
        return lhs;
    }
    default:
        throw std::runtime_error("Unexpected expression type.");
    }
}

void llvm_from_cloog::process( clast_user_stmt* stmt )
{
    vector<value_type> index;

    clast_stmt *index_stmt = stmt->substitutions;
    while(index_stmt)
    {
        assert(CLAST_STMT_IS_A(index_stmt, stmt_ass));
        clast_expr *index_expr =
                reinterpret_cast<clast_assignment*>(index_stmt)->RHS;
        index.push_back( process(index_expr) );

        index_stmt = index_stmt->next;
    }

    if (m_stmt_func)
        m_stmt_func(stmt->statement->name, index, m_builder.GetInsertBlock());
    else
    {
        auto noop_func = llvm::Intrinsic::getDeclaration(&m_module, llvm::Intrinsic::donothing);
        m_builder.CreateCall(noop_func);
    }
}

bool llvm_from_cloog::verify()
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

void llvm_from_cloog::output( std::ostream & out )
{
    llvm::raw_os_ostream llvm_ostream(out);
    m_module.print( llvm_ostream, nullptr );
}

}
}
