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

#ifndef STREAM_POLYHEDRAL_LLVM_IR_CLOOG_TRANSLATOR_INCLUDED
#define STREAM_POLYHEDRAL_LLVM_IR_CLOOG_TRANSLATOR_INCLUDED

#include "model.hpp"
#include "../frontend/context.hpp"

#include <cloog/cloog.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>

namespace stream {
namespace polyhedral {

using std::unordered_map;
using std::string;
using std::vector;

class llvm_from_cloog
{
    using value_type = llvm::Value*;
    using type_type = llvm::Type*;
    using block_type = llvm::BasicBlock*;
    using context = stream::context<string, value_type>;

public:
    llvm_from_cloog(llvm::Module *);

    void generate( clast_stmt *root,
                   block_type start_block,
                   block_type end_block );

    bool verify();

    void output( std::ostream & );

    llvm::Module *module() { return m_module; }

    template<typename F>
    void set_stmt_func(F f)
    {
        m_stmt_func = f;
    }

private:
    void process_list( clast_stmt * );
    void process( clast_stmt * );
    void process( clast_root* );
    void process( clast_block* );
    void process( clast_assignment* );
    void process( clast_guard* );
    void process( clast_equation*,
                  block_type true_block,
                  block_type false_block );
    void process( clast_for* );
    value_type process( clast_expr* );
    void process( clast_user_stmt* );

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

#if 0
    template <typename T> type_type type_for();

    template<> type_type type_for<mpz_t>()
    {

    }
#endif

    value_type value( bool value )
    {
        return llvm::ConstantInt::get(bool_type(), value ? 1 : 0);
    }
#if 0
    value_type value( std::int32_t v )
    {
        return llvm::ConstantInt::getSigned(int32_type(), v);
    }
    value_type value( std::uint32_t v )
    {
        return llvm::ConstantInt::get(int32_type(), v);
    }
#endif
    value_type value( std::int64_t v )
    {
      return llvm::ConstantInt::getSigned(int64_type(), v);
    }
    value_type value( std::uint64_t v )
    {
      return llvm::ConstantInt::get(int64_type(), v);
    }
#ifdef CLOOG_INT_GMP
    value_type value( mpz_t v )
    {
        return value( mpz_get_si(v) );
    }

#endif
    value_type value( double value )
    {
        return llvm::ConstantFP::get(double_type(), value);
    }

    block_type add_block( const string & name )
    {
        auto parent = m_builder.GetInsertBlock()->getParent();
        return llvm::BasicBlock::Create(llvm_context(), name, parent);
    }

    llvm::LLVMContext & llvm_context() { return m_module->getContext(); }

    llvm::Module *m_module;
    llvm::IRBuilder<> m_builder;
    context m_ctx;
    std::function<block_type(const string&,
                             const vector<value_type>&,
                             block_type)> m_stmt_func;
};

}
}

#endif // STREAM_POLYHEDRAL_LLVM_IR_CLOOG_TRANSLATOR_INCLUDED
