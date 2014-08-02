#ifndef STREAM_LANG_IR_GENERATOR_INCLUDED
#define STTREM_LANG_IR_GENERATOR_INCLUDED

#include "context.hpp"
#include "environment.hpp"
#include "types.hpp"
#include "ast.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include <memory>
#include <vector>
#include <cassert>
#include <utility>

namespace stream {
namespace IR {

using namespace semantic;
using std::vector;
using std::pair;

struct value
{
    virtual llvm::Value *get() = 0;
    virtual llvm::Value *getAt( const vector<int> & index ) = 0;
};

using value_ptr = std::shared_ptr<value>;

struct scalar_value : public value
{
    scalar_value(llvm::Value *v): v(v) {}
    virtual llvm::Value *get() { return v; }
    virtual llvm::Value *getAt( const vector<int> & index ) { assert(false); }
    llvm::Value *v;
};

struct value_item;
struct function_item;

struct context_item
{
    enum type_flag
    {
        function,
        value
    };

    virtual type_flag type() = 0;

    value_item *as_value()
    {
        assert(type() == value);
        return reinterpret_cast<value_item*>(this);
    }

    function_item *as_function()
    {
        assert(type() == function);
        return reinterpret_cast<function_item*>(this);
    }
};

using context_item_ptr = std::shared_ptr<context_item>;

struct value_item : public context_item
{
    value_item() {}
    value_item( const value_ptr & val ) : v(val) {}
    virtual type_flag type() { return value; }
    virtual value_ptr get_value()
    {
        return v;
    }
    value_ptr v;
};

struct function_item : public context_item
{
    virtual type_flag type() { return function; }
    virtual value_ptr get_value_for( const vector<value_ptr> & args ) = 0;
};

struct builtin_unary_func_item : public function_item
{
    //value_ptr get_value_for( const vector<value_ptr> & args );
};

struct builtin_binary_func_item : public function_item
{
    //value_ptr get_value_for( const vector<value_ptr> & args );
};

struct user_func_item : public function_item
{
    value_ptr get_value_for( const vector<value_ptr> & args )
    {
        assert(false);
        return value_ptr();
    }

    string name;
    vector<string> parameter_names;
    ast::node_ptr expression;
};

class generator
{
public:
    generator(llvm::Module *, environment &env);

    void generate( const symbol & sym,
                   const type_ptr & result,
                   const vector<type_ptr> & args );

private:
    typedef ::stream::context<string, context_item_ptr> context;

    llvm::Type *llvm_type( const type_ptr & type );
    context_item_ptr item_for_symbol( const symbol & sym );
    value_ptr value_for_function( function_item *,
                                  const vector<value_ptr> & args,
                                  context::scope_iterator scope);

#if 0
    type_ptr process_function( const type_ptr & func,
                               const vector<type_ptr> & args,
                               context::scope_iterator scope );
#endif
    value_ptr process_block( const ast::node_ptr & );
    void process_stmt_list( const ast::node_ptr & );
    void process_stmt( const ast::node_ptr & );
    value_ptr process_expression( const ast::node_ptr & );
    pair<value_ptr, context::scope_iterator>
    process_identifier( const ast::node_ptr & );
    value_ptr process_call( const ast::node_ptr & );
    value_ptr process_binop( const ast::node_ptr & );
#if 0
    value_ptr process_range( const ast::node_ptr & );
    value_ptr process_extent( const ast::node_ptr & );
    value_ptr process_transpose( const ast::node_ptr & );
    value_ptr process_slice( const ast::node_ptr & );
    value_ptr process_iteration( const ast::node_ptr & );
    iterator process_iterator( const ast::node_ptr & );
    value_ptr process_reduction( const ast::node_ptr & );
#endif

    llvm::LLVMContext & llvm_context() { return m_module->getContext(); }

    llvm::Module *m_module;
    environment & m_env;
    context m_ctx;
    llvm::IRBuilder<> m_builder;
};

}
}

#endif // M_LANG_IR_GENERATOR_INCLUDED
