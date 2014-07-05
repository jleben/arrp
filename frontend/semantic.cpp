#include "semantic.hpp"

#include <stdexcept>
#include <cassert>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

environment::environment()
{
    enter_scope();

    // Add built-in functions

    bind( new elemwise_func_symbol("log") );
    bind( new elemwise_func_symbol("sqrt") );
    bind( new elemwise_func_symbol("exp") );
    bind( new elemwise_func_symbol("sin") );
    bind( new elemwise_func_symbol("cos") );
    bind( new elemwise_func_symbol("tan") );
    bind( new elemwise_func_symbol("asin") );
    bind( new elemwise_func_symbol("acos") );
    bind( new elemwise_func_symbol("atan") );
    bind( new elemwise_func_symbol("ceil") );
    bind( new elemwise_func_symbol("floor") );
    bind( new elemwise_func_symbol("abs") );

}

environment top_environment( ast::node * root )
{
    using ast::list_node;
    using ast::leaf_node;
    using ast::node;

    if (root->type != ast::program)
        throw std::runtime_error("Root node is not a program.");

    environment env;

    list_node *statements = root->as_list();
    for ( const sp<node> & stmt : statements->elements )
    {
        try
        {
            symbol *sym = evaluate_statement(env, stmt);
            cout << "[line " << stmt->line << "] "
                 << "Added top-level declaration: " << sym->name()
                 << endl;
        }
        catch( semantic_error & e )
        {
            e.report();
        }
    }

    return std::move(env);
}

sp<type> constant_symbol::evaluate( environment & env, const vector<sp<type>> & args )
{
    if (!args.empty())
    {
        throw wrong_arg_count(name(), 0, args.size() );
    }

    if (!m_value)
        m_value = evaluate_expr_block(env, m_code);

    return m_value;
}

sp<type> function_symbol::evaluate( environment & env, const vector<sp<type>> & args )
{
    if (m_parameters.size() != args.size())
    {
        throw wrong_arg_count(name(), m_parameters.size(), args.size() );
    }

    sp<type> result;

    env.enter_scope();

    for (int i = 0; i < m_parameters.size(); ++i)
    {
        env.bind(m_parameters[i], args[i]);
    }

    result = evaluate_expr_block(env, m_code);

    env.exit_scope();

    return result;
}

sp<type> elemwise_func_symbol::evaluate( environment & env,
                                         const vector<sp<type>> & args )
{
    if (args.size() != 1)
        throw wrong_arg_count(name(), 1, args.size());

    const sp<type> & t = args[0];

    switch(t->get_tag())
    {
    case type::integer_num:
        return sp<type>(new integer_num);
    case type::real_num:
        return sp<type>(new real_num);
    case type::range:
        return sp<type>(new range);
    case type::stream:
        return t;
    default:
        throw call_error(name(), "Invalid argument type.");
    }
}

sp<type> evaluate_expr_block( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::expression_block);

    ast::list_node *expr_block = root->as_list();

    assert(expr_block->elements.size() == 2);

    if (expr_block->elements[0])
        evaluate_stmt_list( env, expr_block->elements[0] );

    return evaluate_expression( env, expr_block->elements[1] );
}

void evaluate_stmt_list( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::statement_list);

    ast::list_node *stmts = root->as_list();

    for ( const sp<ast::node> & stmt : stmts->elements )
    {
        try {
            evaluate_statement(env, stmt);
        } catch( semantic_error & e ) {
            e.report();
        }
    }
}

symbol * evaluate_statement( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::statement);
    ast::list_node *stmt = root->as_list();

    assert(stmt->elements.size() == 3);

    assert(stmt->elements[0]->type == ast::identifier);

    const string & id = stmt->elements[0]->as_leaf<string>()->value;

    bool can_bind = !env[id];

    if (!can_bind)
    {
        ostringstream msg;
        msg << "Name already in scope: '" << id << "'";
        throw semantic_error(msg.str(), root->line);
    }

    // Get parameters

    vector<string> parameters;

    if (stmt->elements[1])
    {
        ast::node *params = stmt->elements[1].get();
        assert(params->type == ast::id_list);

        ast::list_node *param_list = params->as_list();
        for ( const sp<ast::node> & param : param_list->elements )
        {
            assert(param->type == ast::identifier);
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);
        }
    }

    const auto & body = stmt->elements[2];

    symbol *sym;
    if (!parameters.empty())
        sym = new function_symbol(id, parameters, body);
    else
        sym = new constant_symbol(id, body);

    env.bind(sym);

    return sym;
}

sp<type> evaluate_expression( environment & env, const sp<ast::node> & root )
{
    switch(root->type)
    {
    case ast::integer_num:
        return sp<type>( new integer_num( root->as_leaf<int>()->value ) );
    case ast::real_num:
        return sp<type>( new real_num( root->as_leaf<double>()->value ) );
    case ast::range:
        return evaluate_range(env, root);
    case ast::hash_expression:
        return evaluate_hash(env, root);
    case ast::call_expression:
        return evaluate_call(env, root);
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
        return evaluate_binop(env, root);
    case ast::for_expression:
        return evaluate_iteration(env, root);
    case ast::reduce_expression:
        return evaluate_reduction(env, root);
    // TODO...
    default:
        throw semantic_error("Unsupported expression.", root->line);
        //assert(false);
        //return sp<type>();
    }
}

template<typename R, typename LHS, typename RHS>
R * apply_binop( ast::node_type op, LHS * lhs, RHS * rhs )
{
    //cout << "binop<LHS,RHS>" << endl;

    R *result = new R;

    bool constant = (lhs->is_constant() && rhs->is_constant());

    switch(op)
    {
    case ast::add:
        if (constant)
            result->set_constant( lhs->constant_value() + rhs->constant_value() );
        break;
    case ast::subtract:
        if (constant)
            result->set_constant( lhs->constant_value() - rhs->constant_value() );
        break;
    case ast::multiply:
        if (constant)
            result->set_constant( lhs->constant_value() * rhs->constant_value() );
        break;
    case ast::divide:
        if (constant)
            result->set_constant( lhs->constant_value() / rhs->constant_value() );
        break;
    default:
        throw semantic_error("Unexpected operator.");
    }

    return result;
}

template<>
stream * apply_binop<stream,stream,stream>( ast::node_type op, stream * lhs, stream *rhs )
{
    //cout << "binop<stream,stream>" << endl;
    if (lhs->dimensionality() != rhs->dimensionality())
        throw semantic_error("Operand streams have different number of dimensions.");

    if (lhs->size != rhs->size)
        throw semantic_error("Operand streams have different sizes.");

    return new stream(lhs->size);
}

template<typename LHS>
type * apply_binop( ast::node_type op, LHS * lhs, type *rhs )
{
    //cout << "binop<LHS,type>" << endl;

    switch(rhs->get_tag())
    {
    case type::integer_num:
        return apply_binop
                <LHS, LHS, integer_num>
                (op, lhs, static_cast<integer_num*>(rhs));
    case type::real_num:
        return apply_binop
                <real_num, LHS, real_num>
                (op, lhs, static_cast<real_num*>(rhs));
    default:
        throw semantic_error("Right-hand-side operand not a stream or a number.");
    }
}

type * apply_binop( ast::node_type op, type * lhs, type *rhs )
{
    //cout << "binop<type,type>" << endl;

    if (lhs->get_tag() == type::stream && rhs->get_tag() == type::stream)
    {
        return apply_binop<stream,stream,stream>(op,
                           static_cast<stream*>(lhs),
                           static_cast<stream*>(rhs));
    }
    else if (lhs->get_tag() == type::stream &&
             (rhs->get_tag() == type::integer_num ||
              rhs->get_tag() == type::real_num))
    {
        return new stream( static_cast<stream*>(lhs)->size );
    }
    else if (rhs->get_tag() == type::stream &&
             (lhs->get_tag() == type::integer_num ||
              lhs->get_tag() == type::real_num))
    {
        return new stream( static_cast<stream*>(rhs)->size );
    }

    switch(lhs->get_tag())
    {
    case type::integer_num:
        return apply_binop<integer_num>
                (op, static_cast<integer_num*>(lhs), rhs);
    case type::real_num:
        return apply_binop<real_num>
                (op, static_cast<real_num*>(lhs), rhs);
    default:
        throw semantic_error("Left-hand-side operand not a stream or a number.");
    }
}

sp<type> evaluate_binop( environment & env, const sp<ast::node> & root )
{
    ast::list_node *expr = root->as_list();
    assert(expr->elements.size() == 2);

    sp<type> lhs = evaluate_expression( env, expr->elements[0] );
    sp<type> rhs = evaluate_expression( env, expr->elements[1] );

#if 0
    type::tag lhs_t = lhs->get_tag();
    type::tag rhs_t = rhs->get_tag();

    if ( (lhs_t != type::integer_num && lhs_t != type::real_num) ||
         (rhs_t != type::integer_num && rhs_t != type::real_num) )
    {
        throw semantic_error("Invalid operands to binary operator.", root->line);
    }
#endif

    try
    {
        type * result = apply_binop(expr->type, lhs.get(), rhs.get());
        return sp<type>( result );
    }
    catch ( semantic_error & e )
    {
        throw semantic_error(e.what(), expr->line);
    }
}

sp<type> evaluate_range( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::range);
    ast::list_node *range_node = root->as_list();
    assert(range_node->elements.size() == 2);
    const sp<ast::node> & start_node = range_node->elements[0];
    const sp<ast::node> & end_node = range_node->elements[1];

    sp<range> r( new range );

    if (start_node)
    {
        sp<type> val = evaluate_expression(env, start_node);
        if (val->get_tag() != type::integer_num)
            throw semantic_error("Range start not an integer.", start_node->line);
        r->start = val;
    }
    if (end_node)
    {
        sp<type> val = evaluate_expression(env, end_node);
        if (val->get_tag() != type::integer_num)
            throw semantic_error("Range end not an integer.", start_node->line);
        r->end = val;
    }

    return r;
}

sp<type> evaluate_hash( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::hash_expression);
    ast::list_node *range_node = root->as_list();
    assert(range_node->elements.size() == 2);
    const sp<ast::node> & call_node = range_node->elements[0];
    const sp<ast::node> & dim_node = range_node->elements[1];

    sp<type> callee = evaluate_call(env, call_node);

    if (callee->get_tag() != type::stream)
        throw semantic_error("Hash argument not a stream.", call_node->line);

    int dim = 1;
    if (dim_node)
    {
        sp<type> dim_type = evaluate_expression(env, dim_node);
        if (dim_type->get_tag() != type::integer_num)
            throw semantic_error("Dimension not an integer.", dim_node->line);
        integer_num *dim_int = static_cast<integer_num*>(dim_type.get());
        if (!dim_int->is_constant())
            throw semantic_error("Dimension not a constant.", dim_node->line);
        dim = dim_int->constant_value();
    }

    stream *s = static_cast<stream*>(callee.get());
    if (dim < 1 || dim > s->dimensionality())
    {
        ostringstream msg;
        msg << "Dimension " << dim << " out of bounds.";
        throw semantic_error(msg.str(), call_node->line);
    }

    int size = s->size[dim-1];

    return sp<type>( new integer_num(size) );
}

stream *transpose( stream * in, const sp<ast::node> & n )
{
    assert(n->type == ast::int_list);
    ast::list_node *dims = n->as_list();

    if (dims->elements.empty())
    {
        // throw semantic_error("Dimension selector unsupported.", n->line);
        // FIXME: Find more elegant solution.
        return new stream( {1} );
    }

    if (dims->elements.size() > in->dimensionality())
        throw semantic_error("Too many dimension selector elements.", n->line);

    vector<bool> selected_dims(in->dimensionality(), false);

    vector<int> transposed_size;
    transposed_size.reserve(in->dimensionality());

    for ( const sp<ast::node> & e : dims->elements )
    {
        assert(e->type == ast::integer_num);
        int dim = e->as_leaf<int>()->value;
        if (dim < 1 || dim > in->dimensionality())
            throw semantic_error("Dimension selector element out of bounds.", e->line);
        if (selected_dims[dim-1])
            throw semantic_error("Duplicate dimension selector element.", e->line);
        transposed_size.push_back( in->size[dim-1] );
        selected_dims[dim-1] = true;
    }

    for (int dim = 0; dim < selected_dims.size(); ++dim)
    {
        if (!selected_dims[dim])
            transposed_size.push_back( in->size[dim] );
    }

    return new stream( std::move(transposed_size) );
}

sp<type> evaluate_call( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::call_expression);

    ast::list_node * call = root->as_list();

    assert(call->elements.size() == 4);

    // Get value from environment:

    assert(call->elements[0]->type == ast::identifier);
    const string & id = call->elements[0]->as_leaf<string>()->value;
    symbol *sym = env[id];
    if (!sym)
    {
        ostringstream msg;
        msg << "Undeclared name: " << id;
        throw semantic_error(msg.str(), root->line);
    }

    // Extract args

    std::vector<sp<type>> args;

    if (call->elements[1])
    {
        assert(call->elements[1]->type == ast::expression_list);
        ast::list_node *exprs = call->elements[1]->as_list();
        for ( const sp<ast::node> & e : exprs->elements )
        {
            args.emplace_back( evaluate_expression(env, e) );
        }
    }

    sp<type> callee;

    // Evaluate callee

    try {
        callee = sym->evaluate(env, args);
    } catch ( semantic_error & e ) {
        throw semantic_error(e.what(), root->line);
    }

    // Apply stream transposition

    if (call->elements[2])
    {
        if (callee->get_tag() != type::stream)
            throw semantic_error("Dimension selector applied to non-stream value.", call->elements[2]->line);
        stream *transposed_stream = transpose(static_cast<stream*>(callee.get()), call->elements[2]);
        callee.reset( transposed_stream );
    }

    if (call->elements[3])
    {
        assert(call->elements[3]->type == ast::expression_list);
        ast::list_node *exprs = call->elements[3]->as_list();

        if (!callee->is(type::stream))
            throw semantic_error("Slice selector applied to non-stream value.", call->elements[3]->line);

        stream *s = static_cast<stream*>(callee.get());

        int dim = 0;
        for( const auto & e : exprs->elements )
        {
            if (dim >= s->dimensionality())
                throw semantic_error("Too many slice selectors.", call->elements[3]->line);

            sp<type> selector = evaluate_expression(env, e);
            switch(selector->get_tag())
            {
            case type::integer_num:
            {
                s->size[dim] = 1;
                break;
            }
            case type::range:
            {
                range *r = selector->as<range>();
                if (!r->start)
                    r->start.reset( new integer_num(1) );
                if (!r->end)
                    r->end.reset( new integer_num(s->size[dim]) );
                if (!r->is_constant())
                    throw semantic_error("Non-constant slice selector not supported.", e->line);
                int size = r->const_size();
                if (size < 1)
                    throw semantic_error("Invalid slice selector: range size < 1.", e->line);
                s->size[dim] = size;
                break;
            }
            default:
                    throw semantic_error("Invalid type of slice selector.", e->line);
            }
            ++dim;
        }
    }

    return callee;
}

sp<type> evaluate_iteration( environment & env, const sp<ast::node> & root )
{
    //cout << "+++ iteration +++" << endl;

    assert(root->type == ast::for_expression);
    ast::list_node *iteration = root->as_list();
    assert(iteration->elements.size() == 2);

    assert(iteration->elements[0]->type == ast::for_iteration_list);
    ast::list_node *iterator_list = iteration->elements[0]->as_list();

    vector<iterator> iterators;
    iterators.reserve(iterator_list->elements.size());

    for( const sp<ast::node> & e : iterator_list->elements )
    {
        iterators.emplace_back( evaluate_iterator(env, e) );
    }

    assert(!iterators.empty());

    int iteration_count = 0;

    for( const iterator & it : iterators )
    {
        if (!iteration_count)
            iteration_count = it.count;
        else if (it.count != iteration_count)
            throw semantic_error("Iterations with differing counts.", root->line);
    }

    env.enter_scope();

    for( const iterator & it : iterators )
    {
        env.bind(it.id, it.value);
    }

    sp<type> result = evaluate_expr_block(env, iteration->elements[1]);

    env.exit_scope();

    stream *product = new stream({iteration_count});

    switch(result->get_tag())
    {
    case type::stream:
    {
        stream *result_stream = static_cast<stream*>(result.get());
        product->size.insert( product->size.end(),
                              result_stream->size.begin(),
                              result_stream->size.end() );
        product->reduce();
        break;
    }
    case type::integer_num:
    case type::real_num:
        break;
    default:
        throw semantic_error("Unsupported iteration result type.", iteration->elements[1]->line);
    }

    //cout << "result: " << *result << endl;
    //cout << "product: " << *product << endl;
    //cout << "--- iteration ---" << endl;

    return sp<type>(product);
}

iterator evaluate_iterator( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::for_iteration);
    ast::list_node *iteration = root->as_list();
    assert(iteration->elements.size() == 4);

    iterator it;

    if (iteration->elements[0])
    {
        assert(iteration->elements[0]->type == ast::identifier);
        it.id = iteration->elements[0]->as_leaf<string>()->value;
    }

    if (iteration->elements[1])
    {
        sp<type> val = evaluate_expression(env, iteration->elements[1]);
        if (val->get_tag() != type::integer_num)
            throw semantic_error("Iteration size not an integer.");
        integer_num *i = static_cast<integer_num*>(val.get());
        if (!i->is_constant())
            throw semantic_error("Iteration size not a constant.");
        it.size = i->constant_value();
        if (it.size < 1)
            throw semantic_error("Invalid iteration size.");
    }

    if (iteration->elements[2])
    {
        sp<type> val = evaluate_expression(env, iteration->elements[2]);
        if (val->get_tag() != type::integer_num)
            throw semantic_error("Iteration hop not an integer.");
        integer_num *i = static_cast<integer_num*>(val.get());
        if (!i->is_constant())
            throw semantic_error("Iteration hop not a constant.");
        it.hop = i->constant_value();
        if (it.hop < 1)
            throw semantic_error("Invalid hop size.");
    }

    it.domain = evaluate_expression(env, iteration->elements[3]);

    // Get domains size:

    int domain_size;

    switch(it.domain->get_tag())
    {
    case type::stream:
    {
        stream *domain_stream = static_cast<stream*>(it.domain.get());
        assert(domain_stream->dimensionality());
        domain_size = domain_stream->size[0];

        stream *operand_stream = new stream(*domain_stream);
        operand_stream->size[0] = it.size;
        operand_stream->reduce();
        it.value.reset(operand_stream);

        break;
    }
    case type::range:
    {
        range *domain_range = static_cast<range*>(it.domain.get());
        if (!domain_range->is_constant())
            throw semantic_error("Non-constant range not supported as iteration domain.");
        domain_size = domain_range->const_size();

        if (it.size > 1)
        {
            range *operand_range = new range;
            operand_range->start.reset(new integer_num);
            operand_range->end.reset(new integer_num);
            it.value.reset(operand_range);
        }
        else
        {
            it.value.reset( new integer_num );
        }

        break;
    }
    default:
        throw semantic_error("Unsupported iteration domain type.", iteration->line);
    }

    // Compute iteration count:

    int iterable_size = domain_size - it.size;
    if (iterable_size < 0)
        throw semantic_error("Iteration size larger than stream size.", iteration->line);
    if (iterable_size % it.hop != 0)
        throw semantic_error("Iteration does not cover stream size.", iteration->line);
    it.count = iterable_size / it.hop + 1;

    cout << "Iterator: " << it.id << " " << it.count << " x " << it.size << endl;

    return it;
}

sp<type> evaluate_reduction( environment & env, const sp<ast::node> & root )
{
    assert(root->type == ast::reduce_expression);
    ast::list_node *reduction = root->as_list();
    assert(reduction->elements.size() == 4);

    assert(reduction->elements[0]->type == ast::identifier);
    assert(reduction->elements[1]->type == ast::identifier);
    string id1 = reduction->elements[0]->as_leaf<string>()->value;
    string id2 = reduction->elements[1]->as_leaf<string>()->value;

    sp<type> domain = evaluate_expression(env, reduction->elements[2]);

    sp<type> val1;
    sp<type> val2;

    if (domain->get_tag() == type::stream)
    {
        stream *domain_stream = static_cast<stream*>(domain.get());
        stream *operand_stream = new stream(*domain_stream);
        if (operand_stream->dimensionality() > 0)
        {
            operand_stream->size[0] = 1;
            operand_stream->reduce();
        }
        val1 = val2 = sp<type>(operand_stream);
    }
    else if (domain->get_tag() == type::range)
    {
        val1 = val2 = sp<type>( new integer_num );
    }
    else
    {
        throw semantic_error("Invalid reduction domain.", root->line);
    }

    env.enter_scope();

    env.bind(id1, val1);
    env.bind(id2, val2);

    sp<type> reduct = evaluate_expr_block(env, reduction->elements[3]);

    env.exit_scope();

    return reduct;
}


void semantic_error::report()
{
    std::cerr << "ERROR [line " << m_line << "]: " << what() << std::endl;
}

} // namespace semantic
} // namespace stream
