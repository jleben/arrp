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
    cout << "Would generate statement: " << stmt->name << endl;

    if (!stmt->expr)
        return;

    // Drop first dimension denoting period (always zero).
    assert(!raw_index.empty());
    vector<value_type> index(raw_index.begin()+1, raw_index.end());

    m_builder.SetInsertPoint(block);

    value_type value = generate_expression(stmt->expr, index);

    // TODO: write value
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
        value_type address = generate_access(read, index);
        //return m_builder.CreateLoad(address);
        return value(999.0);
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
( stream_access *stmt, const vector<value_type> & index )
{
    assert(index.size() == stmt->pattern.input_dimension());

    vector<value_type> target_index;
    target_index.reserve(stmt->pattern.output_dimension());

    for(int out_dim = 0; out_dim < stmt->pattern.output_dimension(); ++out_dim)
    {
        value_type out_value = value((int64_t) stmt->pattern.constant(out_dim));
        for (int in_dim = 0; in_dim < stmt->pattern.input_dimension(); ++in_dim)
        {
            value_type coefficient =
                    value((int64_t) stmt->pattern.coefficient(in_dim, out_dim));
            value_type term = m_builder.CreateMul(index[in_dim], coefficient);
            out_value = m_builder.CreateAdd(out_value, term);
        }
        target_index.push_back(out_value);
    }

    value_type flat_target_index = flat_index(target_index, stmt->target->domain);

    // TODO: Access buffer
    //return m_builder.CreateGEP(buffer, offset_and_wrapped_index);
    cout << "Would generate access: "
         << stmt->target->name
         << endl;

    return nullptr;
}

llvm_from_model::value_type
llvm_from_model::flat_index
( const vector<value_type> & index, const vector<int> & size )
{
    assert(index.size() > 0);
    assert(size.size() == index.size());
    value_type flat_index = index[0];
    for( unsigned int i = 1; i < index.size(); ++i)
    {
        flat_index = m_builder.CreateMul(flat_index, value((int64_t) size[i]));
        flat_index = m_builder.CreateAdd(flat_index, index[i]);
    }
    return flat_index;
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
