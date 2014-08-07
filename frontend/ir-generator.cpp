#include "ir-generator.hpp"
#include "error.hpp"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/Analysis/Verifier.h>
#include <algorithm>

using namespace std;

namespace stream {
namespace IR {

generator::generator(llvm::Module *module, environment &env):
    m_module(module),
    m_env(env),
    m_buffer_pool(nullptr),
    m_buffer_pool_size(0),
    m_builder(module->getContext())
{}

void generator::generate( const symbol & sym,
                          const vector<type_ptr> & arg_types )
{
    const type_ptr & result_type = sym.source_expression()->semantic_type;

    // Create function

    llvm::Type *i8_ptr_ptr =
            llvm::PointerType::get(llvm::Type::getInt8PtrTy(llvm_context()), 0);


    llvm::FunctionType * func_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(llvm_context()),
                                    i8_ptr_ptr,
                                    false);

    llvm::Function *func =
            llvm::Function::Create(func_type,
                                   llvm::Function::ExternalLinkage,
                                   "process",
                                   m_module);


    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm_context(), "entry", func);
    m_builder.SetInsertPoint(bb);

    // Extract args

    llvm::Value *args = func->arg_begin();

    vector<value_ptr> inputs;
    for (int idx = 0; idx < arg_types.size(); ++idx)
    {
        inputs.push_back(value_for_argument(args, idx, arg_types[idx], true));
    }

    // Extract output destination:

    value_ptr output =
            value_for_argument(args, arg_types.size(), result_type, false);

    // Set up buffer pool:

    llvm::Value *buffer_pool_arg =
            m_builder.CreateGEP(args, get_int32(arg_types.size() + 1));
    llvm::Value *buffer_pool_void_ptr =
            m_builder.CreateLoad(buffer_pool_arg);
    m_buffer_pool =
            m_builder.CreateBitCast(buffer_pool_void_ptr,
                                    llvm::Type::getDoublePtrTy(llvm_context()));
    m_buffer_pool_size = 0;

    // Generate

    context::scope_holder scope(m_ctx);

    context_item_ptr item = item_for_symbol(sym, output);

    if (inputs.size())
    {
        if (item->type() != context_item::function)
            throw error("Should not happen.");
        function_item *func_item = item->as_function();
        value_ptr result = value_for_function(func_item, inputs, output, m_ctx.root_scope());
    }

    m_builder.CreateRetVoid();

    cout << "buffer pool size = " << m_buffer_pool_size << endl;
}

value_ptr generator::value_for_argument( llvm::Value *args, int index,
                                         const type_ptr & arg_type,
                                         bool load_scalar )
{
    llvm::Value *arg_ptr_ptr =
            m_builder.CreateGEP(args, get_int32(index));
    llvm::Value *arg_ptr = m_builder.CreateLoad(arg_ptr_ptr);

    switch(arg_type->get_tag())
    {
    case type::integer_num:
    {
        llvm::Value *val_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getInt32PtrTy(llvm_context()));
        if (load_scalar)
        {
            llvm::Value *val =
                    m_builder.CreateLoad(val_ptr);
            return make_shared<scalar_value>(val);
        }
        else
            return make_shared<scalar_value>(val_ptr);
    }
    case type::real_num:
    {
        llvm::Value *val_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getDoublePtrTy(llvm_context()));
        if (load_scalar)
        {
            llvm::Value *val =
                    m_builder.CreateLoad(val_ptr);
            return make_shared<scalar_value>(val);
        }
        else
            return make_shared<scalar_value>(val_ptr);
    }
    case type::stream:
    {
        vector<int> & stream_size = arg_type->as<stream>().size;
        llvm::Value *data_ptr =
                m_builder.CreateBitCast(arg_ptr, llvm::Type::getDoublePtrTy(llvm_context()));
        return make_shared<stream_value>(data_ptr, stream_size);
    }
    default:
        assert(false);
    }
    return value_ptr();
}

llvm::Type *generator::llvm_type( const type_ptr & t )
{
    using namespace semantic;

    switch(t->get_tag())
    {
    case type::integer_num:
        return llvm::Type::getInt32Ty(llvm_context());
    case type::real_num:
        return llvm::Type::getDoubleTy(llvm_context());
    case type::stream:
        return llvm::PointerType::get( llvm::Type::getDoubleTy(llvm_context()), 0 );
    }

    // TODO
    assert(false);
    return nullptr;
}

context_item_ptr generator::item_for_symbol( const symbol & sym,
                                             const value_ptr & result_space )
{
    switch(sym.type)
    {
    case symbol::expression:
    {
        context::scope_holder(m_ctx, m_ctx.root_scope());
        value_ptr val = process_block(sym.source_expression(), result_space);
        return make_shared<value_item>(val);
    }
    case symbol::function:
    {
        user_func_item *item = new user_func_item;
        item->f = dynamic_pointer_cast<semantic::function>(sym.source->semantic_type);
        if (!item->f)
            throw error("baaad.");
        assert(item->f);
        return context_item_ptr(item);
    }
#if 0
    case symbol::builtin_unary_math:
        return make_shared<builtin_unary_func_item>();
    case symbol::builtin_binary_math:
        return make_shared<builtin_binary_func_item>();
#endif
    }
    assert(false);
    throw error("Should not happen.");
}

value_ptr generator::value_for_function(function_item *func,
                                        const vector<value_ptr> & args,
                                        const value_ptr & result_space,
                                        context::scope_iterator scope)
{
    if (user_func_item *user_func = dynamic_cast<user_func_item*>(func))
    {
        function & f = *user_func->f;
        assert(f.parameters.size() == args.size());

        context::scope_holder func_scope(m_ctx, scope);
        for (int i = 0; i < args.size(); ++i)
            m_ctx.bind(f.parameters[i],
                       make_shared<value_item>(args[i]));

        return process_block(f.expression(), result_space);
    }

    assert(false);
#if 0
    switch (func_type->get_tag())
    {
    case type::builtin_unary_func:
    {
        if (args.size() != 1)
            throw wrong_arg_count_error(1, args.size());
        return builtin_unary_func_type(args[0]);
    }
    case type::builtin_binary_func:
    {
        if (args.size() != 2)
            throw wrong_arg_count_error(2, args.size());
        return builtin_binary_func_type(args[0], args[1]);
    }
    case type::function:
    {
        function &f = func_type->as<function>();
        if (args.size() != f.parameters.size())
            throw wrong_arg_count_error(f.parameters.size(), args.size());

        context_type::scope_holder func_scope(m_ctx, scope);

        for (int i = 0; i < args.size(); ++i)
        {
            m_ctx.bind(f.parameters[i], args[i]);
        }

        return process_block(f.expression);
    }
    default:
        throw type_error("Callee not a function.");
    }
#endif
}

value_ptr generator::process_block( const ast::node_ptr & root,
                                    const value_ptr & result_space )
{
    assert(root->type == ast::expression_block);
    ast::list_node *expr_block = root->as_list();
    assert(expr_block->elements.size() == 2);
    const auto & stmts = expr_block->elements[0];
    const auto & expr = expr_block->elements[1];

    if (stmts)
        process_stmt_list( stmts );

    return process_expression( expr, result_space );
}

void generator::process_stmt_list( const ast::node_ptr & root )
{
    assert(root->type == ast::statement_list || root->type == ast::program);
    ast::list_node *stmts = root->as_list();
    for ( const ast::node_ptr & stmt : stmts->elements )
    {
        process_stmt(stmt);
    }
}

void generator::process_stmt( const ast::node_ptr & root )
{
    ast::list_node *stmt = root->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & expr_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    context_item_ptr ctx_item;

    if (params_node)
    {
        vector<string> parameters;
        ast::list_node *param_list = params_node->as_list();
        for ( const auto & param : param_list->elements )
        {
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);
        }

        user_func_item *item = new user_func_item;
        item->f = dynamic_pointer_cast<function>(root->semantic_type);
        ctx_item = context_item_ptr(item);
    }
    else
    {
        value_ptr result = process_block(expr_node);
        ctx_item = make_shared<value_item>(result);
    }

    m_ctx.bind(id, ctx_item);
}

value_ptr generator::process_expression( const ast::node_ptr & root, const value_ptr & result_space )
{
    //result_stacker result_spacer(m_result_stack, result_space);

    value_ptr result;

    switch(root->type)
    {
    case ast::integer_num:
    {
        int i = root->as_leaf<int>()->value;
        result = make_shared<scalar_value>(get_int32(i));
        break;
    }
    case ast::real_num:
    {
        double d = root->as_leaf<double>()->value;
        llvm::Value * v = llvm::ConstantFP::get(llvm_context(), llvm::APFloat(d));
        result = make_shared<scalar_value>(v);
        break;
    }
    case ast::identifier:;
    {
        result = process_identifier(root).first;
        break;
    }
    case ast::call_expression:
        return process_call(root, result_space);
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
        return process_binop(root, result_space);
#if 0
    case ast::range:
        return process_range(root);
    case ast::hash_expression:
        return process_extent(root);
#endif
    case ast::transpose_expression:
        return process_transpose(root);
    case ast::slice_expression:
        result = process_slice(root);
        break;
    case ast::for_expression:
        return process_iteration(root, result_space);
    case ast::reduce_expression:
        return process_reduction(root, result_space);
    default:
        throw source_error("Unsupported expression.", root->line);
    }

    if (result_space)
        generate_store(result_space, result);

    return result;
}

pair<value_ptr, generator::context::scope_iterator>
generator::process_identifier( const ast::node_ptr & root )
{
    string id = root->as_leaf<string>()->value;
    if (context::item ctx_entry = m_ctx.find(id))
    {
        context_item_ptr ctx_item = ctx_entry.value();
        value_item *val_item = ctx_item->as_value();
        return make_pair(val_item->get_value(), ctx_entry.scope());
    }
    else
    {
        environment::const_iterator it = m_env.find(id);
        if (it != m_env.end())
        {
            context_item_ptr ctx_item = item_for_symbol(it->second);
            m_ctx.root_scope()->emplace(id, ctx_item);

            value_item *val_item = ctx_item->as_value();
            return make_pair(val_item->get_value(), m_ctx.root_scope());
        }
    }
    assert(false);
    throw source_error("Name not in scope.", root->line);
}

value_ptr generator::process_call( const ast::node_ptr & root, const value_ptr & result_space )
{
    assert(root->type == ast::call_expression);

    ast::list_node * call = root->as_list();
    const auto & func_node = call->elements[0];
    const auto & args_node = call->elements[1];

    // Get function

    context_item_ptr ctx_item;
    context::scope_iterator scope;

    string id = func_node->as_leaf<string>()->value;
    if (context::item ctx_entry = m_ctx.find(id))
    {
        ctx_item = ctx_entry.value();
        scope = ctx_entry.scope();
    }
    else
    {
        environment::const_iterator it = m_env.find(id);
        assert(it != m_env.end());
        ctx_item = item_for_symbol(it->second);
        scope = m_ctx.root_scope();
    }

    // Get args

    std::vector<value_ptr> args;
    ast::list_node * arg_list = args_node->as_list();
    for (const auto & arg_node : arg_list->elements)
    {
        args.push_back( process_expression(arg_node) );
    }

    // Process function

    return value_for_function(ctx_item->as_function(), args, result_space, scope);
}

value_ptr generator::process_binop( const ast::node_ptr & root,
                                    const value_ptr & result_space  )
{
    ast::list_node *expr = root->as_list();
    value_ptr lhs = process_expression(expr->elements[0]);
    value_ptr rhs = process_expression(expr->elements[1]);

    llvm::Type *fp_type = llvm::Type::getDoubleTy(llvm_context());
    llvm::Type *int_type = llvm::Type::getInt32Ty(llvm_context());
    if ( typeid(*lhs) == typeid(scalar_value) &&
         typeid(*rhs) == typeid(scalar_value) )
    {
        llvm::Value *lhs_ir = static_cast<scalar_value&>(*lhs).get(m_builder);
        llvm::Value *rhs_ir = static_cast<scalar_value&>(*rhs).get(m_builder);
        llvm::Value *result_ir;

        bool fp_op = lhs_ir->getType() == fp_type || rhs_ir->getType() == fp_type;
        if (fp_op)
        {
            if (lhs_ir->getType() != fp_type)
                lhs_ir = m_builder.CreateSIToFP(lhs_ir, fp_type);
            if (rhs_ir->getType() != fp_type)
                rhs_ir = m_builder.CreateSIToFP(rhs_ir, fp_type);
            switch(root->type)
            {
            case ast::add:
                result_ir = m_builder.CreateFAdd(lhs_ir, rhs_ir); break;
            case ast::subtract:
                result_ir = m_builder.CreateFSub(lhs_ir, rhs_ir); break;
            case ast::multiply:
                result_ir = m_builder.CreateFMul(lhs_ir, rhs_ir); break;
            case ast::divide:
                result_ir = m_builder.CreateFDiv(lhs_ir, rhs_ir); break;
            default:
                assert(false);
            }
        }
        else
        {
            switch(root->type)
            {
            case ast::add:
                result_ir = m_builder.CreateAdd(lhs_ir, rhs_ir); break;
            case ast::subtract:
                result_ir = m_builder.CreateSub(lhs_ir, rhs_ir); break;
            case ast::multiply:
                result_ir = m_builder.CreateMul(lhs_ir, rhs_ir); break;
            case ast::divide:
                result_ir = m_builder.CreateSDiv(lhs_ir, rhs_ir); break;
            default:
                assert(false);
            }
        }

        value_ptr result = make_shared<scalar_value>(result_ir);
        if (result_space)
            generate_store(result_space, result);
        return result;
    }
    else
    {
        abstract_stream_value *lhs_stream =
                dynamic_cast<abstract_stream_value*>(lhs.get());
        abstract_stream_value *rhs_stream =
                dynamic_cast<abstract_stream_value*>(rhs.get());

        if (!lhs_stream && !rhs_stream)
            throw error("Binary operator: Unsupported operand.");

        stream_value_ptr result_stream =
                dynamic_pointer_cast<abstract_stream_value>(result_space);
        if (!result_stream)
        {
            if(lhs_stream)
                result_stream = allocate_stream(lhs_stream->size());
            else
                result_stream = allocate_stream(rhs_stream->size());
        }

        if (!lhs_stream)
        {
            llvm::Value *lhs_val = lhs->get(m_builder);
            if (lhs_val->getType() != fp_type)
                lhs_val = m_builder.CreateSIToFP(lhs_val, fp_type);
            lhs = make_shared<scalar_value>(lhs_val);
        }
        if (!rhs_stream)
        {
            llvm::Value *rhs_val = rhs->get(m_builder);
            if (rhs_val->getType() != fp_type)
                rhs_val = m_builder.CreateSIToFP(rhs_val, fp_type);
            rhs = make_shared<scalar_value>(rhs_val);
        }

        auto operation = [&]( const vector<value_ptr> & stream_index )
        {
            llvm::Value *lhs_val, *rhs_val;

            if (lhs_stream)
                lhs_val = m_builder.CreateLoad( lhs_stream->get_at(stream_index, m_builder) );
            else
                lhs_val = lhs->get(m_builder);

            if (rhs_stream)
                rhs_val = m_builder.CreateLoad( rhs_stream->get_at(stream_index, m_builder) );
            else
                rhs_val = rhs->get(m_builder);

            llvm::Value * result_val;

            switch(root->type)
            {
            case ast::add:
                result_val = m_builder.CreateFAdd(lhs_val, rhs_val); break;
            case ast::subtract:
                result_val = m_builder.CreateFSub(lhs_val, rhs_val); break;
            case ast::multiply:
                result_val = m_builder.CreateFMul(lhs_val, rhs_val); break;
            case ast::divide:
                result_val = m_builder.CreateFDiv(lhs_val, rhs_val); break;
            default:
                assert(false);
            }

            llvm::Value * result_ptr = result_stream->get_at(stream_index, m_builder);
            m_builder.CreateStore(result_val, result_ptr);
        };

        generate_iteration(result_stream->size(), operation);

        return result_stream;
    }
}

value_ptr generator::process_transpose( const ast::node_ptr & root )
{
    assert(root->type == ast::transpose_expression);
    ast::list_node *root_list = root->as_list();
    const auto & object_node = root_list->elements[0];
    const auto & dims_node = root_list->elements[1];

    value_ptr src_object = process_expression(object_node);

    stream_value_ptr src_stream =
            dynamic_pointer_cast<abstract_stream_value>(src_object);
    assert(src_stream);

    vector<int> map;

    ast::list_node *dims = dims_node->as_list();
    for ( const auto & dim_node : dims->elements )
    {
        map.push_back( dim_node->as_leaf<int>()->value );
    }

    return make_shared<transpose_value>(src_stream, map);
}

value_ptr generator::process_slice( const ast::node_ptr & root )
{
    assert(root->type == ast::slice_expression);
    const auto & object_node = root->as_list()->elements[0];
    const auto & ranges_node = root->as_list()->elements[1];

    value_ptr object = process_expression(object_node);

    stream_value_ptr src_stream = dynamic_pointer_cast<abstract_stream_value>(object);
    assert(src_stream);

#if 0
    cout << "-- slice: source stream size: ";
    for (int i = 0; i < src_stream->dimensions(); ++i)
        cout << src_stream->size(i) << " ";
    cout << endl;
#endif

    ast::list_node *range_list = ranges_node->as_list();

    int range_count = range_list->elements.size();

    vector<value_ptr> slice_offset;
    vector<int> slice_size;
    slice_offset.reserve(range_count);
    slice_size.reserve(range_count);
    bool is_scalar = range_count == src_stream->dimensions();
    int dim = 0;
    for( const auto & range_node : range_list->elements )
    {
        value_ptr range = process_expression(range_node);
        if (dynamic_cast<scalar_value*>(range.get()))
        {
            slice_offset.push_back(range);
            slice_size.push_back(1);
        }
        else
        {
            is_scalar = false;
            throw error("Range slices not supported.");
        }
    }

    if (is_scalar)
    {
        llvm::Value *val_ptr = src_stream->get_at(slice_offset, m_builder);
        llvm::Value *val = m_builder.CreateLoad(val_ptr);
        return make_shared<scalar_value>( val );
    }
    else
    {
        return make_shared<slice_value>( src_stream, slice_offset, slice_size );
    }
}

value_ptr generator::process_iteration( const ast::node_ptr & node,
                                        const value_ptr & result_space )
{
    assert(node->type == ast::for_expression);
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    type_ptr iteration_type = body_node->semantic_type;

    stream_value_ptr result;
    if (result_space)
    {
        result =  dynamic_pointer_cast<abstract_stream_value>(result_space);
        assert(result);
    }
    else
    {
        vector<int> & result_size = node->semantic_type->as<stream>().size;
        result = allocate_stream(result_size);
    }

    vector<stream_value_ptr> domains;

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();
        value_ptr domain = process_expression(iter.domain);
        stream_value_ptr stream_domain;
        if (stream_domain = dynamic_pointer_cast<abstract_stream_value>(domain))
            domains.push_back(stream_domain);
        else
            throw error("Unsupported iteration domain type.");
    }

    auto body = [&]( const value_ptr & index )
    {
        // Set up scope.

        context::scope_holder iteration_scope(m_ctx);

        for (int idx = 0; idx < domains.size(); ++idx)
        {
            const auto & iterator_node = iterators_node->as_list()->elements[idx];
            semantic::iterator & iter =
                    iterator_node->semantic_type->as<semantic::iterator>();

            if (iter.id.empty())
                continue;

            value_ptr iterator_index = index;
            if (iter.hop != 1)
            {
                llvm::Value *v =
                        m_builder.CreateMul( index->get(m_builder),
                                             get_uint32(iter.hop) );
                iterator_index = make_shared<scalar_value>(v);
            }

            value_ptr domain_slice = slice_stream(domains[idx], {iterator_index}, {iter.size});
            {
                scalar_value * scalar = dynamic_cast<scalar_value*>(domain_slice.get());
                if(scalar)
                {
                    llvm::Value *v = m_builder.CreateLoad(domain_slice->get(m_builder));
                    domain_slice = make_shared<scalar_value>(v);
                }
            }

            m_ctx.bind(iter.id, make_shared<value_item>(domain_slice));
        }

        // Generate body

        value_ptr body_result_space = slice_stream(result, {index});
        process_block( body_node, body_result_space );
    };

    generate_iteration(0, result->size(0), body);

    return result;
}

value_ptr generator::process_reduction( const ast::node_ptr & node, const value_ptr & result_space )
{
    assert(node->type == ast::reduce_expression);
    const auto & id1_node = node->as_list()->elements[0];
    const auto & id2_node = node->as_list()->elements[1];
    const auto & domain_node = node->as_list()->elements[2];
    const auto & body_node = node->as_list()->elements[3];

    string id1 = id1_node->as_leaf<string>()->value;
    string id2 = id2_node->as_leaf<string>()->value;

    type_ptr reduction_type = body_node->semantic_type;
    assert(reduction_type->is(type::real_num));

    value_ptr result = result_space;
    if (!result)
    {
        llvm::Value *result_ptr =
                m_builder.CreateAlloca(llvm::Type::getDoubleTy(llvm_context()));
        result = make_shared<scalar_value>(result_ptr);
    }

    value_ptr domain_val = process_expression(domain_node);
    stream_value_ptr domain_stream =
            dynamic_pointer_cast<abstract_stream_value>(domain_val);
    assert(domain_stream);
    assert(domain_stream->dimensions() == 1);
    assert(domain_stream->size(0) > 0);

    m_builder.CreateStore( m_builder.CreateLoad(domain_stream->get(m_builder)),
                           result->get(m_builder) );

    auto body = [&]( const value_ptr & index )
    {
        llvm::Value *item1_val =
                m_builder.CreateLoad( result->get(m_builder) );
        llvm::Value *item2_val =
                m_builder.CreateLoad( domain_stream->get_at({index}, m_builder) );

        context::scope_holder iteration_scope(m_ctx);
        m_ctx.bind(id1, make_shared<value_item>(item1_val));
        m_ctx.bind(id2, make_shared<value_item>(item2_val));

        process_block(body_node, result);
    };

    generate_iteration(1, domain_stream->size(0), body);

    return make_shared<scalar_value>( m_builder.CreateLoad(result->get(m_builder)) );
}

void generator::generate_iteration(const value_ptr & from,
                                    const value_ptr & to,
                                    std::function<void(const value_ptr &)> action )
{
    llvm::Function *parent = m_builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *before_block = m_builder.GetInsertBlock();
    llvm::BasicBlock *cond_block = llvm::BasicBlock::Create(llvm_context(), "condition", parent);
    llvm::BasicBlock *action_block = llvm::BasicBlock::Create(llvm_context(), "action", parent);
    llvm::BasicBlock *after_block = llvm::BasicBlock::Create(llvm_context(), "afterloop");

    llvm::Value *start_index = from->get(m_builder);
    llvm::Value *max_index = to->get(m_builder);


    // BEFORE:

    m_builder.CreateBr(cond_block);

    // CONDITION

    m_builder.SetInsertPoint(cond_block);

    // create index
    llvm::PHINode *index = m_builder.CreatePHI(start_index->getType(), 2);
    index->addIncoming(start_index, before_block);

    // create condition
    llvm::Value *condition = m_builder.CreateICmpSLT(index, max_index);
    m_builder.CreateCondBr(condition, action_block, after_block);

    // ACTION

    m_builder.SetInsertPoint(action_block);

    // invoke action functor
    action(make_shared<scalar_value>(index));

    // increment index
    llvm::Value *index_increment = llvm::ConstantInt::get(start_index->getType(), 1);
    llvm::Value *next_index_value =
            m_builder.CreateAdd(index, index_increment);
    index->addIncoming(next_index_value, m_builder.GetInsertBlock());

    m_builder.CreateBr(cond_block);

    // AFTER

    parent->getBasicBlockList().push_back(after_block);

    m_builder.SetInsertPoint(after_block);
}

void generator::generate_iteration( int from, int to,
                                    std::function<void(const value_ptr &)> action )
{
    llvm::Value *start_index =
            llvm::ConstantInt::get(llvm_context(), llvm::APInt(32,from));
    llvm::Value *end_index =
            llvm::ConstantInt::get(llvm_context(), llvm::APInt(32,to));

    generate_iteration(make_shared<scalar_value>(start_index),
                       make_shared<scalar_value>(end_index),
                       action);
}

void generator::generate_iteration( const vector<int> range,
                                    std::function<void(const vector<value_ptr> &)> final_action,
                                    const vector<value_ptr> & stream_index )
{
    if (stream_index.size() < range.size())
    {
        int dim = stream_index.size();
        llvm::Value *start_index =
                llvm::ConstantInt::get(llvm_context(), llvm::APInt(32,0));
        llvm::Value *max_index =
                llvm::ConstantInt::get(llvm_context(), llvm::APInt(32,range[dim]));

        auto action = [&](const value_ptr & index)
        {
            vector<value_ptr> composed_index = stream_index;
            composed_index.push_back(index);
            generate_iteration(range, final_action, composed_index);
        };

        generate_iteration(make_shared<scalar_value>(start_index),
                           make_shared<scalar_value>(max_index),
                           action);
    }
    else
    {
        final_action(stream_index);
    }
}

void generator::generate_store( const value_ptr & dst, const value_ptr & src )
{
    if (scalar_value *scalar = dynamic_cast<scalar_value*>(src.get()))
    {
        m_builder.CreateStore( scalar->get(m_builder), dst->get(m_builder) );
    }
    else if(abstract_stream_value *src_stream = dynamic_cast<abstract_stream_value*>(src.get()))
    {
        abstract_stream_value *dst_stream = dynamic_cast<abstract_stream_value*>(dst.get());

        auto copy_action = [&]( const vector<value_ptr> & index )
        {
            llvm::Value *src_val_ptr = src_stream->get_at(index, m_builder);
            llvm::Value *dst_val_ptr = dst_stream->get_at(index, m_builder);
            m_builder.CreateStore( m_builder.CreateLoad(src_val_ptr), dst_val_ptr );
        };

        generate_iteration(src_stream->size(), copy_action);
    }
}

value_ptr generator::slice_stream( const stream_value_ptr &stream,
                                   const vector<value_ptr> & offset,
                                   const vector<int> & size )
{
    bool all_sizes_one = true;
    for(int i : size)
    {
        if (i != 1)
        {
            all_sizes_one = false;
            break;
        }
    }

    if (offset.size() < stream->dimensions() || !all_sizes_one)
    {
        vector<int> slice_size = size;
        if (slice_size.empty())
            slice_size = vector<int>(offset.size(), 1);
        return make_shared<slice_value>(stream, offset, slice_size);
    }
    else
    {
        return make_shared<scalar_value>(stream->get_at(offset, m_builder));
    }
}


stream_value_ptr generator::allocate_stream( const vector<int> & stream_size )
{
    int alloc_count = 1;
    for (int s : stream_size)
        alloc_count *= s;

    size_t alloc_bytes = alloc_count * 8;

    llvm::Value *offset =
            llvm::ConstantInt::get(llvm_context(),
                                   llvm::APInt(sizeof(alloc_bytes),
                                               m_buffer_pool_size));
    llvm::Value *buffer =
            m_builder.CreateGEP(m_buffer_pool, offset);

    m_buffer_pool_size += alloc_bytes;

    return make_shared<stream_value>(buffer, stream_size);
}

llvm::Value *stream_value::get_at( const vector<value_ptr> & index,
                                   llvm::IRBuilder<> & builder )
{
    assert(index.size() == m_index_coeffs.size() + 1);

    llvm::Value * flat_index = index.back()->get(builder);

    int c = m_index_coeffs.size();
    while(c--)
    {
        llvm::Value *idx_val = index[c]->get(builder);

        llvm::Value *coeff_val =
                llvm::ConstantInt::get(builder.getContext(),
                                       llvm::APInt(32,m_index_coeffs[c],true));

        llvm::Value *partial_val =
                builder.CreateMul( idx_val, coeff_val );

        flat_index = builder.CreateAdd(flat_index, partial_val);
    }

    return llvm::GetElementPtrInst::Create(m_data,
                                           llvm::ArrayRef<llvm::Value*>(&flat_index,1),
                                           "", builder.GetInsertBlock());
}

llvm::Value *slice_value::get_at( const vector<value_ptr> & index,
                                  llvm::IRBuilder<> & builder )
{
    assert(index.size() == m_size.size());
    vector<value_ptr> source_index;
    source_index.reserve(m_preoffset.size() + m_size.size());
    for(const value_ptr & preoffset : m_preoffset)
    {
        source_index.push_back(preoffset);
    }
    int dim = 0;
    while (dim < m_offset.size())
    {
        llvm::Value *index_val =
                builder.CreateAdd(index[dim]->get(builder),
                                  m_offset[dim]->get(builder));
        source_index.push_back(make_shared<scalar_value>(index_val));
        ++dim;
    }
    while (dim < m_size.size())
    {
        source_index.push_back(index[dim]);
        ++dim;
    }
    return m_source->get_at( source_index, builder );
}

}
}
