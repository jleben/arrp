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

#ifndef STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED
#define STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED

#include "../common/types.hpp"
#include "../common/polyhedral_model.hpp"
#include "../common/dataflow_model.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <vector>

namespace stream {
namespace llvm_gen {

using namespace polyhedral;
using std::vector;

enum schedule_type
{
    initial_schedule,
    periodic_schedule
};

class llvm_from_polyhedral
{
    using value_type = llvm::Value*;
    using type_type = llvm::Type*;
    using block_type = llvm::BasicBlock*;

public:
    struct options
    {
        options():
            max_stack_size(4096)
        {}
        int max_stack_size;
    };

    struct context
    {
        context(schedule_type mode): mode(mode) {}

        schedule_type mode;
        llvm::Function *func;
        block_type start_block;
        block_type end_block;
        vector<value_type> inputs;
        vector<value_type> mem_buffers;
        vector<value_type> stack_buffers;
    };

    typedef vector<value_type> index_type;

    llvm_from_polyhedral(llvm::Module *module,
                    const vector<statement*> &,
                    const dataflow::model *,
                    const options & = options());

    context create_process_function(schedule_type mode,
                                    const vector<semantic::type_ptr> & args);

    block_type generate_statement( const string & name,
                                   const index_type &,
                                   const context & ctx,
                                   block_type block );
    block_type generate_statement( statement *,
                                   const index_type &,
                                   const context & ctx,
                                   block_type block );

private:

    struct buffer
    {
        primitive_type type;
        bool has_phase;
        int phase_index;
        int size;
        vector<int> domain;
        bool on_stack;
        int index;
    };

    enum buffer_kind
    {
        bool_buffer = 0,
        int_buffer,
        real_buffer,
        phase_buffer,
        buffer_kind_count
    };

    buffer_kind buffer_kind_for(primitive_type t)
    {
        switch(t)
        {
        case primitive_type::boolean:
            return bool_buffer;
        case primitive_type::integer:
            return int_buffer;
        case primitive_type::real:
            return real_buffer;
        }
        throw false;
    }

    value_type generate_expression( expression *, const index_type &, const context & );
    value_type generate_primitive( primitive_expr *, const index_type &, const context &  );
    value_type generate_buffer_access( statement *, const index_type &, const context &  );
    value_type generate_input_access( statement *, const index_type &, const context &  );
    value_type generate_scalar_input_access( input_access *, const context & );
    value_type generate_reduction_access( reduction_access *, const index_type &, const context & );
    void advance_buffers(const context & );

    template <typename T>
    void transpose( vector<T> & index, int first_dim );

    vector<value_type> mapped_index( const vector<value_type> & index,
                                     const mapping & );

    vector<value_type> buffer_index( statement * stmt,
                                     const index_type &,
                                     const context & );
    value_type flat_buffer_index( statement * stmt,
                                  const index_type &,
                                  const context & );

    value_type load_buffer_elem(value_type ptr, primitive_type);
    void store_buffer_elem(value_type val, value_type ptr, primitive_type);

    int buffer_elem_size(primitive_type t);
    type_type buffer_elem_type(primitive_type t);
    type_type buffer_type(const buffer &);
    type_type buffer_ptr_type(const buffer &);
    type_type array_type(type_type elem_type, const vector<int> domain);

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
    type_type pointer(type_type t)
    {
        return llvm::PointerType::get(t, 0);
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

    type_type type(primitive_type);
    value_type convert( value_type v, primitive_type );
    value_type convert_to_real( value_type v );
    value_type convert_to_integer( value_type v );
    value_type convert_to_boolean( value_type v );

    value_type floor( value_type );

    block_type add_block( const string & name )
    {
        auto parent = m_builder.GetInsertBlock()->getParent();
        return llvm::BasicBlock::Create(llvm_context(), name, parent);
    }

    value_type malloc( type_type t, std::uint64_t size );

    llvm::LLVMContext & llvm_context() { return m_module->getContext(); }

    llvm::Module *m_module;
    llvm::IRBuilder<> m_builder;
    const vector<statement*> & m_statements;
    const dataflow::model * m_dataflow;
    vector<buffer> m_stmt_buffers;
    type_type m_buffer_struct_type;
};

}
}

#endif // STREAM_POLYHEDRAL_LLVM_FROM_MODEL_INCLUDED
