#include "symbols.hpp"
//#include "types.hpp"

#include <stdexcept>
#include <cassert>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

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
            environment_item *item = evaluate_statement(env, stmt);
            cout << "[line " << stmt->line << "] "
                 << "Added top-level declaration: " << item->name()
                 << endl;
        }
        catch( semantic_error & e )
        {
            e.report();
        }
    }

    return std::move(env);
}
#if 0
sp<type> evaluate( environment & env,
                   const sp<function> & f,
                   const vector<sp<type>> & a )
{
    if (a.size() != f->parameters.size())
        throw semantic_error("Wrong number of arguments.");

    env.enter_scope();

    for (int i = 0; i < f->parameters.size(); ++i)
    {
        env.bind(f->parameters[i], a[i]);
    }

    evaluate_expr_block(env, f->body);

    env.exit_scope();
}
#endif
sp<type> environment_item::evaluate( environment & env, const vector<sp<type>> & args )
{
    if (m_value)
        return m_value;

    if (m_parameters.size() != args.size())
        throw semantic_error("Wrong number of arguments.");

    sp<type> result;

    if (m_parameters.size())
    {
        env.enter_scope();

        for (int i = 0; i < m_parameters.size(); ++i)
        {
            env.bind(m_parameters[i], args[i]);
        }

        result = evaluate_expr_block(env, m_code);

        env.exit_scope();
    }
    else
    {
        m_value = result = evaluate_expr_block(env, m_code);
    }

    return result;
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

environment_item * evaluate_statement( environment & env, const sp<ast::node> & root )
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

    environment_item *env_item = new environment_item(id, parameters, stmt->elements[2]);

    env.bind(id, env_item);

    return env_item;
}

sp<type> evaluate_expression( environment & env, const sp<ast::node> & root )
{
    switch(root->type)
    {
    case ast::integer_num:
        cout << "got int" << endl;
        return sp<type>( new integer_num( root->as_leaf<int>()->value ) );
    case ast::real_num:
        cout << "got real" << endl;
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
    // TODO...
    default:
        throw semantic_error("Unsupported expression.", root->line);
        //assert(false);
        //return sp<type>();
    }
}


template<typename R, typename LHS, typename RHS>
type * apply_binop( ast::node_type op, const LHS * lhs, const RHS * rhs )
{
    R *result = new R;

    bool constant = (lhs->is_constant() && rhs->is_constant());

    cout << "constant: " << constant << endl;

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

template<typename LHS>
type * apply_binop( ast::node_type op, const LHS * lhs, type *rhs )
{
    switch(rhs->get_tag())
    {
    case type::integer_num:
        return apply_binop<LHS, LHS, integer_num>
                (op, lhs, static_cast<integer_num*>(rhs));
    case type::real_num:
        return apply_binop<real_num, LHS, real_num>
                (op, lhs, static_cast<real_num*>(rhs));
    default:
        throw semantic_error("Right-hand-side operand not a number.");
    }
}

type * apply_binop( ast::node_type op, type * lhs, type *rhs )
{
    switch(lhs->get_tag())
    {
    case type::integer_num:
        return apply_binop<integer_num>
                (op, static_cast<integer_num*>(lhs), rhs);
    case type::real_num:
        return apply_binop<real_num>
                (op, static_cast<real_num*>(lhs), rhs);
    default:
        throw semantic_error("Left-hand-side operand not a number.");
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
        throw semantic_error("Dimension selector unsupported.", n->line);

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
    environment_item *item = env[id];
    if (!item)
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
        callee = item->evaluate(env, args);
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

    // TODO: Apply stream range selection

    return callee;
}

#if 0
function::function( const sp<ast::node> & def )
{
    assert(def->type == ast::statement);
    ast::list_node *statement = def->as_list();

    assert(statement->elements.size() == 3);

    // Get name

    ast::node *id = statement->elements[0].get();
    assert(id);
    assert(id->type == ast::identifier);

    name = id->as_leaf<string>()->value;

    // Get parameters

    ast::node *params = statement->elements[1].get();
    if (params)
    {
        assert(params->type == ast::id_list);
        ast::list_node *param_list = params->as_list();
        for ( const sp<ast::node> & param : param_list->elements )
        {
            assert(param->type == ast::identifier);
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);
        }
    }

    // Store body

    assert( statement->elements[2]->type == ast::expression_block );
    body = statement->elements[2];
}

sp<type> func_environment_item::evaluate_type(environment & envir ,
                                              const vector<sp<type> > & args)
{
    // TODO
    return sp<type>();
}
#endif

void semantic_error::report()
{
    std::cerr << "ERROR [line " << m_line << "]: " << what() << std::endl;
}

} // namespace semantic
} // namespace stream
