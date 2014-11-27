#ifndef STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED
#define STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED

#include "model.hpp"
#include "dataflow_model.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <vector>

namespace stream {
namespace polyhedral {

using std::vector;

class llvm_from_model
{
    using value_type = llvm::Value*;
    using type_type = llvm::Type*;
    using block_type = llvm::BasicBlock*;

public:
    llvm_from_model(llvm::Module *module,
                    int input_count,
                    const vector<statement*> &,
                    const dataflow::model * );

    llvm::Function * function() { return m_function; }
    block_type start_block() { return m_start_block; }
    block_type end_block() { return m_end_block; }

    block_type generate_statement( const string & name,
                                   const vector<value_type> & index,
                                   block_type block );
    block_type generate_statement( statement *,
                                   const vector<value_type> & index,
                                   block_type block );
private:

    value_type generate_expression( expression *, const vector<value_type> & index );
    value_type generate_intrinsic( intrinsic *, const vector<value_type> & index );
    value_type generate_buffer_access( statement *, const vector<value_type> & index );
    value_type generate_input_access( statement *, const vector<value_type> & index );
    value_type generate_reduction_access( reduction_access *, const vector<value_type> & index );
    void advance_buffers();

    //int flat_buffer_size( statement *, int flow_count );

    template <typename T>
    void transpose( vector<T> & index, int first_dim );

    vector<value_type> mapped_index( const vector<value_type> & index,
                                     const mapping & );

    value_type flat_buffer_index( statement * stmt, const vector<value_type> & index );

    value_type flat_index( const vector<value_type> & index,
                           const vector<int> & domain );

    int statement_index( statement * stmt );

    type_type bool_type()
    {
        return llvm::Type::getInt1Ty(llvm_context());
    }
    type_type int32_type()
    {
        return llvm::Type::getInt32Ty(llvm_context());
    }
    type_type int64_type()
    {
      return llvm::Type::getInt64Ty(llvm_context());
    }
    type_type double_type()
    {
        return llvm::Type::getDoubleTy(llvm_context());
    }

    value_type value( bool value )
    {
        return llvm::ConstantInt::get(bool_type(), value ? 1 : 0);
    }
    value_type value( std::int32_t v )
    {
        return llvm::ConstantInt::getSigned(int32_type(), v);
    }
    value_type value( std::uint32_t v )
    {
        return llvm::ConstantInt::get(int32_type(), v);
    }
    value_type value( std::int64_t v )
    {
      return llvm::ConstantInt::getSigned(int64_type(), v);
    }
    value_type value( std::uint64_t v )
    {
      return llvm::ConstantInt::get(int64_type(), v);
    }
    value_type value( double value )
    {
        return llvm::ConstantFP::get(double_type(), value);
    }

    value_type value( type_type t, int32_t v )
    {
        return llvm::ConstantInt::getSigned(t,v);
    }
    value_type value( type_type t, uint32_t v )
    {
        return llvm::ConstantInt::get(t,v);
    }

    block_type add_block( const string & name )
    {
        auto parent = m_builder.GetInsertBlock()->getParent();
        llvm::BasicBlock::Create(llvm_context(), name, parent);
    }

    llvm::LLVMContext & llvm_context() { return m_module->getContext(); }

    llvm::Module *m_module;
    llvm::Function *m_function;
    llvm::IRBuilder<> m_builder;
    const vector<statement*> & m_statements;
    const dataflow::model * m_dataflow;
    value_type m_inputs;
    value_type m_output;
    value_type m_buffers;
    block_type m_start_block;
    block_type m_end_block;
};

}
}

#endif // STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED
