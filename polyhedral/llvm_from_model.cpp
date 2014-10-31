#include "llvm_from_model.hpp"

#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

namespace stream {
namespace polyhedral {

llvm_from_model::llvm_from_model
(llvm::Module *module,
 const vector<statement*> & statements,
 const vector<dataflow_dependency> & dependencies):
    m_module(module),
    m_builder(module->getContext()),
    m_statements(statements),
    m_dependencies(dependencies)
{

    type_type i8_ptr_type = llvm::Type::getInt8PtrTy(llvm_context());
    type_type buffer_type =
            llvm::StructType::create(vector<type_type>({i8_ptr_type, int32_type()}));
    type_type buffer_pointer_type = llvm::PointerType::get(buffer_type, 0);

    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    buffer_pointer_type,
                                    false);

    m_function = llvm::Function::Create(func_type,
                                        llvm::Function::ExternalLinkage,
                                        "process",
                                        m_module);

    //llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm_context(), "entry", func);
    //m_builder.SetInsertPoint(bb);
}

void llvm_from_model::generate_statement( const string & name,
                                          const vector<value_type> & index,
                                          block_type block )
{
    auto stmt_ref = std::find_if(m_statements.begin(), m_statements.end(),
                                 [&](statement *s){ return s->name == name; });
    assert(stmt_ref != m_statements.end());
    generate_statement(*stmt_ref, index, block);
}

void llvm_from_model::generate_statement
( statement *stmt, const vector<value_type> & raw_index, block_type block )
{
    if (!stmt->expr)
    {
        cout << "Not generating statement without expression: " << stmt->name << endl;
        return;
    }

    // Drop first dimension denoting period (always zero).
    assert(!raw_index.empty());
    vector<value_type> index(raw_index.begin()+1, raw_index.end());

    m_builder.SetInsertPoint(block);

    value_type value = generate_expression(stmt->expr, index);

    value_type dst = generate_access(stmt, index);

    m_builder.CreateStore(value, dst);
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
        value_type address = generate_access(read->target, target_index);
        return m_builder.CreateLoad(address);
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
llvm_from_model::generate_access
( statement *stmt, const vector<value_type> & index )
{
    // Get basic info about accessed statement

    int finite_slice_size = 1;
    for (int dim = 0; dim < stmt->domain.size(); ++dim)
    {
        int size = stmt->domain[dim];
        if (size >= 0)
            finite_slice_size *= size;
    }

    assert(stmt->buffer_size >= 0);
    int buffer_size = finite_slice_size * stmt->buffer_size;

    value_type stmt_index = value( (int32_t) statement_index(stmt) );

    // Compute flat access index

    value_type flat_index = this->flat_index(index, stmt);

    // Get buffer state

    value_type buffer_data = m_function->arg_begin();
    value_type buffer, phase;

    {
        vector<value_type> indices{stmt_index, value((int32_t) 0)};
        value_type buffer_ptr =
                m_builder.CreateGEP(buffer_data, indices);
        buffer = m_builder.CreateLoad(buffer_ptr);
        type_type real_buffer_type = llvm::Type::getDoublePtrTy(llvm_context());
        buffer = m_builder.CreateBitCast(buffer, real_buffer_type);
    }
    {
        vector<value_type> indices{stmt_index, value((int32_t) 1)};
        value_type phase_ptr =
                m_builder.CreateGEP(buffer_data, indices);
        phase = m_builder.CreateLoad(phase_ptr);
        if (phase->getType() == int32_type())
            phase = m_builder.CreateSExt(phase, int64_type());
    }

    // Add access index to current buffer state (phase)

    phase = m_builder.CreateMul(phase, value((int64_t)finite_slice_size));

    value_type buffer_index =
            m_builder.CreateAdd(flat_index, phase);

    buffer_index =
            m_builder.CreateSRem(buffer_index, value((int64_t)buffer_size));

    // Get value from buffer

    value_type value_ptr =
            m_builder.CreateGEP(buffer, buffer_index);

    return value_ptr;
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
( const vector<value_type> & index, statement *stmt )
{
    assert(index.size() > 0);
    assert(stmt->domain.size() == index.size());

    int major_dim = stmt->dimension;
    assert(major_dim >= 0 && major_dim < stmt->domain.size());

    value_type flat_index = index[major_dim];
    for( unsigned int i = 0; i < index.size(); ++i)
    {
        if (i == major_dim)
            continue;
        int size = stmt->domain[i];
        flat_index = m_builder.CreateMul(flat_index, value((int64_t) size));
        flat_index = m_builder.CreateAdd(flat_index, index[i]);
    }

    return flat_index;
}

int llvm_from_model::statement_index( statement * stmt )
{
    auto stmt_ref = std::find(m_statements.begin(), m_statements.end(), stmt);
    assert(stmt_ref != m_statements.end());
    return (int) std::distance(stmt_ref, m_statements.begin());
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
