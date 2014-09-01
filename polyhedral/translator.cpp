#include "translator.hpp"

#include <stdexcept>

namespace stream {
namespace polyhedral {

using namespace std;

translator::translator()
{
    //m_context.enter_scope();
}

expression * translator::translate_type(const semantic::type_ptr & type)
{
    switch(type->get_tag())
    {
    case semantic::type::integer_num:
        return new constant<int>(0);
    case semantic::type::real_num:
        return new constant<double>(0.0);
    case semantic::type::stream:
    {
        auto stream_type = type->as<semantic::stream>();

        statement *generator = new statement;
        generator->domain = stream_type.size;
        m_statements.push_back(generator);

        int dimension = generator->domain.size();

        auto expr = new stream_view;
        expr->target = generator;
        //expr->index_range = generator->domain;
        expr->map = matrix<int>::identity(dimension, dimension);
        expr->offset = vector<int>(dimension, 0);
        expr->current_iteration = 0;
        return expr;
    }
    default:
        throw std::runtime_error("Translation: Unexpected type.");
    }
}

void translator::translate(const semantic::function & func,
                           const vector<semantic::type_ptr> & args)
{
    using namespace semantic;

    context::scope_holder func_scope(m_context);

    assert(func.parameters.size() == args.size());
    for(int i = 0; i < func.parameters.size(); ++i)
    {
        m_context.bind(func.parameters[i], translate_type(args[i]) );
    }

    expression * result = do_block( func.expression() );

    vector<int> result_domain;

    switch(func.result_type()->get_tag())
    {
    case type::stream:
        result_domain = func.result_type()->as<stream>().size;
        break;
    case type::integer_num:
    case type::real_num:
        result_domain.resize(1,1);
        break;
    default:
        throw runtime_error("Unexpected type.");
    }

    if (auto view = dynamic_cast<stream_view*>(result))
        result = complete_access(view);

    auto stmt = new statement;
    m_statements.push_back(stmt);
    stmt->domain = result_domain;
    stmt->expr = result;
}

void translator::do_statement_list(const ast::node_ptr &node)
{
    for ( const sp<ast::node> & stmt : node->as_list()->elements )
        do_statement(stmt);
}

void translator::do_statement(const ast::node_ptr &node)
{
    ast::list_node *stmt = node->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & body_node = stmt->elements[2];
    // user-defined functions should have been inlined:
    // TODO: go check whether they are actually removed!
    assert(!params_node);

    const string & id = id_node->as_leaf<string>()->value;

    expression *expr = do_block(body_node);

    if(!dynamic_cast<stream_view*>(expr))
    {
        using namespace semantic;

        vector<int> domain;
        domain.insert(domain.end(),
                      m_domain.begin(), m_domain.end());
        switch(node->semantic_type->get_tag())
        {
        case type::integer_num:
        case type::real_num:
            break;
        case type::stream:
            vector<int> expr_domain =
                    node->semantic_type->as<stream>().size;
            domain.insert(domain.end(),
                          expr_domain.begin(), expr_domain.end());
            break;
        };
        expr = make_statement(expr, domain);
    }

    m_context.bind(id, expr);
}

expression * translator::do_block(const ast::node_ptr &node)
{
    ast::list_node *expr_block = node->as_list();
    const auto & stmt_list = expr_block->elements[0];
    const auto & expr = expr_block->elements[1];
    if (stmt_list)
        do_statement_list(stmt_list);
    return do_expression(expr);
}

expression * translator::do_expression(const ast::node_ptr &node)
{
    switch(node->type)
    {
    case ast::identifier:
    {
        return do_identifier(node);
    }
    case ast::integer_num:
    {
        int value = node->as_leaf<int>()->value;
        return new constant<int>(value);
    }
    case ast::real_num:
    {
        double value = node->as_leaf<double>()->value;
        return new constant<double>(value);
    }
    //case ast::range:
    case ast::negate:
    {
        return do_unary_op(node);
    }
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::raise:
    {
        return do_binary_op(node);
    }
    //case ast::call_expression:

    case ast::transpose_expression:
    {
        return do_transpose(node);
    }
    //case ast::slice_expression:
    //case ast::hash_expression:

    //case ast::for_expression:
    //case ast::reduce_expression:

    default:
        throw std::runtime_error("Unexpected AST node type.");
    }
}

expression * translator::do_identifier(const ast::node_ptr &node)
{
    string id = node->as_leaf<string>()->value;

    return m_context.find(id).value().source;
}

expression * translator::do_unary_op(const ast::node_ptr &node)
{
    expression *operand = do_expression(node->as_list()->elements[0]);

    auto source_stream = dynamic_cast<stream_view*>(operand);

    // complete access

    if (source_stream)
        operand = complete_access(source_stream);

    // create operation

    auto operation_result = new intrinsic;

    operation_result->operands.push_back(operand);

    switch(node->type)
    {
    case ast::negate:
        operation_result->kind = intrinsic::negate;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    return operation_result;
#if 0
    // create and return result

    if (source_stream)
    {
        vector<int> domain = m_domain;
        std::copy(source_stream->index_range.begin(),
                  source_stream->index_range.end(),
                  std::back_inserter(domain));
        auto result_stream = make_statement(operation_result, domain);
        delete source_stream;
        return result_stream;
    }
    else
    {
        return operation_result;
    }
#endif
}

expression * translator::do_binary_op(const ast::node_ptr &node)
{
    expression *operand1 = do_expression(node->as_list()->elements[0]);
    expression *operand2 = do_expression(node->as_list()->elements[1]);

    auto source_stream1 = dynamic_cast<stream_view*>(operand1);
    auto source_stream2 = dynamic_cast<stream_view*>(operand2);

    // complete access

    if (source_stream1)
        operand1 = complete_access(source_stream1);
    if (source_stream2)
        operand2 = complete_access(source_stream2);

    // create operation

    auto operation_result = new intrinsic;

    operation_result->operands.push_back(operand1);
    operation_result->operands.push_back(operand2);

    switch(node->type)
    {
    case ast::add:
        operation_result->kind = intrinsic::add;
        break;
    case ast::subtract:
        operation_result->kind = intrinsic::subtract;
        break;
    case ast::multiply:
        operation_result->kind = intrinsic::multiply;
        break;
    case ast::divide:
        operation_result->kind = intrinsic::divide;
        break;
    case ast::raise:
        operation_result->kind = intrinsic::raise;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    return operation_result;
#if 0
    // create and return result

    assert( !(source_stream1 && source_stream2) ||
            source_stream1->size == source_stream2->size );

    if (source_stream1 || source_stream2)
    {
        auto additional_domain = source_stream1 ? source_stream1->size : source_stream2->size;
        auto result_stream = partial_access(operation_result, additional_domain);

        delete source_stream1;
        delete source_stream2;

        return result_stream;
    }
    else
    {
        return operation_result;
    }
#endif
}

expression * translator::do_transpose(const ast::node_ptr &node)
{
    const auto & object_node = node->as_list()->elements[0];
    const auto & dims_node = node->as_list()->elements[1];

    expression *object = do_expression(object_node);
    semantic::stream & stream_type =
            node->semantic_type->as<semantic::stream>();
    int dimension = current_dimension() + stream_type.dimensionality();

    vector<int> order;
    order.reserve(dimension);

    vector<bool> done(stream_type.dimensionality(), false);

    for (int d = 0; d < current_dimension(); ++d)
        order.push_back(d);

    for ( const auto & dim_node : dims_node->as_list()->elements )
    {
        int d = dim_node->as_leaf<int>()->value - 1;
        done[d] = true;
        order.push_back( d + current_dimension() );
    }

    for (int d = 0; d < done.size(); ++d)
        if (!done[d])
            order.push_back( d + current_dimension() );

    matrix<int> transposition =
            matrix<int>::identity(dimension, dimension).reordered( order );

    vector<int> zero_offset(dimension, 0);

    update_accesses(object, transposition, zero_offset);

    return object;
}

matrix<int> translator::access(stream_view *source)
{
    if (source->current_iteration == current_dimension())
    {
        return source->map;
    }

    int distance = current_dimension() - source->current_iteration;
    assert(distance > 0);

    matrix<int> map = source->map.resized(source->map.rows(),
                                          source->map.columns() + distance);
    int col = map.columns() - 1;
    for (; col >= current_dimension(); --col)
    {
        for (int r = 0; r < map.rows(); ++r)
            map(r, col) = map(r, col - distance);
    }
    for (; col >= source->current_iteration; --col)
    {
        for (int r = 0; r < map.rows(); ++r)
            map(r, col) = 0;
    }

    source->current_iteration = current_dimension();

    return map;
}


stream_access * translator::complete_access( stream_view * view )
{
    matrix<int> map = this->access(view);

    auto access = new stream_access;
    access->target = view->target;
    access->map = map;
    access->offset = view->offset;

    return access;
#if 0
    int dims = view->target->domain.size();
    int extra_dims = dims - view->iteration_map.rows();
    int iters = view->iteration_map.columns() + extra_dims;

    if (view->current_iteration != current_dimension())

    matrix<int> iter_map = view->iteration_map.resized(dims, iters);
    for (int i = 0; i < extra_dims; ++i)
    {
        int dim = view->iteration_map.rows() + i;
        int iter = view->iteration_map.columns() + i;
        iter_map(dim, iter) = 1;
    }

    auto access = new stream_access;
    access->target = view->target;
    access->map = view->index_map * iter_map;
    access->offset = view->index_offset;

    return access;
#endif
}

translator::stream_view *
translator::make_statement( expression * expr,
                            const vector<int> & domain )
{
    // create statement

    auto stmt = new statement;
    m_statements.push_back(stmt);
    stmt->domain = domain;
    stmt->expr = expr;

    // create view

    auto view = new stream_view;
    view->target = stmt;
    view->map = matrix<int>::identity(domain.size(), domain.size());
    view->offset = vector<int>(domain.size(), 0);
    view->current_iteration = m_domain.size();
    //view->iteration_map = matrix<int>::identity(m_domain.size(), m_domain.size());

    return view;
}

void translator::update_accesses(expression *expr,
                                 const matrix<int> & map,
                                 const vector<int> & offset)
{
    if (auto const_int = dynamic_cast<constant<int>*>(expr))
        return;
    if (auto const_real = dynamic_cast<constant<double>*>(expr))
        return;
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto sub_expr : operation->operands)
            update_accesses(sub_expr, map, offset);
        return;
    }
    if (auto dependency = dynamic_cast<stream_access*>(expr))
    {
        auto pattern =
                utility::compose(dependency->map, dependency->offset,
                                 map, offset);

        dependency->map = pattern.first;
        dependency->offset = pattern.second;
    }
    if (auto view = dynamic_cast<stream_view*>(expr))
    {
        auto pattern =
                utility::compose(view->map, view->offset,
                                 map, offset);

        view->map = pattern.first;
        view->offset = pattern.second;
    }
}

}
}
