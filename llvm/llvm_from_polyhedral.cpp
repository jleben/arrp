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

#include "llvm_from_polyhedral.hpp"

#include <llvm/IR/Intrinsics.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace llvm_gen {

int volume( vector<int> extent )
{
    if (extent.empty())
        return 0;
    int v = 1;
    for(int e : extent)
        v *= e;
    return v;
}

llvm_from_polyhedral::llvm_from_polyhedral
(llvm::Module *module,
 const vector<statement*> & statements,
 const dataflow::model *dataflow,
 const options &opt):
    m_module(module),
    m_builder(module->getContext()),
    m_statements(statements),
    m_dataflow(dataflow)
{
    // Analyze buffer requirements

    std::vector<int> buffers_on_stack;
    std::vector<int> buffers_in_memory;

    int buf_sizes[buffer_kind_count] = {0,0,0,0};

    type_type buf_elem_types[buffer_kind_count] =
    {
        buffer_elem_type(primitive_type::boolean),
        buffer_elem_type(primitive_type::integer),
        buffer_elem_type(primitive_type::real),
        int64_type(),
    };

    int buf_elem_sizes[buffer_kind_count] =
    {
        buffer_elem_size(primitive_type::boolean),
        buffer_elem_size(primitive_type::integer),
        buffer_elem_size(primitive_type::real),
        8
    };

    int stack_buf_idx = 0;

    for (statement *stmt : statements)
    {
        buffer buf;
        buf.type = stmt->expr->type;
        buf.size = volume(stmt->buffer);
        buf.domain = stmt->buffer;

        const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
        if (actor)
        {
            bool period_overlaps =
                    actor->steady_count % stmt->buffer[actor->flow_dimension] != 0;
            bool init_overlaps =
                    actor->init_count % stmt->buffer[actor->flow_dimension] != 0;

            buf.has_phase = period_overlaps || init_overlaps;
            buf.phase_index = buf_sizes[phase_buffer]++;
            transpose(buf.domain, actor->flow_dimension);
        }
        else
        {
            buf.has_phase = false;
            buf.phase_index = -1;
        }

        m_stmt_buffers.push_back(buf);

        int buf_idx = m_stmt_buffers.size() - 1;
        if (stmt->inter_period_dependency || stmt == statements.back())
            buffers_in_memory.push_back(buf_idx);
        else
            buffers_on_stack.push_back(buf_idx);
    }

    auto buffer_size_is_smaller = [&](int a, int b) -> bool
    { return m_stmt_buffers[a].size < m_stmt_buffers[b].size; };

    std::sort(buffers_on_stack.begin(), buffers_on_stack.end(), buffer_size_is_smaller);

    int stack_size = 0;

    {
        for(int idx = 0; idx < buffers_on_stack.size(); ++idx)
        {
            int buf_idx = buffers_on_stack[idx];
            buffer & b = m_stmt_buffers[buf_idx];
            int buf_size = buffer_elem_size(b.type) * b.size;
            if (stack_size + buf_size < opt.max_stack_size)
            {
                b.on_stack = true;
                b.index = stack_buf_idx++;
                stack_size += buf_size;
            }
            else
            {
                buffers_in_memory.push_back(buf_idx);
            }
        }

        for(int idx = 0; idx < buffers_in_memory.size(); ++idx)
        {
            int buf_idx = buffers_in_memory[idx];
            buffer & b = m_stmt_buffers[buf_idx];
            b.on_stack = false;
            int & buf_size = buf_sizes[buffer_kind_for(b.type)];
            b.index = buf_size;
            buf_size += b.size;
        }
    }

    //

    vector<type_type> buffer_struct_member_types;
    for(int idx = 0; idx < buffer_kind_count; ++idx)
        buffer_struct_member_types.push_back(pointer(buf_elem_types[idx]));

    m_buffer_struct_type =
            llvm::StructType::create(buffer_struct_member_types);

    // Generate buffer allocation function

    {
        type_type buffer_struct_ptr_type =
                llvm::PointerType::get(m_buffer_struct_type, 0);

        llvm::FunctionType * func_type =
                llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                        buffer_struct_ptr_type,
                                        false);

        llvm::Function * func =
                llvm::Function::Create(func_type,
                                       llvm::Function::ExternalLinkage,
                                       "allocate",
                                       m_module);

        value_type buf_struct_ptr = func->arg_begin();

        block_type block
                = llvm::BasicBlock::Create(llvm_context(), "", func);
        m_builder.SetInsertPoint(block);

        for(int buf_idx = 0; buf_idx < buffer_kind_count; ++buf_idx)
        {
            vector<value_type> address = { value((int32_t)0), value((int32_t)buf_idx) };
            value_type buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
            int elem_count = buf_sizes[buf_idx];
            int elem_size = buf_elem_sizes[buf_idx];
            value_type buf = malloc(buf_elem_types[buf_idx], elem_count * elem_size);
            m_builder.CreateStore(buf, buf_ptr);
        }

        m_builder.CreateRetVoid();

        m_builder.ClearInsertionPoint();
    }

    // Generate output access function

    {
        const buffer & buf_info = m_stmt_buffers.back();
        assert(buf_info.on_stack == false);

        type_type ret_type = pointer(buffer_elem_type(buf_info.type));

        type_type buffer_struct_ptr_type =
                llvm::PointerType::get(m_buffer_struct_type, 0);

        llvm::FunctionType * func_type =
                llvm::FunctionType::get(ret_type,
                                        buffer_struct_ptr_type,
                                        false);

        llvm::Function * func =
                llvm::Function::Create(func_type,
                                       llvm::Function::ExternalLinkage,
                                       "get_output",
                                       m_module);

        value_type buf_struct_ptr = func->arg_begin();

        block_type block
                = llvm::BasicBlock::Create(llvm_context(), "", func);
        m_builder.SetInsertPoint(block);

        int data_field = buffer_kind_for(buf_info.type);
        vector<value_type> address = { value((int)0),
                                       value(data_field) };
        value_type buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
        value_type buf = m_builder.CreateLoad(buf_ptr);

        value_type output = m_builder.CreateGEP(buf, value(buf_info.index));

        m_builder.CreateRet(output);

        m_builder.ClearInsertionPoint();
    }
}

llvm_from_polyhedral::context
llvm_from_polyhedral::create_process_function
(schedule_type mode, const vector<semantic::type_ptr> & args)
{
    context ctx(mode);

    type_type buffer_ptr_type = llvm::PointerType::get(m_buffer_struct_type, 0);

    vector<type_type> param_types;

    for(const auto & arg : args)
    {
        type_type param_type;

        switch(arg->get_tag())
        {
        case semantic::type::boolean:
            param_type = buffer_elem_type(primitive_type::boolean);
            break;
        case semantic::type::integer_num:
            param_type = buffer_elem_type(primitive_type::integer);
            break;
        case semantic::type::real_num:
            param_type = buffer_elem_type(primitive_type::real);
            break;
        case semantic::type::stream:
            param_type = pointer(buffer_elem_type(primitive_type::real));
            break;
        default:
            throw string("Unexpected argument type.");
        }

        param_types.push_back(param_type);
    }

    param_types.push_back(buffer_ptr_type);

    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    param_types,
                                    false);

    const char *func_name =
            mode == initial_schedule ? "initialize" : "process";

    ctx.func =
            llvm::Function::Create(func_type,
                                   llvm::Function::ExternalLinkage,
                                   func_name,
                                   m_module);

    auto arg = ctx.func->arg_begin();

    for (int i = 0; i < args.size(); ++i)
    {
        value_type in = arg++;
        ostringstream name;
        name << "in" << i;
        in->setName(name.str());
        ctx.inputs.push_back(in);
    }

    value_type buffer_info_ptr = arg++;

    // Start block

    ctx.start_block = llvm::BasicBlock::Create(llvm_context(), "", ctx.func);
    m_builder.SetInsertPoint(ctx.start_block);

    // Load dynamic buffer addresses
    for (int buf_idx = 0; buf_idx < buffer_kind_count; ++buf_idx)
    {
        vector<value_type> address = { value((int32_t)0), value((int32_t)buf_idx) };
        value_type buf =
                m_builder.CreateLoad(m_builder.CreateGEP(buffer_info_ptr, address));
        ctx.mem_buffers.push_back(buf);
    }

    // Allocate stack buffers

    int stack_buffer_count = 0;
    for (const buffer & buf : m_stmt_buffers)
    {
        if (buf.on_stack)
            ++stack_buffer_count;
    }
    ctx.stack_buffers.resize(stack_buffer_count);
    for (const buffer & buf : m_stmt_buffers)
    {
        if (!buf.on_stack)
            continue;

        type_type elem_type = buffer_elem_type(buf.type);
        value_type buf_ptr;
        if (buf.size > 1)
        {
            type_type array_type = this->array_type(elem_type, buf.domain);
            buf_ptr = m_builder.CreateAlloca(array_type);
        }
        else
        {
            assert(buf.size == 1);
            buf_ptr = m_builder.CreateAlloca(elem_type);
        }

        ctx.stack_buffers[buf.index] = buf_ptr;
    }

    // End block

    ctx.end_block = llvm::BasicBlock::Create(llvm_context(), "", ctx.func);
    m_builder.SetInsertPoint(ctx.end_block);

    if (mode == periodic_schedule)
        advance_buffers(ctx);

    m_builder.CreateRetVoid();

    return ctx;
}

llvm_from_polyhedral::block_type
llvm_from_polyhedral::generate_statement( const string & name,
                                     const index_type & index,
                                     const context & ctx,
                                     block_type block )
{
    auto stmt_ref = std::find_if(m_statements.begin(), m_statements.end(),
                                 [&](statement *s){ return s->name == name; });
    assert(stmt_ref != m_statements.end());
    return generate_statement(*stmt_ref, index, ctx, block);
}

llvm_from_polyhedral::block_type
llvm_from_polyhedral::generate_statement
( statement *stmt, const index_type & ctx_index,
  const context & ctx, block_type block )
{
    // Drop first dimension denoting period (always zero).
    assert(!ctx_index.empty());
    vector<value_type> index(ctx_index.begin()+1, ctx_index.end());

    m_builder.SetInsertPoint(block);

    // Offset steady-period index by initialization count
    // TODO:
    // - How does this affect the requirements for wrapping?
    // - Could this be avoided by rather modifying how the index is generated
    //   in the first place?
    vector<value_type> global_index = index;
    const dataflow::actor *actor = m_dataflow->find_actor_for(stmt);
    if (ctx.mode == periodic_schedule && actor)
    {
        value_type &flow_index = global_index[actor->flow_dimension];
        flow_index = m_builder.CreateAdd(flow_index,
                                         this->value((int64_t)actor->init_count));
    }

    value_type value;

    if (dynamic_cast<input_access*>(stmt->expr))
    {
        value = generate_input_access(stmt, index, ctx);
    }
    else
    {
        value = generate_expression(stmt->expr, global_index, ctx);
    }

    value_type dst = generate_buffer_access(stmt, global_index, ctx);
    store_buffer_elem(value, dst, stmt->expr->type);

    return m_builder.GetInsertBlock();
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_expression
( expression *expr, const index_type & index, const context & ctx )
{
    value_type result;

    if (auto operation = dynamic_cast<primitive_expr*>(expr))
    {
        result = generate_primitive(operation, index, ctx);
    }
    else if (auto input = dynamic_cast<input_access*>(expr))
    {
        result = generate_scalar_input_access(input, ctx);
    }
    else if (auto iterator = dynamic_cast<iterator_access*>(expr))
    {
        assert(iterator->dimension >= 0 && iterator->dimension < index.size());
        value_type val = index[iterator->dimension];
        val = m_builder.CreateTrunc(val, int32_type());
        if (iterator->ratio != 1)
            val = m_builder.CreateMul(val, value((int32_t)iterator->ratio));
        if (iterator->offset)
            val = m_builder.CreateAdd(val, value((int32_t)iterator->offset));
        result = val;
    }
    else if (auto read = dynamic_cast<stmt_access*>(expr))
    {
        vector<value_type> target_index = mapped_index(index, read->pattern);
        value_type address = generate_buffer_access(read->target, target_index, ctx);
        result = load_buffer_elem(address, expr->type);
    }
    else if (auto access = dynamic_cast<reduction_access*>(expr))
    {
        result = generate_reduction_access(access, index, ctx);
    }
    else if ( auto const_int = dynamic_cast<constant<int>*>(expr) )
    {
        result = value((int32_t) const_int->value);
    }
    else if ( auto const_double = dynamic_cast<constant<double>*>(expr) )
    {
        result = value(const_double->value);
    }
    else if ( auto const_bool = dynamic_cast<constant<bool>*>(expr) )
    {
        result = value(const_bool->value);
    }
    else
    {
        throw std::runtime_error("Unexpected expression type.");
    }

    result = convert(result, expr->type);

    return result;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_primitive
(primitive_expr *expr, const index_type & index, const context & ctx )
{
    switch(expr->op)
    {
    case primitive_op::logic_and:
    {
        // TODO: would branching to avoid unnecessary evaluation speed things up?
        value_type lhs = generate_expression(expr->operands[0], index, ctx);
        value_type rhs = generate_expression(expr->operands[1], index, ctx);
        return m_builder.CreateAnd(lhs, rhs);
    }
    case primitive_op::logic_or:
    {
        // TODO: would branching to avoid unnecessary evaluation speed things up?
        value_type lhs = generate_expression(expr->operands[0], index, ctx);
        value_type rhs = generate_expression(expr->operands[1], index, ctx);
        return m_builder.CreateOr(lhs, rhs);
    }
    case primitive_op::conditional:
    {
        block_type after_block = add_block("if.after");
        block_type true_block = add_block("if.true");
        block_type false_block = add_block("if.false");

        value_type condition = generate_expression(expr->operands[0], index, ctx);
        m_builder.CreateCondBr(condition, true_block, false_block);

        m_builder.SetInsertPoint(true_block);
        value_type true_value = generate_expression(expr->operands[1], index, ctx);
        true_value = convert(true_value, expr->type);
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(false_block);
        value_type false_value = generate_expression(expr->operands[2], index, ctx);
        false_value = convert(false_value, expr->type);
        m_builder.CreateBr(after_block);

        m_builder.SetInsertPoint(after_block);
        auto result = m_builder.CreatePHI(true_value->getType(), 2);
        result->addIncoming(true_value, true_block);
        result->addIncoming(false_value, false_block);

        return result;
    }
    default:
        break;
    }

    vector<value_type> operands;
    operands.reserve(expr->operands.size());
    for (expression * operand_expr : expr->operands)
    {
        operands.push_back( generate_expression(operand_expr, index, ctx) );
    }

    type_type d_type = double_type();
    type_type i_type = int32_type();
    type_type b_type = bool_type();

    switch(expr->op)
    {
    case primitive_op::negate:
    {
        value_type operand = operands[0];
        type_type operand_type = operand->getType();
        if (operand_type == i_type)
            return m_builder.CreateSub(value((int32_t)0), operand);
        else if (operand_type == d_type)
            return m_builder.CreateFSub(value((double)0), operand);
        else if (operand_type == b_type)
            return m_builder.CreateNot(operand);
    }
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
    case primitive_op::compare_g:
    case primitive_op::compare_geq:
    case primitive_op::compare_l:
    case primitive_op::compare_leq:
    {
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            switch(expr->op)
            {
            case primitive_op::add:
                return m_builder.CreateAdd(operands[0], operands[1]);
            case primitive_op::subtract:
                return m_builder.CreateSub(operands[0], operands[1]);
            case primitive_op::multiply:
                return m_builder.CreateMul(operands[0], operands[1]);
            case primitive_op::compare_g:
                return m_builder.CreateICmpSGT(operands[0], operands[1]);
            case primitive_op::compare_geq:
                return m_builder.CreateICmpSGE(operands[0], operands[1]);
            case primitive_op::compare_l:
                return m_builder.CreateICmpSLT(operands[0], operands[1]);
            case primitive_op::compare_leq:
                return m_builder.CreateICmpSLE(operands[0], operands[1]);
            default: assert(false);
            }
        }
        else
        {
            if (operands[0]->getType() != d_type)
                operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
            if (operands[1]->getType() != d_type)
                operands[1] = m_builder.CreateSIToFP(operands[1], d_type);
            switch(expr->op)
            {
            case primitive_op::add:
                return m_builder.CreateFAdd(operands[0], operands[1]);
            case primitive_op::subtract:
                return m_builder.CreateFSub(operands[0], operands[1]);
            case primitive_op::multiply:
                return m_builder.CreateFMul(operands[0], operands[1]);
            case primitive_op::compare_g:
                return m_builder.CreateFCmpUGT(operands[0], operands[1]);
            case primitive_op::compare_geq:
                return m_builder.CreateFCmpUGE(operands[0], operands[1]);
            case primitive_op::compare_l:
                return m_builder.CreateFCmpULT(operands[0], operands[1]);
            case primitive_op::compare_leq:
                return m_builder.CreateFCmpULE(operands[0], operands[1]);
            default: assert(false);
            }
        }
    }
    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
    {
        type_type lhs_type = operands[0]->getType();
        type_type rhs_type = operands[1]->getType();
        if (lhs_type == b_type || rhs_type == b_type)
        {
            assert(lhs_type == b_type && rhs_type == b_type);
            if (expr->op == primitive_op::compare_eq)
                return m_builder.CreateICmpEQ(operands[0], operands[1]);
            else
                return m_builder.CreateICmpNE(operands[0], operands[1]);
        }
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            if (expr->op == primitive_op::compare_eq)
                return m_builder.CreateICmpEQ(operands[0], operands[1]);
            else
                return m_builder.CreateICmpNE(operands[0], operands[1]);
        }
        else
        {
            value_type lhs = convert_to_real(operands[0]);
            value_type rhs = convert_to_real(operands[1]);
            if (expr->op == primitive_op::compare_eq)
                return m_builder.CreateFCmpUEQ(lhs, rhs);
            else
                return m_builder.CreateFCmpUNE(lhs, rhs);
        }
    }
    case primitive_op::divide:
    {
        if (operands[0]->getType() != d_type)
            operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
        if (operands[1]->getType() != d_type)
            operands[1] = m_builder.CreateSIToFP(operands[1], d_type);
        return m_builder.CreateFDiv(operands[0], operands[1]);
    }
    case primitive_op::divide_integer:
    {
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            return m_builder.CreateSDiv(operands[0], operands[1]);
        }
        else
        {
            if (operands[0]->getType() != d_type)
                operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
            if (operands[1]->getType() != d_type)
                operands[1] = m_builder.CreateSIToFP(operands[1], d_type);
            value_type f = m_builder.CreateFDiv(operands[0], operands[1]);
            return m_builder.CreateFPToSI(f, i_type);
        }
    }
    case primitive_op::modulo:
    {
        value_type x = operands[0];
        value_type y = operands[1];
        if (x->getType() == i_type && y->getType() == i_type)
        {
            // FIXME: handle floating point exception when divisor is 0
            value_type m = m_builder.CreateSRem(x,y);
            value_type m_not_zero = m_builder.CreateICmpNE(m, value((int32_t)0));
            value_type m_neg = m_builder.CreateICmpSLT(m, value((int32_t)0));
            value_type y_neg = m_builder.CreateICmpSLT(y, value((int32_t)0));
            value_type sign_m_not_y = m_builder.CreateICmpNE(m_neg, y_neg);
            value_type do_correct =
                    m_builder.CreateAnd(m_not_zero, sign_m_not_y);
            value_type m_corrected =
                    m_builder.CreateAdd(m, y);
            return m_builder.CreateSelect(do_correct, m_corrected, m);
        }
        else
        {
            x = convert_to_real(x);
            y = convert_to_real(y);
            value_type q = floor(m_builder.CreateFDiv(x,y));
            return m_builder.CreateFSub(x, m_builder.CreateFMul(y,q));
        }
    }
    case primitive_op::raise:
    {
        llvm::Intrinsic::ID id =
                (operands[1]->getType() == i_type) ?
                            llvm::Intrinsic::powi : llvm::Intrinsic::pow;

        if (operands[0]->getType() != d_type)
            operands[0] = m_builder.CreateSIToFP(operands[0], d_type);

        llvm::Function *func =
                llvm::Intrinsic::getDeclaration(m_module, id, d_type);

        return m_builder.CreateCall(func, operands);
    }
    case primitive_op::floor:
    case primitive_op::ceil:
    {
        value_type operand = operands[0];

        if (operand->getType() == i_type)
            return operand;

        llvm::Intrinsic::ID id = expr->op == primitive_op::floor ?
                    llvm::Intrinsic::floor : llvm::Intrinsic::ceil;

        llvm::Function *func =
                llvm::Intrinsic::getDeclaration(m_module, id, operand->getType());

        return m_builder.CreateCall(func, operand);
    }
    case primitive_op::abs:
    {
        value_type operand = operands[0];
        if (operand->getType() == d_type)
        {
            llvm::Function *func =
                    llvm::Intrinsic::getDeclaration(m_module,
                                                    llvm::Intrinsic::fabs,
                                                    d_type);
            return m_builder.CreateCall(func, operand);
        }
        else
        {
            value_type operand_is_positive =
                    m_builder.CreateICmpSGE(operand, value((uint32_t)0));
            value_type negated_operand =
                    m_builder.CreateSub(value((uint32_t)0), operand);
            return m_builder.CreateSelect(operand_is_positive,
                                          operand,
                                          negated_operand);
        }
    }
    case primitive_op::max:
    case primitive_op::min:
    {
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            value_type condition;
            if (expr->op == primitive_op::max)
                condition = m_builder.CreateICmpSGT(operands[0], operands[1]);
            else
                condition = m_builder.CreateICmpSLT(operands[0], operands[1]);
            return m_builder.CreateSelect(condition, operands[0], operands[1]);
        }
        else
        {
            if (operands[0]->getType() != d_type)
                operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
            if (operands[1]->getType() != d_type)
                operands[1] = m_builder.CreateSIToFP(operands[1], d_type);

            value_type condition;
            if (expr->op == primitive_op::max)
                condition = m_builder.CreateFCmpUGT(operands[0], operands[1]);
            else
                condition = m_builder.CreateFCmpULT(operands[0], operands[1]);

            return m_builder.CreateSelect(condition, operands[0], operands[1]);

#if 0 // Note: maxnum and minnum not supported by LLVM on my machine
            llvm::Intrinsic::ID id = op->op == primitive_op::max ?
                        llvm::Intrinsic::maxnum : llvm::Intrinsic::minnum;

            vector<type_type> arg_types = { d_type, d_type };

            if (operands[0]->getType() != d_type)
                operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
            if (operands[1]->getType() != d_type)
                operands[1] = m_builder.CreateSIToFP(operands[1], d_type);

            llvm::Function *func =
                    llvm::Intrinsic::getDeclaration(m_module,
                                                    id,
                                                    arg_types);
            return m_builder.CreateCall(func, operands);
#endif
        }
    }
    default:
        llvm::Intrinsic::ID id;
        switch(expr->op)
        {
        case primitive_op::log:
            id = llvm::Intrinsic::log; break;
        case primitive_op::log2:
            id = llvm::Intrinsic::log2; break;
        case primitive_op::log10:
            id = llvm::Intrinsic::log10; break;
        case primitive_op::exp:
            id = llvm::Intrinsic::exp; break;
        case primitive_op::exp2:
            id = llvm::Intrinsic::exp2; break;
        case primitive_op::sqrt:
            id = llvm::Intrinsic::sqrt; break;
        case primitive_op::sin:
            id = llvm::Intrinsic::sin; break;
        case primitive_op::cos:
            id = llvm::Intrinsic::cos; break;
        default:
            ostringstream text;
            text << "Unexpected primitive op: " << expr->op;
            throw std::runtime_error(text.str());
        }
        value_type operand = operands[0];
        if (operand->getType() != d_type)
            operand = m_builder.CreateSIToFP(operand, d_type);
        llvm::Function *func =
                llvm::Intrinsic::getDeclaration(m_module,
                                                id,
                                                d_type);
        return m_builder.CreateCall(func, operand);
    }
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_buffer_access
( statement *stmt, const index_type & index, const context & ctx )
{
    const buffer & buf_info = m_stmt_buffers[statement_index(stmt)];

    value_type buffer_ptr;

    if (buf_info.on_stack)
    {
        buffer_ptr = ctx.stack_buffers[buf_info.index];
    }
    else
    {
        buffer_ptr = ctx.mem_buffers[buffer_kind_for(buf_info.type)];

        // Avoid creating a GEP for single-element buffers,
        // so they can be optimized away.
        if (buf_info.index != 0)
        {
            buffer_ptr = m_builder.CreateGEP(buffer_ptr, value((int64_t)buf_info.index));
        }
        if (buf_info.size > 1)
        {
            buffer_ptr = m_builder.CreateBitCast(buffer_ptr, buffer_ptr_type(buf_info));
        }
    }

    if (buf_info.size > 1)
    {
        vector<value_type> elem_idx = buffer_index(stmt, index, ctx);
        vector<value_type> access_idx(elem_idx.size() + 1);
        access_idx[0] = value((int32_t)0);
        std::copy(elem_idx.begin(), elem_idx.end(), ++access_idx.begin());

        buffer_ptr = m_builder.CreateGEP(buffer_ptr, access_idx);
    }

    return buffer_ptr;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_input_access
( statement *stmt, const index_type & index, const context & ctx)
{
    vector<value_type> the_index(index);
    vector<int> the_domain(stmt->domain);
    const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);

    if (actor)
    {
        switch(ctx.mode)
        {
        case initial_schedule:
            the_domain[actor->flow_dimension] = actor->init_count;
            break;
        case periodic_schedule:
            the_domain[actor->flow_dimension] = actor->steady_count;
            break;
        }
        transpose(the_index, actor->flow_dimension);
        transpose(the_domain, actor->flow_dimension);
    }

    value_type flat_index = this->flat_index(the_index, the_domain);

    int input_num = reinterpret_cast<input_access*>(stmt->expr)->index;

    value_type input = ctx.inputs[input_num];
    assert(input->getType()->isPointerTy());
    input = m_builder.CreateGEP(input, flat_index);
    input = load_buffer_elem(input, stmt->expr->type);

    return input;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_scalar_input_access( input_access *access, const context & ctx )
{
    int input_num = access->index;
    value_type val = ctx.inputs[input_num];
    if (access->type == primitive_type::boolean)
        val = m_builder.CreateTrunc(val, type(primitive_type::boolean));
    return val;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::generate_reduction_access
( reduction_access *access, const index_type & index, const context & ctx)
{
    int reduction_dim = access->reductor->domain.size() - 1;
    assert(reduction_dim < index.size());

    block_type init_block = add_block("reduction.init");
    block_type recall_block = add_block("reduction.recall");
    block_type after_block = add_block("reduction.after");

    value_type cond = m_builder.CreateICmpEQ(index[reduction_dim],
                                             value((int64_t)0));

    m_builder.CreateCondBr(cond, init_block, recall_block);

    value_type init_value, recall_value;

    {
        m_builder.SetInsertPoint(init_block);

        assert(index.size() >= access->initializer->domain.size());
        vector<value_type> init_index;
        if (index.size() > 1)
        {
            int init_dims = access->initializer->domain.size();
            init_index.insert(init_index.end(),
                              index.begin(),
                              index.begin() + init_dims);
        }
        else
        {
            init_index.push_back(0);
        }
        assert(access->initializer->domain.size() == init_index.size());
        value_type init_ptr = generate_buffer_access(access->initializer, init_index, ctx);
        init_value = load_buffer_elem(init_ptr, access->initializer->expr->type);

        m_builder.CreateBr(after_block);
    }

    {
        m_builder.SetInsertPoint(recall_block);

        int reduction_dim = access->reductor->domain.size() - 1;
        vector<value_type> reductor_index;
        reductor_index.insert(reductor_index.end(),
                              index.begin(), index.begin() + reduction_dim + 1);
        reductor_index[reduction_dim] =
                m_builder.CreateSub(reductor_index[reduction_dim],
                                    value((int64_t) 1));
        value_type recall_ptr = generate_buffer_access(access->reductor,
                                                       reductor_index, ctx);
        recall_value = load_buffer_elem(recall_ptr, access->reductor->expr->type);

        m_builder.CreateBr(after_block);
    }

    m_builder.SetInsertPoint(after_block);

    llvm::PHINode *value = m_builder.CreatePHI(double_type(), 2);
    value->addIncoming(init_value, init_block);
    value->addIncoming(recall_value, recall_block);

    return value;
}

void llvm_from_polyhedral::advance_buffers(const context & ctx)
{
    int buf_idx = -1;
    for (statement * stmt : m_statements)
    {
        ++buf_idx;

        const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);

        if (!actor)
            continue;

        const buffer & buf = m_stmt_buffers[buf_idx];

        if (!buf.has_phase)
            continue;

        int offset = actor->steady_count;
        int buffer_size = stmt->buffer[actor->flow_dimension];

        value_type phase_ptr =
                m_builder.CreateGEP(ctx.mem_buffers[phase_buffer], value(buf.phase_index));
        value_type phase_val = m_builder.CreateLoad(phase_ptr);
        value_type offset_val = value(phase_val->getType(), offset);
        value_type buf_size_val = value(phase_val->getType(), buffer_size);
        phase_val = m_builder.CreateAdd(phase_val, offset_val);
        phase_val = m_builder.CreateSRem(phase_val, buf_size_val);
        m_builder.CreateStore(phase_val, phase_ptr);
    }
}

vector<llvm_from_polyhedral::value_type>
llvm_from_polyhedral::buffer_index
( statement * stmt, const index_type & index, const context & ctx )
{
    // Get basic info about accessed statement

    const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
    int stmt_index = statement_index(stmt);
    const buffer & buf = m_stmt_buffers[stmt_index];

    // Prepare info

    vector<value_type> the_index(index);
    vector<int> the_domain = stmt->domain;
    vector<int> the_buffer_size = stmt->buffer;

    if (actor)
    {
        the_domain[actor->flow_dimension] = actor->init_count;
        if (ctx.mode == periodic_schedule)
            the_domain[actor->flow_dimension] += actor->steady_count;
    }

    // Add flow index phase

    if (actor &&  ctx.mode == periodic_schedule)
    {
        if (buf.has_phase)
        {
            value_type phase_ptr =
                    m_builder.CreateGEP(ctx.mem_buffers[phase_buffer], value(buf.phase_index));
            value_type phase =
                    m_builder.CreateLoad(phase_ptr);

            value_type & flow_index = the_index[actor->flow_dimension];
            flow_index = m_builder.CreateAdd(flow_index, phase);
        }
    }

    // Wrap index

    for (int dim = 0; dim < the_index.size(); ++dim)
    {
        int domain_size = the_domain[dim];
        int buffer_size = the_buffer_size[dim];

        if (buffer_size < 2)
        {
            the_index[dim] = value((int64_t) 0);
            continue;
        }

        bool may_wrap =
                domain_size > buffer_size ||
                (actor && dim == actor->flow_dimension && buf.has_phase);

        if (may_wrap)
        {
            value_type & dim_index = the_index[dim];
            dim_index = m_builder.CreateSRem( dim_index,
                                              value((int64_t)buffer_size) );
            // FIXME: Avoid correction when possible
            value_type idx_is_pos =
                    m_builder.CreateICmpSGE(dim_index, value((int64_t)0));
            value_type correct_dim_index =
                    m_builder.CreateAdd(dim_index, value((int64_t)buffer_size));
            dim_index =
                    m_builder.CreateSelect(idx_is_pos, dim_index, correct_dim_index);
        }
    }

    if (actor)
    {
        transpose(the_index, actor->flow_dimension);
    }

    return the_index;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::load_buffer_elem(value_type ptr, primitive_type t)
{
    value_type val = m_builder.CreateLoad(ptr);
    if (t == primitive_type::boolean)
        return m_builder.CreateTrunc(val, type(primitive_type::boolean));
    else
        return val;
}

void
llvm_from_polyhedral::store_buffer_elem(value_type val, value_type ptr, primitive_type t)
{
    if (t == primitive_type::boolean)
        val = m_builder.CreateZExt(val, buffer_elem_type(primitive_type::boolean));
    m_builder.CreateStore(val, ptr);
}

int llvm_from_polyhedral::buffer_elem_size(primitive_type t)
{
    switch(t)
    {
    case primitive_type::boolean:
        return 4;
    case primitive_type::integer:
        return 4;
    case primitive_type::real:
        return 8;
    }
    throw false;
}

llvm_from_polyhedral::type_type
llvm_from_polyhedral::buffer_elem_type(primitive_type t)
{
    switch(t)
    {
    case primitive_type::boolean:
        return int32_type();
    case primitive_type::integer:
        return int32_type();
    case primitive_type::real:
        return double_type();
    }
    throw false;
}

llvm_from_polyhedral::type_type
llvm_from_polyhedral::buffer_type(const buffer &buf)
{
    return array_type(buffer_elem_type(buf.type), buf.domain);
}

llvm_from_polyhedral::type_type
llvm_from_polyhedral::buffer_ptr_type(const buffer &buf)
{
    return pointer(buffer_type(buf));
}

llvm_from_polyhedral::type_type
llvm_from_polyhedral::array_type(type_type elem_type, const vector<int> domain)
{
    type_type result_type = elem_type;
    for (int i = domain.size() - 1; i >= 0; --i)
    {
        assert(domain[i] >= 0);
        result_type = llvm::ArrayType::get(result_type, (uint64_t)domain[i]);
    }
    return result_type;
}

template <typename T>
void llvm_from_polyhedral::transpose( vector<T> & index, int first_dim )
{
    assert(first_dim < index.size());
    T tmp = index[first_dim];
    for (int i = first_dim; i > 0; --i)
        index[i] = index[i-1];
    index[0] = tmp;
}

vector<llvm_from_polyhedral::value_type>
llvm_from_polyhedral::mapped_index
( const vector<value_type> & index, const mapping & map )
{
    assert(index.size() == map.input_dimension());

    vector<value_type> target_index;
    target_index.reserve(map.output_dimension());

    for(int out_dim = 0; out_dim < map.output_dimension(); ++out_dim)
    {
        value_type out_value = value((int64_t) map.constant(out_dim));
        for (int in_dim = 0; in_dim < map.input_dimension(); ++in_dim)
        {
            int coefficient = map.coefficient(in_dim, out_dim);
            if (coefficient == 0)
                continue;
            value_type term = index[in_dim];
            if (coefficient != 1)
                term = m_builder.CreateMul(term, value((int64_t)coefficient));
            out_value = m_builder.CreateAdd(out_value, term);
        }
        target_index.push_back(out_value);
    }

    return target_index;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::flat_index
( const vector<value_type> & index, const vector<int> & domain )
{
    assert(index.size() > 0);
    assert(domain.size() == index.size());

    value_type flat_index = index.back();

    int factor = 1;
    for( int i = index.size() - 2; i >= 0; --i )
    {
        factor *= domain[i+1];
        assert(factor != 0);
        value_type term = index[i];
        if (factor != 1)
            term = m_builder.CreateMul(term, value((int64_t)factor));
        flat_index = m_builder.CreateAdd(flat_index, term);
    }
#if 0

    value_type flat_index = index[0];
    for( unsigned int i = 1; i < index.size(); ++i)
    {
        int size = domain[i];
        assert(size >= 0);
        flat_index = m_builder.CreateMul(flat_index, value((int64_t) size));
        flat_index = m_builder.CreateAdd(flat_index, index[i]);
    }
#endif
    return flat_index;
}

int llvm_from_polyhedral::statement_index( statement * stmt )
{
    auto stmt_ref = std::find(m_statements.begin(), m_statements.end(), stmt);
    assert(stmt_ref != m_statements.end());
    return (int) std::distance(m_statements.begin(), stmt_ref);
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::malloc( type_type t, std::uint64_t size )
{
    // FIXME: determine size_t type!

    type_type t_ptr_type = llvm::PointerType::get(t, 0);

    value_type malloc_func =
            m_module->getOrInsertFunction("malloc",
                                          llvm::AttributeSet(),
                                          llvm::Type::getInt8PtrTy(llvm_context()),
                                          int64_type(), nullptr);

    value_type ptr = m_builder.CreateCall(malloc_func, value(size));

    return m_builder.CreateBitCast(ptr, t_ptr_type);
}

llvm_from_polyhedral::type_type
llvm_from_polyhedral::type(primitive_type t)
{
    switch(t)
    {
    case primitive_type::boolean:
        return bool_type();
    case primitive_type::integer:
        return int32_type();
    case primitive_type::real:
        return double_type();
    }
    throw false;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::convert( value_type v,
                          primitive_type t )
{
    switch(t)
    {
    case primitive_type::real:
    {
        return convert_to_real(v);
    }
    case primitive_type::integer:
    {
        return convert_to_integer(v);
    }
    case primitive_type::boolean:
    {
        return convert_to_boolean(v);
    }
    }
    throw false;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::convert_to_real( value_type v )
{
    type_type t = v->getType();

    if (t == bool_type())
        return m_builder.CreateUIToFP(v, double_type());
    else if (t == int32_type())
        return m_builder.CreateSIToFP(v, double_type());

    return v;
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::convert_to_integer( value_type v )
{
    type_type t = v->getType();

    if (t == bool_type())
        return m_builder.CreateZExt(v, int32_type());
    else if (t == double_type())
        return m_builder.CreateFPToSI(v, int32_type());

    return v;
}


llvm_from_polyhedral::value_type
llvm_from_polyhedral::convert_to_boolean( value_type v )
{
    type_type t = v->getType();

    if (t == type(primitive_type::boolean))
        return v;
    else
        throw runtime_error("LLVM generator: can not convert value to boolean.");
}

llvm_from_polyhedral::value_type
llvm_from_polyhedral::floor( value_type v )
{
    llvm::Function *f =
            llvm::Intrinsic::getDeclaration(m_module, llvm::Intrinsic::floor, v->getType());

    return m_builder.CreateCall(f, v);
}

}
}
