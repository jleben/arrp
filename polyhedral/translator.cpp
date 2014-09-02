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
        expr->pattern = mapping::identity(dimension, dimension);
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
        {
            vector<int> expr_domain =
                    node->semantic_type->as<stream>().size;
            domain.insert(domain.end(),
                          expr_domain.begin(), expr_domain.end());
            break;
        }
        default:
            throw runtime_error("Unexpected type.");
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
    case ast::slice_expression:
    {
        return do_slicing(node);
    }
    //case ast::hash_expression:

    case ast::for_expression:
    {
        return do_mapping(node);
    }
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

    semantic::stream & stream_type =
            node->semantic_type->as<semantic::stream>();

    int dimension = current_dimension() + stream_type.dimensionality();

    vector<int> order(dimension,-1);

    int dim = 0;
    for(; dim < current_dimension(); ++dim)
    {
        order[dim] = dim;
    }
    for ( const auto & dim_node : dims_node->as_list()->elements )
    {
        int pos = current_dimension() + dim_node->as_leaf<int>()->value - 1;
        order[pos] = dim;
        ++dim;
    }
    for (int pos = current_dimension(); pos < dimension; ++pos)
    {
        if (order[pos] < 0)
        {
            order[pos] = dim;
            ++dim;
        }
    }
    assert(dim == dimension);

    mapping transposition = mapping::identity(dimension, dimension);
    transposition.coefficients = transposition.coefficients.reordered( order );

    expression *object = do_expression(object_node);

    update_accesses(object, transposition);

    return object;
}

expression * translator::do_slicing(const  ast::node_ptr &node)
{
    using namespace semantic;

    const auto & object_node = node->as_list()->elements[0];
    const auto & ranges_node = node->as_list()->elements[1];

    semantic::stream & stream_type =
            node->semantic_type->as<semantic::stream>();

    int dimension = current_dimension() + stream_type.dimensionality();

    mapping slicing = mapping::identity(dimension, dimension);

    int dim = 0;
    for( const auto & range_node : ranges_node->as_list()->elements )
    {
        int offset;

        // TODO: detect affine expressions of loop indeces and parameters
        // auto selector = do_expression(range_node);

        type_ptr selector_type = range_node->semantic_type;
        switch(selector_type->get_tag())
        {
        case type::integer_num:
        {
            integer_num &int_type = selector_type->as<integer_num>();
            if (!int_type.is_constant())
                throw runtime_error("Non-constant slicing not supported.");
            offset = int_type.constant_value() - 1;
            break;
        }
        case type::range:
        {
            range &r = selector_type->as<range>();
            if (!r.start)
                offset = 0;
            else
            {
                assert(r.start_is_constant());
                offset = r.const_start() - 1;
            }
            break;
        }
        default:
            throw runtime_error("Unexpected slice selector type.");
        }

        slicing.constants[current_dimension() + dim] = offset;

        ++dim;
    }

    expression *object = do_expression(object_node);

    update_accesses(object, slicing);

    return object;
}

expression * translator::do_mapping(const  ast::node_ptr &node)
{
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    auto result_type = node->semantic_type->as<semantic::stream>();

    vector<expression*> sources;

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();

        // TODO: range as source

        if (!iter.domain->semantic_type->is(semantic::type::stream))
            throw runtime_error("Unsupported mapping source.");

        semantic::stream source_type = iter.domain->semantic_type->as<semantic::stream>();
        expression *source_expr = do_expression(iter.domain);

        stream_view *source_stream;
        if (source_stream = dynamic_cast<stream_view*>(source_expr))
        {
            stream_view *copy = new stream_view;
            copy->target = source_stream->target;
            copy->pattern = access(source_stream);
            source_stream = copy;
        }
        else
        {
            vector<int> domain;
            domain.insert(domain.end(), m_domain.begin(), m_domain.end());
            domain.insert(domain.end(), source_type.size.begin(), source_type.size.end());
            source_stream = make_statement(source_expr, domain);
        }

        source_stream->current_iteration = current_dimension() + 1;

        sources.push_back(source_stream);
    }

    context::scope_holder mapping_scope(m_context);
    for(int i = 0; i < sources.size(); ++i)
    {
        semantic::iterator & iter =
                iterators_node->as_list()->elements[i]
                ->semantic_type->as<semantic::iterator>();
        m_context.bind(iter.id, sources[i]);
    }

    m_domain.push_back( result_type.size[0] );

    expression *result = do_block(body_node);

    m_domain.pop_back();

    return result;
}

mapping translator::access(stream_view *source)
{
    if (source->current_iteration == current_dimension())
    {
        return source->pattern;
    }

    int distance = current_dimension() - source->current_iteration;
    assert(distance > 0);

    int out_dim = source->pattern.output_dimension();
    int in_dim = source->pattern.input_dimension() + distance;
    mapping pattern;
    pattern.coefficients =
            source->pattern.coefficients.resized(out_dim, in_dim);
    pattern.constants = source->pattern.constants;

    auto & coef = pattern.coefficients;

    int col = coef.columns() - 1;
    for (; col >= current_dimension(); --col)
    {
        for (int r = 0; r < coef.rows(); ++r)
            coef(r, col) = coef(r, col - distance);
    }
    for (; col >= source->current_iteration; --col)
    {
        for (int r = 0; r < coef.rows(); ++r)
            coef(r, col) = 0;
    }

    return pattern;
}


stream_access * translator::complete_access( stream_view * view )
{
    auto access = new stream_access;
    access->target = view->target;
    access->pattern = this->access(view);

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
    view->pattern = mapping::identity(domain.size(), domain.size());
    view->current_iteration = current_dimension();

    return view;
}

void translator::update_accesses(expression *expr, const mapping & map )
{
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto sub_expr : operation->operands)
            update_accesses(sub_expr, map);
        return;
    }
    if (auto dependency = dynamic_cast<stream_access*>(expr))
    {
        //cout << "before: " << endl << dependency->pattern.coefficients;
        dependency->pattern = dependency->pattern * map;
        //cout << "after: " << endl  << dependency->pattern.coefficients;
        return;
    }
    if (auto view = dynamic_cast<stream_view*>(expr))
    {
        //cout << "before: " << endl << view->pattern.coefficients;
        view->pattern = view->pattern * map;
        //cout << "after: " << endl  << view->pattern.coefficients;
        return;
    }
}

}
}
