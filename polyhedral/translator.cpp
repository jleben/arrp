#include "translator.hpp"
#include "../utility/debug_source.hpp"

#include <stdexcept>
#include <algorithm>

namespace stream {
namespace polyhedral {

using namespace std;

numerical_type expr_type_for( const semantic::type_ptr t )
{
    switch(t->get_tag())
    {
    case semantic::type::integer_num:
        return integer;
    case semantic::type::real_num:
        return real;
    case semantic::type::range:
        return integer;
    case semantic::type::stream:
        return real;
    default:
        throw string("Unexpected type.");
    }
}

translator::translator(const semantic::environment &env):
    m_env(env)
{
    //m_context.enter_scope();
}

void translator::translate(const semantic::symbol & sym,
                           const vector<semantic::type_ptr> & args)
{
    using namespace semantic;

    expression * result = nullptr;

    switch(sym.type)
    {
    case semantic::symbol::expression:
    {
        result = do_block( sym.source_expression() );
        break;
    }
    case semantic::symbol::function:
    {
        context::scope_holder func_scope(m_context);

        assert(sym.parameter_names.size() == args.size());
        for(int i = 0; i < args.size(); ++i)
        {
            m_context.bind(sym.parameter_names[i], translate_input(args[i], i) );
        }

        result = do_block( sym.source_expression() );
        break;
    }
    }

    assert(result);

    type_ptr result_type = sym.source_expression()->semantic_type;

    vector<int> result_domain;

    switch(result_type->get_tag())
    {
    case type::integer_num:
    case type::real_num:
        result_domain.resize(1,1);
        break;
    case type::stream:
    {
        result_domain = result_type->as<stream>().size;
        break;
    }
    case type::range:
    {
        const semantic::range & range_type =
                result_type->as<semantic::range>();
        if (!range_type.is_constant())
            throw error("Non-constant range not supported as result type.");
        result_domain = { range_type.const_size() };
        break;
    }

    default:
        throw runtime_error("Unexpected type.");
    }

    result = iterate(result, result_type);
    auto stmt = make_statement(result, result_domain);
}

expression * translator::translate_input(const semantic::type_ptr & type,
                                         int index)
{
    switch(type->get_tag())
    {
    case semantic::type::integer_num:
        return new input_access(integer, index);
    case semantic::type::real_num:
        return new input_access(real, index);
    case semantic::type::stream:
    {
        auto & stream_type = type->as<semantic::stream>();

        statement *generator =
                make_statement(new input_access(real, index), stream_type.size);

        int dimension = generator->domain.size();

        auto expr = new stmt_view(generator);
        expr->pattern = mapping::identity(dimension, dimension);
        expr->current_iteration = 0;
        return expr;
    }
    default:
        throw std::runtime_error("Translation: Unexpected type.");
    }
}

void translator::do_statement_list(const ast::node_ptr &node)
{
    for ( const sp<ast::node> & stmt : node->as_list()->elements )
        do_statement(stmt);
}

expression* translator::do_statement(const ast::node_ptr &node)
{
    ast::list_node *stmt = node->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & body_node = stmt->elements[2];

    // Skip function definitions -
    // They are given as semantic types at function calls
    if (params_node)
        return nullptr;

    const string & id = id_node->as_leaf<string>()->value;

    expression *expr = do_block(body_node);

    if(!dynamic_cast<stmt_view*>(expr))
    {
        using namespace semantic;

        vector<int> domain = m_domain;
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

        if (!domain.empty())
        {
            expr = make_current_view( make_statement(expr, domain) );
        }
        else
        {
            domain.push_back(1);
            auto stmt = make_statement(expr, domain);
            auto view = new stmt_view(stmt);
            view->pattern = mapping(1,1);
            view->current_iteration = 0;
            expr = view;
        }
    }

    m_context.bind(id, expr);

    return expr;
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
    switch(node->semantic_type->get_tag())
    {
    case semantic::type::integer_num:
    {
        const semantic::integer_num &num = node->semantic_type->as<semantic::integer_num>();
        if (num.is_constant())
            return  new constant<int>(num.constant_value());
        break;
    }
    case semantic::type::real_num:
    {
        const semantic::real_num &num = node->semantic_type->as<semantic::real_num>();
        if (num.is_constant())
            return  new constant<double>(num.constant_value());
        break;
    }
    default:;
    }

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
    case ast::range:
    {
        const semantic::range &type = node->semantic_type->as<semantic::range>();
        range *r = new range;

        if (type.start_is_constant())
            r->start = new constant<int>(type.const_start());
        else if (node->as_list()->elements[0])
            r->start = do_expression(node->as_list()->elements[0]);
        else
            r->start = nullptr;

        if (type.end_is_constant())
            r->end = new constant<int>(type.const_end());
        else if (node->as_list()->elements[1])
            r->end = do_expression(node->as_list()->elements[1]);
        else
            r->end = nullptr;

        return r;
    }
    case ast::negate:
    {
        return do_unary_op(node);
    }
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::divide_integer:
    case ast::raise:
    {
        return do_binary_op(node);
    }
    case ast::call_expression:
    {
        return do_call(node);
    }
    case ast::transpose_expression:
    {
        return do_transpose(node);
    }
    case ast::slice_expression:
    {
        return do_slicing(node);
    }
    case ast::hash_expression:
    {
        assert(node->semantic_type->is(semantic::type::integer_num));
        const auto & integer = node->semantic_type->as<semantic::integer_num>();
        return new constant<int>(integer.constant_value());
    }
    case ast::for_expression:
    {
        return do_mapping(node);
    }
    case ast::reduce_expression:
    {
        return do_reduction(node);
    }

    default:
        throw std::runtime_error("Unexpected AST node type.");
    }
}

expression * translator::do_identifier(const ast::node_ptr &node)
{
    string id = node->as_leaf<string>()->value;

    auto context_item = m_context.find(id);
    if (context_item)
        return context_item.value().source;

    // Clear domain
    vector<int> local_domain;
    std::swap(local_domain, m_domain);

    // Process statement
    const semantic::symbol & sym = m_env.at(id);
    assert(sym.source->type == ast::statement);
    expression *result = do_statement(sym.source);

    // Restore domain
    std::swap(m_domain, local_domain);

    return result;
}

expression * translator::do_call(const ast::node_ptr &node)
{
    static unordered_map<string,intrinsic::of_kind> intrinsics =
    {
        {"log", intrinsic::log},
        {"log2", intrinsic::log2},
        {"log10", intrinsic::log10},
        {"exp", intrinsic::exp},
        {"exp2", intrinsic::exp2},
        {"pow", intrinsic::raise},
        {"sqrt", intrinsic::sqrt},
        {"sin", intrinsic::sin},
        {"cos", intrinsic::cos},
        {"tan", intrinsic::tan},
        {"asin", intrinsic::asin},
        {"acos", intrinsic::acos},
        {"atan", intrinsic::atan},
        {"ceil", intrinsic::ceil},
        {"floor", intrinsic::floor},
        {"abs", intrinsic::abs},
        {"min", intrinsic::min},
        {"max", intrinsic::max}
    };

    ast::list_node * call = node->as_list();
    const auto & id_node = call->elements[0];
    const auto & args_node = call->elements[1];

    // Get callee id
    assert(id_node->type == ast::identifier);
    const string & id = id_node->as_leaf<string>()->value;

    // Process args
    std::vector<expression*> args;
    for (const auto & arg_node : args_node->as_list()->elements)
    {
        args.push_back( do_expression(arg_node) );
    }

    // Try intrinsic
    auto intrinsic_map = intrinsics.find(id);
    if (intrinsic_map != intrinsics.end())
    {
        for (expression *& arg : args)
            arg = iterate(arg, node->semantic_type);

        intrinsic::of_kind intrinsic_kind = intrinsic_map->second;

        intrinsic *expr = new intrinsic
                ( expr_type_for(node->semantic_type),
                  intrinsic_kind, args );

        return expr;
    }

    // Try user function
    auto func = dynamic_cast<semantic::function*>(id_node->semantic_type.get());
    assert(func->parameters.size() == args.size());

    context::scope_holder func_scope(m_context);

    for (int a = 0; a < args.size(); ++a)
        m_context.bind(func->parameters[a], args[a]);

    expression * result = do_block(func->expression());

    return result;
}

expression * translator::do_unary_op(const ast::node_ptr &node)
{
    expression *operand = iterate
            ( do_expression(node->as_list()->elements[0]),
            node->semantic_type );

    // create operation

    auto operation_result =
            new intrinsic(expr_type_for(node->semantic_type));

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
}

expression * translator::do_binary_op(const ast::node_ptr &node)
{
    expression *operand1 = iterate( do_expression(node->as_list()->elements[0]),
            node->semantic_type );
    expression *operand2 = iterate( do_expression(node->as_list()->elements[1]),
            node->semantic_type );

    // create operation

    auto operation_result =
            new intrinsic(expr_type_for(node->semantic_type));

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
    case ast::divide_integer:
        operation_result->kind = intrinsic::divide_integer;
        break;
    case ast::raise:
        operation_result->kind = intrinsic::raise;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    return operation_result;
}

expression * translator::do_transpose(const ast::node_ptr &node)
{
    const auto & object_node = node->as_list()->elements[0];
    const auto & dims_node = node->as_list()->elements[1];

    semantic::stream & object_type =
            object_node->semantic_type->as<semantic::stream>();

    int dimension = object_type.dimensionality();

    vector<int> order(dimension,-1);
    vector<bool> used_dims(dimension,false);

    int in_dim = 0;
    for ( const auto & dim_node : dims_node->as_list()->elements )
    {
        int out_dim = dim_node->as_leaf<int>()->value - 1;
        order[out_dim] = in_dim;
        ++in_dim;
    }
    for(int out_dim = 0; out_dim < dimension; ++out_dim)
    {
        if (order[out_dim] < 0)
        {
            order[out_dim] = in_dim;
            ++in_dim;
        }
    }
    assert(in_dim == dimension);

    mapping transposition = mapping::identity(dimension, dimension);
    transposition.coefficients = transposition.coefficients.reordered( order );

    expression *object = do_expression(object_node);

    object = update_accesses(object, transposition);

    return object;
}

expression * translator::do_slicing(const  ast::node_ptr &node)
{
    using namespace semantic;

    const auto & object_node = node->as_list()->elements[0];
    const auto & ranges_node = node->as_list()->elements[1];

    const auto & object_type =
            object_node->semantic_type->as<semantic::stream>();
    const auto & result_type = node->semantic_type;

    int object_dimension, slice_dimension;

    object_dimension = object_type.dimensionality();

    if (result_type->is(semantic::type::stream))
        slice_dimension = result_type->as<semantic::stream>().dimensionality();
    else
        slice_dimension = 1;

    mapping slicing =
            mapping(slice_dimension,
                    object_dimension);

    int in_dim = 0;
    int out_dim = 0;

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
            // FIXME: do not assume range size > 1 !
            semantic::range &r = selector_type->as<semantic::range>();
            if (!r.start)
                offset = 0;
            else
            {
                assert(r.start_is_constant());
                offset = r.const_start() - 1;
            }

            slicing.coefficients(out_dim, in_dim) = 1;
            ++in_dim;

            break;
        }
        default:
            throw runtime_error("Unexpected slice selector type.");
        }

        slicing.constants[out_dim] += offset;

        ++out_dim;
    }

    for (; in_dim < slice_dimension; ++in_dim, ++out_dim)
    {
        slicing.coefficients(out_dim, in_dim) = 1;
    }

    expression *object = do_expression(object_node);

    object = update_accesses(object, slicing);

    return object;
}

expression * translator::do_mapping(const  ast::node_ptr &node)
{
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    auto & result_type = node->semantic_type->as<semantic::stream>();

    vector<expression*> sources;

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();

        expression *source_expr = do_expression(iter.domain);

        if(range *r = dynamic_cast<range*>(source_expr))
        {
            iterator_access *it = iterate(r);
            it->ratio = iter.hop;

            if (iter.size > 1)
            {
                throw error("Iterating range with slice size > 1 not yet implemented.");
            }

            sources.push_back(it);
            continue;
        }

        assert(iter.domain->semantic_type->is(semantic::type::stream));
        semantic::stream & source_type =
                iter.domain->semantic_type->as<semantic::stream>();

        stmt_view *source_stream;
        if (source_stream = dynamic_cast<stmt_view*>(source_expr))
        {
            stmt_view *copy = new stmt_view(source_stream->target);
            copy->pattern = access(source_stream);
            source_stream = copy;
        }
        else
        {
            vector<int> domain = m_domain;
            domain.insert(domain.end(), source_type.size.begin(), source_type.size.end());
            source_stream = make_current_view( make_statement(source_expr, domain) );
        }

        // Expand

        if (iter.size > 1)
        {
            const mapping &src = source_stream->pattern;
            mapping dst(src.input_dimension() + 1, src.input_dimension());
            for (int dim = 0; dim <= current_dimension(); ++dim)
            {
                dst.coefficients(dim, dim) = 1;
            }
            for (int dim = current_dimension(); dim < dst.output_dimension(); ++dim)
            {
                dst.coefficients(dim, dim + 1) = 1;
            }
            source_stream->pattern = source_stream->pattern * dst;
        }

        // Apply stride

        mapping stride =
                mapping::identity(source_stream->pattern.input_dimension(),
                                  source_stream->pattern.input_dimension());
        stride.coefficients(current_dimension(), current_dimension()) = iter.hop;
        source_stream->pattern = source_stream->pattern * stride;

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

    if (stmt_view *view = dynamic_cast<stmt_view*>(result))
    {
        view->pattern = access(view);
        view->current_iteration = current_dimension() - 1;
        assert(view->current_iteration >= 0);
    }

    m_domain.pop_back();

    result->type = polyhedral::real;

    return result;
}

expression * translator::do_reduction(const  ast::node_ptr &node)
{
    const auto & accum_node = node->as_list()->elements[0];
    const auto & iter_node = node->as_list()->elements[1];
    const auto & source_node = node->as_list()->elements[2];
    const auto & body_node = node->as_list()->elements[3];

    if (!source_node->semantic_type->is(semantic::type::stream))
        throw runtime_error("Unsupported reduction source.");

    semantic::stream & source_type = source_node->semantic_type->as<semantic::stream>();
    assert(source_type.size.size() == 1);
    assert(source_type.size.front() > 0);

    expression *source_expr = do_expression(source_node);

    stmt_view *source_stmt_view;
    if (source_stmt_view = dynamic_cast<stmt_view*>(source_expr))
    {
        stmt_view *copy = new stmt_view(source_stmt_view->target);
        copy->pattern = access(source_stmt_view);
        copy->current_iteration = current_dimension();
        source_stmt_view = copy;
    }
    else
    {
        vector<int> domain = m_domain;
        domain.insert(domain.end(), source_type.size.begin(), source_type.size.end());
        source_stmt_view = make_current_view( make_statement(source_expr, domain) );
    }

    vector<int> result_domain = m_domain;
    if (result_domain.empty())
        result_domain.push_back(1);

    stmt_access *init_access = new stmt_access(source_stmt_view->target);
    {
        init_access->pattern = source_stmt_view->pattern;
        mapping &m = init_access->pattern;
        if (m.input_dimension() > 1)
        {
            m.coefficients = m.coefficients.resized(m.coefficients.rows(),
                                                    m.coefficients.columns()-1);
        }
        else
        {
            for (int d = 0; d < m.output_dimension(); ++d)
                m.coefficient(m.input_dimension()-1,d) = 0;
        }
    }
    statement *init_stmt = make_statement(init_access, result_domain);

    reduction_access *accumulator = new reduction_access(init_stmt->expr->type);
    accumulator->initializer = init_stmt;

    stmt_view *iterator = source_stmt_view;
    iterator->current_iteration = current_dimension() + 1;
    {
        int dims = iterator->pattern.input_dimension();
        assert(iterator->current_iteration == dims);
        mapping offset = mapping::identity(dims, dims);
        offset.constant(dims - 1) = 1;
        iterator->pattern = iterator->pattern * offset;
    }

    string accum_id = accum_node->as_leaf<string>()->value;
    string iter_id = iter_node->as_leaf<string>()->value;
    m_context.bind( accum_id, accumulator );
    m_context.bind( iter_id, iterator );

    // Expand domain
    int original_domain_size = m_domain.size();
    m_domain.push_back(source_type.size.front() - 1);

    expression *reduction_expr = do_block(body_node);
    statement *reduction_stmt = make_statement(reduction_expr, m_domain);
    accumulator->reductor = reduction_stmt;

    assert(init_stmt->expr->type == reduction_stmt->expr->type);

    // Restore domain
    m_domain.resize(original_domain_size);

    // Create a view with fixed mapping to last element of reduction:
    auto result = new stmt_view(reduction_stmt);
    result->pattern = mapping::identity(result_domain.size(),
                                        reduction_stmt->domain.size());
    if (current_dimension() < result_domain.size())
        result->pattern.coefficient(current_dimension(),
                                    current_dimension()) = 0;
    result->pattern.constant(current_dimension())
            = reduction_stmt->domain.back() - 1;
    result->current_iteration = current_dimension();

    return result;
}

mapping translator::access(stmt_view *source, int padding)
{
    assert(padding >= 0);

    if (source->current_iteration == current_dimension() && padding == 0)
    {
        return source->pattern;
    }

    int distance = current_dimension() - source->current_iteration;
    assert(distance >= 0);
    int out_dim = source->pattern.output_dimension();
    int in_dim = source->pattern.input_dimension() + distance + padding;

    mapping pattern;
    pattern.coefficients =
            source->pattern.coefficients.resized(out_dim, in_dim);
    pattern.constants = source->pattern.constants;

    auto & coef = pattern.coefficients;

    int col = coef.columns() - padding - 1;
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

expression *translator::iterate (expression *expr, const semantic::type_ptr & result_type)
{
    if (auto rng = dynamic_cast<range*>(expr))
        return iterate(rng);
    else if (auto strm = dynamic_cast<stmt_view*>(expr))
        return complete_access(strm, result_type);
    else
        return expr;
}

iterator_access * translator::iterate( range *r )
{
    constant<int> *start, *end;
    assert(start = dynamic_cast<constant<int>*>(r->start));
    assert(end = dynamic_cast<constant<int>*>(r->end));

    iterator_access *it = new iterator_access(r->type);
    it->dimension = current_dimension();
    it->offset = start->value;
    it->ratio = 1;
    return it;
}

stmt_access * translator::complete_access
( stmt_view * view, const semantic::type_ptr & result_type)
{
    int padding = 0;
    if (result_type->is(semantic::type::stream))
    {
        padding = result_type->as<semantic::stream>().dimensionality() -
                (view->pattern.input_dimension() - view->current_iteration);
    }
    assert(padding >= 0);

    auto access = new stmt_access(view->target);
    access->pattern = this->access(view, padding);

    return access;
}

statement *
translator::make_statement( expression * expr,
                            const vector<int> & domain )
{
    ostringstream name;
    name << "S_" << m_statements.size();

    auto stmt = new statement;
    m_statements.push_back(stmt);
    stmt->domain = domain;
    stmt->expr = expr;
    stmt->name = name.str();

    return stmt;
}

translator::stmt_view *
translator::make_current_view( statement * stmt )
{
    auto view = new stmt_view(stmt);
    view->pattern = mapping::identity(stmt->domain.size(), stmt->domain.size());
    view->current_iteration = current_dimension();
    return view;
}

expression * translator::update_accesses(expression *expr, const mapping & map )
{
    if (auto operation = dynamic_cast<intrinsic*>(expr))
    {
        for (auto & sub_expr : operation->operands)
        {
            sub_expr = update_accesses(sub_expr, map);
        }
        return expr;
    }
    if (auto dependency = dynamic_cast<stmt_access*>(expr))
    {
        // FIXME: duplicate, for the sake of consistency with stream_view below.

        //cout << "before: " << endl << dependency->pattern;
        dependency->pattern = dependency->pattern * map;
        //cout << "after: " << endl  << dependency->pattern;
        return expr;
    }
    if (auto view = dynamic_cast<stmt_view*>(expr))
    {
        //cout << "Applying map:" << endl << map;

        int d = view->pattern.input_dimension() - map.output_dimension();
        assert(d >= 0);
        mapping m2 = mapping::identity(map.input_dimension() + d,
                                       map.output_dimension() + d);

        for (int out = 0; out < map.output_dimension(); ++out)
        {
            int out2 = d + out;
            m2.constant(out2) = map.constant(out);
            for(int in = 0; in < map.input_dimension(); ++in)
            {
                int in2 = d + in;
                m2.coefficient(in2, out2) = map.coefficient(in,out);
            }
        }

        //cout << "Transform:" << endl << m2;
        //cout << "Base:" << endl << view->pattern;

        // FIXME: Only duplicate if shared (avoid memory leak).

        auto new_view = new stmt_view(view->target);
        new_view->pattern = view->pattern * m2;
        new_view->current_iteration = view->current_iteration;
        //cout << "Result: " << endl  << new_view->pattern;
        return new_view;
    }
    if ( dynamic_cast<constant<int>*>(expr) ||
         dynamic_cast<constant<double>*>(expr) )
    {
        return expr;
    }
    throw std::runtime_error("Unexpected expression type.");
}

}
}
