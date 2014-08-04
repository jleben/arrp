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
#include <functional>

namespace stream {
namespace IR {

using namespace semantic;
using std::vector;
using std::pair;

struct value
{
    virtual llvm::Value *get( llvm::IRBuilder<> & ) = 0;
};

using value_ptr = std::shared_ptr<value>;

struct scalar_value : public value
{
    scalar_value(llvm::Value *v): v(v) {}
    virtual llvm::Value *get( llvm::IRBuilder<> & ) { return v; }
    llvm::Value *v;
};

struct abstract_stream_value : public value
{
    virtual int dimensions() = 0;
    virtual int size( int dimension ) = 0;
    virtual vector<int> size() = 0;
    virtual llvm::Value *get_at( const vector<value_ptr> & index, llvm::IRBuilder<> & ) = 0;
};

using stream_value_ptr = std::shared_ptr<abstract_stream_value>;

struct stream_value : public abstract_stream_value
{
    int dimensions() { return m_size.size(); }
    int size( int dimension ) { return m_size[dimension]; }
    vector<int> size() { return m_size; }

    stream_value( llvm::Value *data, vector<int> size ):
        m_data(data),
        m_size(size)
    {
        if (size.size() > 1)
        {
            m_index_coeffs = vector<int>(size.size() - 1);
            int coeff = 1;
            int dim = size.size() - 1;
            while(dim > 0)
            {
                coeff *= size[dim];
                m_index_coeffs[dim-1] = coeff;
                --dim;
            }
        }
    }

    virtual llvm::Value *get( llvm::IRBuilder<> & )
    {
        return m_data;
    }

    virtual llvm::Value *get_at( const vector<value_ptr> & index, llvm::IRBuilder<> & builder );

private:
    vector<int> m_size;
    vector<int> m_index_coeffs;
    llvm::Value *m_data;
};

#if 0
struct index_value : public value
{
    index_value( const stream_value_ptr & stream,
                 const vector<value_ptr> & index ):
        stream(stream),
        index(index)
    {}

    virtual llvm::Value *get( llvm::IRBuilder<> & builder )
    {
        return stream->get_at( index, builder );
    }

    stream_value_ptr stream;
    vector<value_ptr> index;
};
#endif

struct slice_value : public abstract_stream_value
{
    slice_value( const stream_value_ptr & stream,
                 const vector<value_ptr> & offset,
                 const vector<int> & size ):
        m_source(stream)
    {
        assert(size.size() <= stream->dimensions());
        int dim = 0;
        while(dim < size.size() && size[dim] == 1)
        {
            m_preoffset.push_back(offset[dim]);
            ++dim;
        }
        while(dim < size.size())
        {
            m_offset.push_back(offset[dim]);
            m_size.push_back(size.size());
            ++dim;
        }
        while(dim < stream->dimensions())
        {
            m_size.push_back(stream->size(dim));
            ++dim;
        }
    }

    virtual llvm::Value *get( llvm::IRBuilder<> & builder )
    {
        return m_source->get(builder);
    }

    virtual llvm::Value *get_at( const vector<value_ptr> & index,
                                 llvm::IRBuilder<> & builder );

    int dimensions() { return m_size.size(); }
    int size( int dimension ) { return m_size[dimension]; }
    vector<int> size() { return m_size; }

private:
    stream_value_ptr m_source;
    vector<value_ptr> m_preoffset;
    vector<value_ptr> m_offset;
    vector<int> m_size;
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
    //value_ptr process_range( const ast::node_ptr & );
    //value_ptr process_extent( const ast::node_ptr & );
    //value_ptr process_transpose( const ast::node_ptr & );
    value_ptr process_slice( const ast::node_ptr & );
    //value_ptr process_iteration( const ast::node_ptr & );
    //iterator process_iterator( const ast::node_ptr & );
    //value_ptr process_reduction( const ast::node_ptr & );

    void generate_iteration( const value_ptr & from,
                             const value_ptr & to,
                             std::function<void(const value_ptr &)> );

    stream_value_ptr allocate_stream( const vector<int> & size );

    llvm::LLVMContext & llvm_context() { return m_module->getContext(); };

    llvm::Module *m_module;
    environment & m_env;
    context m_ctx;
    llvm::IRBuilder<> m_builder;

    struct {
        llvm::BasicBlock *block;
    } m_allocator;
};

}
}

#endif // M_LANG_IR_GENERATOR_INCLUDED
