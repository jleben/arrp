#include "llvm_from_model.hpp"

#include <llvm/IR/Intrinsics.h>

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace polyhedral {

int volume( vector<int> extent )
{
    if (extent.empty())
        return 0;
    int v = 1;
    for(int e : extent)
        v *= e;
    return v;
}

llvm_from_model::llvm_from_model
(llvm::Module *module,
 const vector<statement*> & statements,
 const dataflow::model *dataflow ):
    m_module(module),
    m_builder(module->getContext()),
    m_statements(statements),
    m_dataflow(dataflow)
{
    // Analyze buffer requirements

    int int_buf_idx = 0;
    int real_buf_idx = 0;
    int phase_buf_idx = 0;

    for (statement *stmt : statements)
    {
        buffer buf;

        buf.type = stmt->expr->type;

        switch(stmt->expr->type)
        {
        case integer:
            buf.index = int_buf_idx;
            int_buf_idx += volume(stmt->buffer);
            break;
        case real:
            buf.index = real_buf_idx;
            real_buf_idx += volume(stmt->buffer);
            break;
        default:
            throw std::runtime_error("Unexpected expression type.");
        }

        const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
        if (actor)
        {
            bool period_overlaps =
                    actor->steady_count % stmt->buffer[actor->flow_dimension] != 0;
            bool init_overlaps =
                    actor->init_count % stmt->buffer[actor->flow_dimension] != 0;

            buf.has_phase = period_overlaps || init_overlaps;
            buf.phase_index = phase_buf_idx++;
        }
        else
        {
            buf.has_phase = false;
            buf.phase_index = -1;
        }

        m_stmt_buffers.push_back(buf);
    }

    //

    type_type i32_ptr_type = llvm::PointerType::get(int32_type(), 0);
    type_type i64_ptr_type = llvm::PointerType::get(int64_type(), 0);
    type_type double_ptr_type = llvm::PointerType::get(double_type(), 0);

    vector<type_type> buffer_struct_member_types =
    {
        double_ptr_type,
        i32_ptr_type,
        i64_ptr_type
    };

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

        if (real_buf_idx)
        {
            vector<value_type> address = { value((int32_t)0), value((int32_t)0) };
            value_type real_buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
            value_type buf = malloc(double_type(), real_buf_idx * 8);
            m_builder.CreateStore(buf, real_buf_ptr);
        }
        if (int_buf_idx)
        {
            vector<value_type> address = { value((int32_t)0), value((int32_t)1) };
            value_type int_buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
            value_type buf = malloc(int32_type(), real_buf_idx * 4);
            m_builder.CreateStore(buf, int_buf_ptr);
        }
        if (phase_buf_idx)
        {
            vector<value_type> address = { value((int32_t)0), value((int32_t)2) };
            value_type phase_buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
            value_type buf = malloc(int64_type(), real_buf_idx * 8);
            m_builder.CreateStore(buf, phase_buf_ptr);
        }

        m_builder.CreateRetVoid();

        m_builder.ClearInsertionPoint();
    }

    // Generate output access function

    {
        const buffer & buf_info = m_stmt_buffers.back();

        type_type ret_type;

        switch(buf_info.type)
        {
        case integer:
            ret_type = pointer(int32_type());
            break;
        case real:
            ret_type = pointer(double_type());
            break;
        default:
            assert(false);
        }

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

        int data_field = buf_info.type == real ? 0 : 1;
        vector<value_type> address = { value((int)0),
                                       value(data_field) };
        value_type buf_ptr = m_builder.CreateGEP(buf_struct_ptr, address);
        value_type buf = m_builder.CreateLoad(buf_ptr);

        value_type output = m_builder.CreateGEP(buf, value(buf_info.index));

        m_builder.CreateRet(output);

        m_builder.ClearInsertionPoint();
    }
}

llvm_from_model::context
llvm_from_model::create_process_function
(schedule_type mode, const vector<semantic::type_ptr> & args)
{
    context ctx(mode);

    type_type buffer_ptr_type = llvm::PointerType::get(m_buffer_struct_type, 0);

    vector<type_type> param_types;

    for(const auto & arg : args)
    {
        switch(arg->get_tag())
        {
        case semantic::type::integer_num:
            param_types.push_back(int32_type());
            break;
        case semantic::type::real_num:
            param_types.push_back(double_type());
            break;
        case semantic::type::stream:
            param_types.push_back(llvm::PointerType::get(double_type(),0));
            break;
        default:
            throw string("Unexpected argument type.");
        }
    }

    param_types.push_back(buffer_ptr_type);

    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    param_types,
                                    false);

    const char *func_name;
    switch(mode)
    {
    case initial_schedule:
        func_name = "initialize"; break;
    case periodic_schedule:
        func_name = "process"; break;
    }

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

    {
        vector<value_type> address = { value((int32_t)0), value((int32_t)0) };
        ctx.real_buffer = m_builder.CreateLoad(
                    m_builder.CreateGEP(buffer_info_ptr, address), "real_buf");
    }
    {
        vector<value_type> address = { value((int32_t)0), value((int32_t)1) };
        ctx.int_buffer = m_builder.CreateLoad(
                    m_builder.CreateGEP(buffer_info_ptr, address), "int_buf");
    }
    {
        vector<value_type> address = { value((int32_t)0), value((int32_t)2) };
        ctx.phase_buffer = m_builder.CreateLoad(
                    m_builder.CreateGEP(buffer_info_ptr, address), "phase_buf");
    }

    // End block

    ctx.end_block = llvm::BasicBlock::Create(llvm_context(), "", ctx.func);
    m_builder.SetInsertPoint(ctx.end_block);

    if (mode == periodic_schedule)
        advance_buffers(ctx);

    m_builder.CreateRetVoid();

    return ctx;
}

llvm_from_model::block_type
llvm_from_model::generate_statement( const string & name,
                                     const index_type & index,
                                     const context & ctx,
                                     block_type block )
{
    auto stmt_ref = std::find_if(m_statements.begin(), m_statements.end(),
                                 [&](statement *s){ return s->name == name; });
    assert(stmt_ref != m_statements.end());
    return generate_statement(*stmt_ref, index, ctx, block);
}

llvm_from_model::block_type
llvm_from_model::generate_statement
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

    if (auto input = dynamic_cast<input_access*>(stmt->expr))
    {
        value = generate_input_access(stmt, index, ctx);
    }
    else
    {
        value = generate_expression(stmt->expr, global_index, ctx);
    }

    value_type dst = generate_buffer_access(stmt, global_index, ctx);

#if 0
    if (value->getType() != double_type())
        value = m_builder.CreateSIToFP(value, double_type());
#endif
    m_builder.CreateStore(value, dst);

    return m_builder.GetInsertBlock();
}

llvm_from_model::value_type
llvm_from_model::generate_expression
( expression *expr, const index_type & index, const context & ctx )
{
    value_type result;

    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        result = generate_intrinsic(operation, index, ctx);
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
        result = m_builder.CreateLoad(address);
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
    else
    {
        throw std::runtime_error("Unexpected expression type.");
    }

    if (result->getType() == int32_type() && expr->type == polyhedral::real)
    {
        result = m_builder.CreateSIToFP(result, double_type());
    }
    else if (result->getType() == double_type() && expr->type == polyhedral::integer)
    {
        result = m_builder.CreateFPToSI(result, int32_type());
    }

    return result;
}

llvm_from_model::value_type
llvm_from_model::generate_intrinsic
( intrinsic *op, const index_type & index, const context & ctx )
{
    vector<value_type> operands;
    operands.reserve(op->operands.size());
    for (expression * expr : op->operands)
    {
        operands.push_back( generate_expression(expr, index, ctx) );
    }

    type_type d_type = double_type();
    type_type i_type = int32_type();

    switch(op->kind)
    {
    case intrinsic::negate:
    {
        value_type operand = operands[0];
        if (operand->getType() == i_type)
            return m_builder.CreateSub(value((int32_t)0), operand);
        else
            return m_builder.CreateFSub(value((double)0), operand);
    }
    case intrinsic::add:
    case intrinsic::subtract:
    case intrinsic::multiply:
    {
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            switch(op->kind)
            {
            case intrinsic::add:
                return m_builder.CreateAdd(operands[0], operands[1]);
            case intrinsic::subtract:
                return m_builder.CreateSub(operands[0], operands[1]);
            case intrinsic::multiply:
                return m_builder.CreateMul(operands[0], operands[1]);
            default: assert(false);
            }
        }
        else
        {
            if (operands[0]->getType() != d_type)
                operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
            if (operands[1]->getType() != d_type)
                operands[1] = m_builder.CreateSIToFP(operands[1], d_type);
            switch(op->kind)
            {
            case intrinsic::add:
                return m_builder.CreateFAdd(operands[0], operands[1]);
            case intrinsic::subtract:
                return m_builder.CreateFSub(operands[0], operands[1]);
            case intrinsic::multiply:
                return m_builder.CreateFMul(operands[0], operands[1]);
            default: assert(false);
            }
        }
    }
    case intrinsic::divide:
    {
        if (operands[0]->getType() != d_type)
            operands[0] = m_builder.CreateSIToFP(operands[0], d_type);
        if (operands[1]->getType() != d_type)
            operands[1] = m_builder.CreateSIToFP(operands[1], d_type);
        return m_builder.CreateFDiv(operands[0], operands[1]);
    }
    case intrinsic::divide_integer:
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
    case intrinsic::raise:
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
    case intrinsic::floor:
    case intrinsic::ceil:
    {
        value_type operand = operands[0];

        if (operand->getType() == i_type)
            return operand;

        llvm::Intrinsic::ID id = op->kind == intrinsic::floor ?
                    llvm::Intrinsic::floor : llvm::Intrinsic::ceil;

        llvm::Function *func =
                llvm::Intrinsic::getDeclaration(m_module, id, operand->getType());

        return m_builder.CreateCall(func, operand);
    }
    case intrinsic::abs:
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
    case intrinsic::max:
    case intrinsic::min:
    {
        if (operands[0]->getType() == i_type && operands[1]->getType() == i_type)
        {
            value_type condition;
            if (op->kind == intrinsic::max)
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
            if (op->kind == intrinsic::max)
                condition = m_builder.CreateFCmpUGT(operands[0], operands[1]);
            else
                condition = m_builder.CreateFCmpULT(operands[0], operands[1]);

            return m_builder.CreateSelect(condition, operands[0], operands[1]);

#if 0 // Note: maxnum and minnum not supported by LLVM on my machine
            llvm::Intrinsic::ID id = op->kind == intrinsic::max ?
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
        switch(op->kind)
        {
        case intrinsic::log:
            id = llvm::Intrinsic::log; break;
        case intrinsic::log2:
            id = llvm::Intrinsic::log2; break;
        case intrinsic::log10:
            id = llvm::Intrinsic::log10; break;
        case intrinsic::exp:
            id = llvm::Intrinsic::exp; break;
        case intrinsic::exp2:
            id = llvm::Intrinsic::exp2; break;
        case intrinsic::sqrt:
            id = llvm::Intrinsic::sqrt; break;
        case intrinsic::sin:
            id = llvm::Intrinsic::sin; break;
        case intrinsic::cos:
            id = llvm::Intrinsic::cos; break;
        default:
            ostringstream text;
            text << "Unexpected intrinsic type: " << op->kind;
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

llvm_from_model::value_type
llvm_from_model::generate_buffer_access
( statement *stmt, const index_type & index, const context & ctx )
{
    value_type flat_index = flat_buffer_index(stmt, index, ctx);
    value_type stmt_index = value( (int32_t) statement_index(stmt) );
    value_type buffer;
    {
        switch(stmt->expr->type)
        {
        case integer:
        {
            buffer = ctx.int_buffer;
            break;
        }
        case real:
        {
            buffer = ctx.real_buffer;
            break;
        }
        default:
            throw string("Unexpected expression type.");
        }
    }

    value_type buffer_element_ptr =
            m_builder.CreateGEP(buffer, flat_index);

    return buffer_element_ptr;
}

llvm_from_model::value_type
llvm_from_model::generate_input_access
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
    input = m_builder.CreateLoad(input);

    return input;
}

llvm_from_model::value_type
llvm_from_model::generate_scalar_input_access( input_access *access, const context & ctx )
{
    int input_num = access->index;
    return ctx.inputs[input_num];
}

llvm_from_model::value_type
llvm_from_model::generate_reduction_access
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
        init_value = m_builder.CreateLoad(init_ptr);

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
        recall_value = m_builder.CreateLoad(recall_ptr);

        m_builder.CreateBr(after_block);
    }

    m_builder.SetInsertPoint(after_block);

    llvm::PHINode *value = m_builder.CreatePHI(double_type(), 2);
    value->addIncoming(init_value, init_block);
    value->addIncoming(recall_value, recall_block);

    return value;
}

void llvm_from_model::advance_buffers(const context & ctx)
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
                m_builder.CreateGEP(ctx.phase_buffer, value(buf.phase_index));
        value_type phase_val = m_builder.CreateLoad(phase_ptr);
        value_type offset_val = value(phase_val->getType(), offset);
        value_type buf_size_val = value(phase_val->getType(), buffer_size);
        phase_val = m_builder.CreateAdd(phase_val, offset_val);
        phase_val = m_builder.CreateSRem(phase_val, buf_size_val);
        m_builder.CreateStore(phase_val, phase_ptr);
    }
}

llvm_from_model::value_type
llvm_from_model::flat_buffer_index
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
                    m_builder.CreateGEP(ctx.phase_buffer, value(buf.phase_index));
            value_type phase =
                    m_builder.CreateLoad(phase_ptr);

            value_type & flow_index = the_index[actor->flow_dimension];
            flow_index = m_builder.CreateAdd(flow_index, phase);
        }
    }

    // Wrap index

    for (int dim = 0; dim < the_index.size(); ++dim)
    {
        int domain = the_domain[dim];
        int buffer = the_buffer_size[dim];

        if (buffer < 2)
        {
            the_index[dim] = value((int64_t) 0);
            continue;
        }

        bool may_wrap =
                domain > buffer ||
                (actor && dim == actor->flow_dimension && buf.has_phase);

        if (may_wrap)
        {
            value_type & dim_index = the_index[dim];
            dim_index = m_builder.CreateSRem( dim_index,
                                              value((int64_t)buffer) );
        }
    }

    // Flatten index

    if (actor)
    {
        transpose(the_index, actor->flow_dimension);
        transpose(the_buffer_size, actor->flow_dimension);
    }

    value_type flat_index = this->flat_index(the_index, the_buffer_size);

    // Add statement offset:
    if (buf.index != 0)
        flat_index = m_builder.CreateAdd(flat_index, value((int64_t)buf.index));

    return flat_index;
}

template <typename T>
void llvm_from_model::transpose( vector<T> & index, int first_dim )
{
    assert(first_dim < index.size());
    T tmp = index[first_dim];
    for (int i = first_dim; i > 0; --i)
        index[i] = index[i-1];
    index[0] = tmp;
}

vector<llvm_from_model::value_type>
llvm_from_model::mapped_index
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
            value_type coefficient =
                    value((int64_t) map.coefficient(in_dim, out_dim));
            value_type term = m_builder.CreateMul(index[in_dim], coefficient);
            out_value = m_builder.CreateAdd(out_value, term);
        }
        target_index.push_back(out_value);
    }

    return target_index;
}

llvm_from_model::value_type
llvm_from_model::flat_index
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

int llvm_from_model::statement_index( statement * stmt )
{
    auto stmt_ref = std::find(m_statements.begin(), m_statements.end(), stmt);
    assert(stmt_ref != m_statements.end());
    return (int) std::distance(m_statements.begin(), stmt_ref);
}

llvm_from_model::value_type
llvm_from_model::malloc( type_type t, std::uint64_t size )
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

}
}
