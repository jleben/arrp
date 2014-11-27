#include "llvm_from_model.hpp"

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

namespace stream {
namespace polyhedral {

llvm_from_model::llvm_from_model
(llvm::Module *module,
 int input_count,
 const vector<statement*> & statements,
 const dataflow::model *dataflow):
    m_module(module),
    m_builder(module->getContext()),
    m_statements(statements),
    m_dataflow(dataflow)
{
    type_type i8_ptr_type = llvm::Type::getInt8PtrTy(llvm_context());
    type_type i8_ptr_ptr_type = llvm::PointerType::get(i8_ptr_type, 0);
    type_type buffer_type =
            llvm::StructType::create(vector<type_type>({i8_ptr_type, int32_type()}));
    type_type buffer_pointer_type = llvm::PointerType::get(buffer_type, 0);

    // inputs, output, buffers
    vector<type_type> arg_types = { i8_ptr_ptr_type, buffer_pointer_type };

    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    arg_types,
                                    false);

    m_function = llvm::Function::Create(func_type,
                                        llvm::Function::ExternalLinkage,
                                        "process",
                                        m_module);

    auto arg = m_function->arg_begin();
    m_inputs = arg++;
    //m_output = arg++;
    m_buffers = arg++;

    m_start_block = llvm::BasicBlock::Create(llvm_context(), "", m_function);

    m_end_block = llvm::BasicBlock::Create(llvm_context(), "", m_function);
    m_builder.SetInsertPoint(m_end_block);

    advance_buffers();

    m_builder.CreateRetVoid();
}

llvm_from_model::block_type
llvm_from_model::generate_statement( const string & name,
                                     const vector<value_type> & index,
                                     block_type block )
{
    auto stmt_ref = std::find_if(m_statements.begin(), m_statements.end(),
                                 [&](statement *s){ return s->name == name; });
    assert(stmt_ref != m_statements.end());
    return generate_statement(*stmt_ref, index, block);
}

llvm_from_model::block_type
llvm_from_model::generate_statement
( statement *stmt, const vector<value_type> & raw_index, block_type block )
{
    // Drop first dimension denoting period (always zero).
    assert(!raw_index.empty());
    vector<value_type> index(raw_index.begin()+1, raw_index.end());

    m_builder.SetInsertPoint(block);

    // Offset by initialization count
    // TODO:
    // - How does this affect the requirements for wrapping?
    // - Could this be avoid by rather modifying how the index is generated
    //   in the first place?
    vector<value_type> offset_index = index;
    const dataflow::actor *actor = m_dataflow->find_actor_for(stmt);
    if (actor)
    {
        value_type &flow_index = offset_index[actor->flow_dimension];
        flow_index = m_builder.CreateAdd(flow_index,
                                         this->value((int64_t)actor->init_count));
    }

    value_type value;

    if (auto input = dynamic_cast<input_access*>(stmt->expr))
    {
        value_type address = generate_input_access(stmt, index);
        value = m_builder.CreateLoad(address);
    }
    else
    {
        value = generate_expression(stmt->expr, offset_index);
    }

    value_type dst = generate_buffer_access(stmt, offset_index);

    m_builder.CreateStore(value, dst);

    return m_builder.GetInsertBlock();
}

llvm_from_model::value_type
llvm_from_model::generate_expression
( expression *expr, const vector<value_type> & index )
{
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        return generate_intrinsic(operation, index);
    }
    if (auto read = dynamic_cast<stream_access*>(expr))
    {
        vector<value_type> target_index = mapped_index(index, read->pattern);
        value_type address = generate_buffer_access(read->target, target_index);
        return m_builder.CreateLoad(address);
    }
    if (auto access = dynamic_cast<reduction_access*>(expr))
    {
        return generate_reduction_access(access, index);
    }
    if ( auto const_int = dynamic_cast<constant<int>*>(expr) )
    {
        // FIXME: integers...
        return value((double) const_int->value);
    }
    if ( auto const_double = dynamic_cast<constant<double>*>(expr) )
    {
        return value(const_double->value);
    }
    throw std::runtime_error("Unexpected expression type.");
}

llvm_from_model::value_type
llvm_from_model::generate_intrinsic
( intrinsic *op, const vector<value_type> & index )
{
    vector<value_type> operands;
    operands.reserve(op->operands.size());
    for (expression * expr : op->operands)
    {
        operands.push_back( generate_expression(expr, index) );
    }

    switch(op->kind)
    {
    case intrinsic::add:
        return m_builder.CreateFAdd(operands[0], operands[1]);
    case intrinsic::subtract:
        return m_builder.CreateFSub(operands[0], operands[1]);
    case intrinsic::multiply:
        return m_builder.CreateFMul(operands[0], operands[1]);
    case intrinsic::divide:
        return m_builder.CreateFDiv(operands[0], operands[1]);
    default:
        throw std::runtime_error("Unexpected expression type.");
    }
}

llvm_from_model::value_type
llvm_from_model::generate_buffer_access
( statement *stmt, const vector<value_type> & index )
{
    value_type flat_index = flat_buffer_index(stmt, index);
    value_type stmt_index = value( (int32_t) statement_index(stmt) );
    value_type buffer;
    {
        vector<value_type> indices{stmt_index, value((int32_t) 0)};
        value_type buffer_ptr =
                m_builder.CreateGEP(m_buffers, indices);
        buffer = m_builder.CreateLoad(buffer_ptr);
        type_type real_buffer_type = llvm::Type::getDoublePtrTy(llvm_context());
        buffer = m_builder.CreateBitCast(buffer, real_buffer_type);
    }

    value_type buffer_element_ptr =
            m_builder.CreateGEP(buffer, flat_index);

    return buffer_element_ptr;
}

llvm_from_model::value_type
llvm_from_model::generate_input_access
( statement *stmt, const vector<value_type> & index )
{
    vector<value_type> the_index(index);
    vector<int> the_domain(stmt->domain);
    const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
    if (actor)
    {
        the_domain[actor->flow_dimension] = actor->steady_count;
        transpose(the_index, actor->flow_dimension);
        transpose(the_domain, actor->flow_dimension);
    }

    value_type flat_index = this->flat_index(the_index, the_domain);

    int input_num = reinterpret_cast<input_access*>(stmt->expr)->index;
    value_type input_ptr = m_builder.CreateGEP(m_inputs, value(input_num));
    value_type input = m_builder.CreateLoad(input_ptr);

    type_type double_ptr_type = llvm::Type::getDoublePtrTy(llvm_context());
    input = m_builder.CreateBitCast(input, double_ptr_type);

    value_type value_ptr = m_builder.CreateGEP(input, flat_index);

    return value_ptr;
}

llvm_from_model::value_type
llvm_from_model::generate_reduction_access
( reduction_access *access, const vector<value_type> & index )
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
        value_type init_ptr = generate_buffer_access(access->initializer, init_index);
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
                                                       reductor_index);
        recall_value = m_builder.CreateLoad(recall_ptr);

        m_builder.CreateBr(after_block);
    }

    m_builder.SetInsertPoint(after_block);

    llvm::PHINode *value = m_builder.CreatePHI(double_type(), 2);
    value->addIncoming(init_value, init_block);
    value->addIncoming(recall_value, recall_block);

    return value;
}

void llvm_from_model::advance_buffers()
{
    int buf_idx = -1;
    for (statement * stmt : m_statements)
    {
        ++buf_idx;

        const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);

        if (!actor)
            continue;

        int offset = actor->steady_count;
        int buffer_size = stmt->buffer[actor->flow_dimension];

        if (offset == 0 || offset % buffer_size == 0)
            continue;

        vector<value_type> indices
        {
            value((int32_t) buf_idx),
                    value((int32_t) 1),
        };
        value_type phase_ptr =
                m_builder.CreateGEP(m_buffers, indices);
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
( statement * stmt, const vector<value_type> & index )
{
    // Get basic info about accessed statement

    const dataflow::actor * actor = m_dataflow->find_actor_for(stmt);
    value_type stmt_index = value( (int32_t) statement_index(stmt) );

    //int finite_slice_size = flat_buffer_size(stmt, 1);

    bool use_phase = false;

    // Canonical transposition (flow dimension first);

    vector<value_type> the_index(index);
    vector<int> the_domain(stmt->domain);

    if (actor)
        the_domain[actor->flow_dimension] = actor->steady_count;

    // Add flow index phase

    if (actor)
    {
        bool period_overlaps =
                actor->steady_count > 1 &&
                (actor->steady_count % stmt->buffer[actor->flow_dimension] != 0);
        bool init_overlaps =
                actor->init_count % stmt->buffer[actor->flow_dimension] != 0;

        use_phase = period_overlaps || init_overlaps;

        if (use_phase)
        {
            value_type phase;
            {
                vector<value_type> indices{stmt_index, value((int32_t) 1)};
                value_type phase_ptr =
                        m_builder.CreateGEP(m_buffers, indices);
                phase = m_builder.CreateLoad(phase_ptr);
                if (phase->getType() == int32_type())
                    phase = m_builder.CreateSExt(phase, int64_type());
            }

            value_type & flow_index = the_index[actor->flow_dimension];
            flow_index = m_builder.CreateAdd(flow_index, phase);
        }
    }

    // Wrap index

    for (int dim = 0; dim < the_index.size(); ++dim)
    {
        int domain = stmt->domain[dim];
        int buffer = stmt->buffer[dim];

        bool may_wrap =
                domain > buffer ||
                (actor && dim == actor->flow_dimension && use_phase);

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
        transpose(the_domain, actor->flow_dimension);
    }

    value_type flat_index = this->flat_index(the_index, the_domain);

    return flat_index;
}

int llvm_from_model::flat_buffer_size( statement *stmt, int flow_count )
{
    if (stmt->domain.empty())
        return 0;

    int flat_size = 1;
    for (int dim = 0; dim < stmt->domain.size(); ++dim)
    {
        int size = stmt->domain[dim];
        if (size == -1)
            size = flow_count;

        flat_size *= size;
    }
    return flat_size;
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

#if 0
vector<llvm_from_model::value_type>
llvm_from_model::map_index
( statement *stmt, const vector<value_type> & index )
{

}
#endif

}
}
