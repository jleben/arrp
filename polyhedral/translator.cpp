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

#include "translator.hpp"
#include "../common/error.hpp"

#include <stdexcept>
#include <algorithm>

namespace stream {
namespace polyhedral {

using namespace std;
using semantic::primitive_type_for;

translator::translator(const semantic::environment &env):
    m_env(env)
{
    //m_context.enter_scope();
}

void translator::translate(const semantic::symbol & sym,
                           const vector<semantic::type_ptr> & args)
{
    using namespace semantic;

    assert(m_context.level() == 0);
    context::scope_holder root_scope(m_context);
    if (debug::is_enabled())
        cout << "[poly] Entering root scope." << endl;

    expression * result = nullptr;

    switch(sym.type)
    {
    case semantic::symbol::expression:
    {
        if (debug::is_enabled())
            cout << "[poly] Entering expression scope: " << sym.name << endl;

        context::scope_holder expr_scope(m_context);

        result = do_block( sym.source_expression() );

        if (debug::is_enabled())
            cout << "[poly] Exiting expression scope: "  << sym.name << endl;

        break;
    }
    case semantic::symbol::function:
    {
        if (debug::is_enabled())
            cout << "[poly] Entering function scope: " << sym.name << endl;
        context::scope_holder func_scope(m_context);

        assert(sym.parameter_names.size() == args.size());
        for(int i = 0; i < args.size(); ++i)
        {
            m_context.bind(sym.parameter_names[i], translate_input(args[i], i) );
        }

        result = do_block( sym.source_expression() );

        if (debug::is_enabled())
            cout << "[poly] Exiting function scope: "  << sym.name << endl;

        break;
    }
    }

    assert(result);

    type_ptr result_type = sym.source_expression()->semantic_type;
    auto result_type_struct = structure(result_type);

    vector<int> result_domain = result_type_struct.size;

    if (result_domain.empty())
        result_domain = {1};

    result = iterate(result, result_type);
    make_statement(result, result_domain);

    if (debug::is_enabled())
        cout << "[poly] Exiting root scope." << endl;

    // Check flow dimensions;

    for(statement *stmt : m_statements)
    {
        int flow_dim = -1;
        for(int dim = 0; dim < stmt->domain.size(); ++dim)
        {
            if (stmt->domain[dim] == infinite)
            {
                if (flow_dim >= 0)
                {
                    throw error("Statement infinite in multiple dimensions.");
                }
                flow_dim = dim;
            }
        }
        stmt->flow_dim = flow_dim;
    }

    for(array_ptr a : m_arrays)
    {
        int flow_dim = -1;
        for(int dim = 0; dim < a->size.size(); ++dim)
        {
            if (a->size[dim] == infinite)
            {
                if (flow_dim >= 0)
                {
                    throw error("Array infinite in multiple dimensions.");
                }
                flow_dim = dim;
            }
        }
        a->flow_dim = flow_dim;
    }

    // Add output statement;

    assert(!m_statements.empty());
    statement *last_stmt = m_statements.back();
    if (last_stmt->flow_dim >= 0)
    {
        auto src = new array_access(last_stmt->array);
        // NOTE: special pattern will be fixed later
        auto stmt = make_statement();
        stmt->domain = { infinite };
        stmt->flow_dim = 0;
        stmt->expr = src;
        // NOTE: no array
    }
}

expression * translator::translate_input(const semantic::type_ptr & type,
                                         int index)
{
    switch(type->get_tag())
    {
    case semantic::type::boolean:
        return new input_access(primitive_type::boolean, index);
    case semantic::type::integer_num:
        return new input_access(primitive_type::integer, index);
    case semantic::type::real_num:
        return new input_access(primitive_type::real, index);
    case semantic::type::stream:
    {
        auto & stream_type = type->as<semantic::stream>();

        array_ptr array = make_array(primitive_type_for(type));
        array->size = stream_type.size;

        statement *generator = make_statement();
        generator->expr = new input_access(array->type, index);
        generator->domain = array->size;
        generator->array = array;
        generator->write_relation = mapping::identity(array->size.size());

        auto expr = new array_view(array);
        expr->pattern = mapping::identity(array->size.size());
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
    {
        do_statement(stmt);
        if (stmt->semantic_type->is(semantic::type::function))
            m_context.enter_scope();
    }
}

expression* translator::do_statement(const ast::node_ptr &node)
{
    ast::list_node *stmt = node->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & body_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    if (params_node)
    {
        // Function definition is obtained as semantic type at function call
        m_context.bind(id, symbol(nullptr));
        if (debug::is_enabled())
            cout << "[poly] Binding: " << id << endl;
        return nullptr;
    }

    expression *expr = do_block(body_node);

    if(!dynamic_cast<array_view*>(expr))
    {
        using namespace semantic;

        vector<int> domain = m_domain;

        if (!node->semantic_type->is_scalar())
        {
            assert(node->semantic_type->is(type::stream));
            auto & s = node->semantic_type->as<stream>();
            domain.insert(domain.end(), s.size.begin(), s.size.end());
        }

        expr = iterate(expr, node->semantic_type);

        if (!domain.empty())
        {
            expr = make_current_view( make_statement(expr, domain)->array );
        }
        else
        {
            domain.push_back(1);
            auto stmt = make_statement(expr, domain);
            auto view = new array_view(stmt->array);
            view->pattern = mapping(1,1);
            view->current_iteration = 0;
            expr = view;
        }
    }

    if (debug::is_enabled())
    {
        cout << "[poly] Binding: " << id << endl;
    }

    m_context.bind(id, expr);

    return expr;
}

expression * translator::do_block(const ast::node_ptr &node)
{
    ast::list_node *expr_block = node->as_list();
    const auto & stmt_list = expr_block->elements[0];
    const auto & expr = expr_block->elements[1];

    int base_ctx_level = m_context.level();

    if (stmt_list)
    {
        try
        {
            do_statement_list(stmt_list);
        }
        catch(...)
        {
            m_context.roll_back_to(base_ctx_level);
            throw;
        }
    }

    expression *result = do_expression(expr);

    m_context.roll_back_to(base_ctx_level);

    return result;
}

expression * translator::do_expression(const ast::node_ptr &node)
{
    // Make use of propagated constants:

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
    case semantic::type::boolean:
    {
        const semantic::boolean & b = node->semantic_type->as<semantic::boolean>();
        if (b.is_constant())
            return  new constant<bool>(b.constant_value());
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
    case ast::boolean:
    {
        bool value = node->as_leaf<bool>()->value;
        return new constant<bool>(value);
    }
    case ast::range:
    {
        const auto & start_node = node->as_list()->elements[0];
        const auto & end_node = node->as_list()->elements[1];
        assert(start_node);
        assert(end_node);

        assert(start_node->semantic_type->is(semantic::type::integer_num));
        assert(end_node->semantic_type->is(semantic::type::integer_num));

        auto & start_int_t = start_node->semantic_type->as<semantic::integer_num>();
        auto & end_int_t = end_node->semantic_type->as<semantic::integer_num>();
        assert(start_int_t.is_constant());
        assert(end_int_t.is_constant());

        range *r = new range;
        r->start = start_int_t.constant_value();
        r->end = end_int_t.constant_value();

        return r;
    }
    case ast::negate:
    case ast::oppose:
    {
        return do_unary_op(node);
    }
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::divide_integer:
    case ast::modulo:
    case ast::raise:
    case ast::greater:
    case ast::greater_or_equal:
    case ast::lesser:
    case ast::lesser_or_equal:
    case ast::equal:
    case ast::not_equal:
    case ast::logic_and:
    case ast::logic_or:
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
    case ast::if_expression:
    {
        return do_conditional(node);
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

    if (debug::is_enabled())
    {
        cout << "[poly] Identifier: " << id << endl;
    }

    auto context_item = m_context.find(id);
    if (context_item)
        return context_item.value().source;

    if (debug::is_enabled())
        cout << "[poly] Entering root scope for id: " << id << endl;

    context::scope_holder root_scope(m_context, m_context.root_scope());

    // Clear domain
    vector<int> local_domain;
    std::swap(local_domain, m_domain);

    // Process statement
    const semantic::symbol & sym = m_env.at(id);
    assert(sym.source->type == ast::statement);
    expression *result = do_statement(sym.source);

    // Restore domain
    std::swap(m_domain, local_domain);

    if (debug::is_enabled())
        cout << "[poly] Exiting root scope for id: " << id << endl;

    return result;
}

expression * translator::do_call(const ast::node_ptr &node)
{
    static unordered_map<string, primitive_op> primitive_ops =
    {
        {"log", primitive_op::log},
        {"log2", primitive_op::log2},
        {"log10", primitive_op::log10},
        {"exp", primitive_op::exp},
        {"exp2", primitive_op::exp2},
        {"pow", primitive_op::raise},
        {"sqrt", primitive_op::sqrt},
        {"sin", primitive_op::sin},
        {"cos", primitive_op::cos},
        {"tan", primitive_op::tan},
        {"asin", primitive_op::asin},
        {"acos", primitive_op::acos},
        {"atan", primitive_op::atan},
        {"ceil", primitive_op::ceil},
        {"floor", primitive_op::floor},
        {"abs", primitive_op::abs},
        {"min", primitive_op::min},
        {"max", primitive_op::max}
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

    // Try primitive
    auto primitive_op_mapping = primitive_ops.find(id);
    if (primitive_op_mapping != primitive_ops.end())
    {
        for (expression *& arg : args)
            arg = iterate(arg, node->semantic_type);

        primitive_op op = primitive_op_mapping->second;

        primitive_expr *expr = new primitive_expr
                ( primitive_type_for(node->semantic_type),
                  op, args );

        return expr;
    }

    // Try user function
    auto func = dynamic_cast<semantic::function*>(id_node->semantic_type.get());
    assert(func->parameters.size() == args.size());

    context::scope_iterator parent_scope;
    if (context::item local_func = m_context.find(id))
    {
        parent_scope = local_func.scope();
        if (debug::is_enabled())
            cout << "[poly] Entering local scope for call to: " << id << endl;
    }
    else
    {
        parent_scope = m_context.root_scope();
        if (debug::is_enabled())
            cout << "[poly] Entering root scope for call to: " << id << endl;
    }

    context::scope_holder func_scope(m_context, parent_scope);

    for (int a = 0; a < args.size(); ++a)
        m_context.bind(func->parameters[a], args[a]);

    expression * result = do_block(func->expression());

    if (debug::is_enabled())
        cout << "[poly] Exiting scope for call to: " << id << endl;

    return result;
}

expression * translator::do_unary_op(const ast::node_ptr &node)
{
    expression *operand = iterate
            ( do_expression(node->as_list()->elements[0]),
            node->semantic_type );

    // create operation

    auto operation_result =
            new primitive_expr(primitive_type_for(node->semantic_type));

    operation_result->operands.push_back(operand);

    switch(node->type)
    {
    case ast::negate:
    case ast::oppose:
        operation_result->op = primitive_op::negate;
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
            new primitive_expr(primitive_type_for(node->semantic_type));

    operation_result->operands.push_back(operand1);
    operation_result->operands.push_back(operand2);

    switch(node->type)
    {
    case ast::add:
        operation_result->op = primitive_op::add;
        break;
    case ast::subtract:
        operation_result->op = primitive_op::subtract;
        break;
    case ast::multiply:
        operation_result->op = primitive_op::multiply;
        break;
    case ast::divide:
        operation_result->op = primitive_op::divide;
        break;
    case ast::divide_integer:
        operation_result->op = primitive_op::divide_integer;
        break;
    case ast::modulo:
        operation_result->op = primitive_op::modulo;
        break;
    case ast::raise:
        operation_result->op = primitive_op::raise;
        break;
    case ast::lesser:
        operation_result->op = primitive_op::compare_l;
        break;
    case ast::lesser_or_equal:
        operation_result->op = primitive_op::compare_leq;
        break;
    case ast::greater:
        operation_result->op = primitive_op::compare_g;
        break;
    case ast::greater_or_equal:
        operation_result->op = primitive_op::compare_geq;
        break;
    case ast::equal:
        operation_result->op = primitive_op::compare_eq;
        break;
    case ast::not_equal:
        operation_result->op = primitive_op::compare_neq;
        break;
    case ast::logic_and:
        operation_result->op = primitive_op::logic_and;
        break;
    case ast::logic_or:
        operation_result->op = primitive_op::logic_or;
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
    const auto & selectors_node = node->as_list()->elements[1];

    const auto & object_type =
            object_node->semantic_type->as<semantic::stream>();
    const auto & result_type = node->semantic_type;

    int object_dimension, slice_dimension;

    object_dimension = object_type.dimensionality();

    if (result_type->is(semantic::type::stream))
        slice_dimension = result_type->as<semantic::stream>().dimensionality();
    else
        slice_dimension = 0;

    mapping slicing =
            mapping(slice_dimension,
                    object_dimension);

    int in_dim = 0;
    int out_dim = 0;

    for( const auto & selector_node : selectors_node->as_list()->elements )
    {
        int offset;

        // TODO: detect affine expressions of loop indeces and parameters
        // auto selector = do_expression(range_node);

        switch(selector_node->type)
        {
        case ast::range:
        {
            const auto & start_node = selector_node->as_list()->elements[0];
            const auto & end_node = selector_node->as_list()->elements[1];

            int start;
            int end;

            if (start_node)
            {
                assert(start_node->semantic_type->is(semantic::type::integer_num));
                auto & start_int_t = start_node->semantic_type->as<semantic::integer_num>();
                assert(start_int_t.is_constant());
                start = start_int_t.constant_value();
            }
            else
            {
                start = 1;
            }

            if (end_node)
            {
                assert(end_node->semantic_type->is(semantic::type::integer_num));
                auto & end_int_t = end_node->semantic_type->as<semantic::integer_num>();
                assert(end_int_t.is_constant());
                end = end_int_t.constant_value();
            }
            else
            {
                end = object_type.size[out_dim];
            }

            offset = start - 1;

            int size = end - start + 1;
            assert(size > 0);
            if (size > 1)
            {
                slicing.coefficients(out_dim, in_dim) = 1;
                ++in_dim;
            }
        }
        default:
            assert(selector_node->semantic_type->is(semantic::type::integer_num));
            auto & offset_int_t = selector_node->semantic_type->as<semantic::integer_num>();
            assert(offset_int_t.is_constant());
            offset = offset_int_t.constant_value() - 1;
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

expression * translator::do_conditional(const  ast::node_ptr &node)
{
    const auto & condition_node = node->as_list()->elements[0];
    const auto & true_node = node->as_list()->elements[1];
    const auto & false_node = node->as_list()->elements[2];

    assert(condition_node->semantic_type->is_scalar());

    // TODO: might wanna make a statement for the condition, to avoid
    // evaluating it for every item of true/false expression streams

    expression *condition = iterate(do_expression(condition_node), node->semantic_type);
    expression *true_expr = iterate(do_block(true_node), node->semantic_type);
    expression *false_expr = iterate(do_block(false_node), node->semantic_type);

    auto result = new primitive_expr(primitive_type_for(node->semantic_type));
    result->operands = { condition, true_expr, false_expr };
    result->op = primitive_op::conditional;

    return result;
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

        assert(iter.domain->semantic_type->is(semantic::type::stream));
        semantic::stream & source_type =
                iter.domain->semantic_type->as<semantic::stream>();

        array_view *source_stream;
        if ((source_stream = dynamic_cast<array_view*>(source_expr)))
        {
            array_view *copy = new array_view(source_stream->target);
            copy->pattern = access(source_stream);
            source_stream = copy;
        }
        else
        {
            vector<int> domain = m_domain;
            domain.insert(domain.end(), source_type.size.begin(), source_type.size.end());
            if (auto r = dynamic_cast<range*>(source_expr))
                source_expr = iterate(r);
            source_stream = make_current_view( make_statement(source_expr, domain)->array );
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

    if (array_view *view = dynamic_cast<array_view*>(result))
    {
        view->pattern = access(view);
        view->current_iteration = current_dimension() - 1;
        assert(view->current_iteration >= 0);
    }

    m_domain.pop_back();

    result->type = result_type.element_type;

    return result;
}

expression * translator::do_reduction(const  ast::node_ptr &node)
{
    const auto & accum_node = node->as_list()->elements[0];
    const auto & iter_node = node->as_list()->elements[1];
    const auto & source_node = node->as_list()->elements[2];
    const auto & body_node = node->as_list()->elements[3];

    assert(source_node->semantic_type->is(semantic::type::stream));

    semantic::stream & source_stream = source_node->semantic_type->as<semantic::stream>();
    assert(source_stream.size.size() == 1);
    assert(source_stream.size.front() > 0);

    expression *source_expr = do_expression(source_node);

    array_view *source_array_view;
    if ((source_array_view = dynamic_cast<array_view*>(source_expr)))
    {
        array_view *copy = new array_view(source_array_view->target);
        copy->pattern = access(source_array_view);
        copy->current_iteration = current_dimension();
        source_array_view = copy;
    }
    else
    {
        vector<int> domain = m_domain;
        domain.insert(domain.end(), source_stream.size.begin(), source_stream.size.end());
        if (auto r = dynamic_cast<range*>(source_expr))
            source_expr = iterate(r);
        source_array_view = make_current_view( make_statement(source_expr, domain)->array );
    }

    vector<int> result_domain = m_domain;
    if (result_domain.empty())
        result_domain.push_back(1);

    auto reduct_array = make_array(primitive_type_for(node->semantic_type));
    reduct_array->size = m_domain;
    reduct_array->size.push_back(source_stream.size.front());

    array_access *iterator0 = new array_access(source_array_view->target);
    {
        iterator0->pattern = source_array_view->pattern;
        mapping &m = iterator0->pattern;
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
    statement *init_stmt = make_statement();
    init_stmt->expr = iterator0;
    init_stmt->domain = result_domain;
    init_stmt->array = reduct_array;
    init_stmt->write_relation = mapping::identity(result_domain.size(),
                                                  result_domain.size() + 1);

    array_access *iterator = new array_access(source_array_view->target);
    {
        iterator->pattern = source_array_view->pattern;

        // Offset by 1 in last (= reduction) dim
        int in_dims = iterator->pattern.input_dimension();
        mapping offset = mapping::identity(in_dims);
        offset.constants.back() = 1;

        iterator->pattern = iterator->pattern * offset;
    }

    array_access *accumulator = new array_access(reduct_array);
    {
        accumulator->pattern = mapping::identity(reduct_array->size.size());
        // Offset by -1 in last (= reduction) dim
        //accumulator->pattern.constants.back() = -1;
    }

    statement *reduction_stmt;
    {
        context::scope_holder reduction_scope(m_context);

        string accum_id = accum_node->as_leaf<string>()->value;
        string iter_id = iter_node->as_leaf<string>()->value;
        m_context.bind( accum_id, accumulator );
        m_context.bind( iter_id, iterator );

        // Expand domain
        int original_domain_size = m_domain.size();
        m_domain.push_back(source_stream.size.front() - 1);

        expression *reduction_expr = do_block(body_node);

        reduction_stmt = make_statement();
        reduction_stmt->expr = reduction_expr;
        reduction_stmt->domain = m_domain;
        reduction_stmt->array = reduct_array;
        reduction_stmt->write_relation = mapping::identity(m_domain.size());
        reduction_stmt->write_relation.constants.back() = 1;

        // Restore domain
        m_domain.resize(original_domain_size);
    }

    assert(init_stmt->expr->type == reduct_array->type);
    assert(reduction_stmt->expr->type == reduct_array->type);

    // Create a view with fixed mapping to last element of reduction:
    auto result = new array_view(reduct_array);
    result->pattern = mapping::identity(result_domain.size(),
                                        reduct_array->size.size());
    if (current_dimension() < result_domain.size())
        result->pattern.coefficient(current_dimension(),
                                    current_dimension()) = 0;
    result->pattern.constant(current_dimension())
            = reduct_array->size.back() - 1;
    result->current_iteration = current_dimension();

    return result;
}

mapping translator::access(array_view *source, int padding)
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
    else if (auto strm = dynamic_cast<array_view*>(expr))
        return complete_access(strm, result_type);
    else
        return expr;
}

iterator_access * translator::iterate( range *r )
{
    iterator_access *it = new iterator_access(r->type);
    it->dimension = current_dimension();
    it->offset = r->start;
    if (r->end > r->start)
        it->ratio = 1;
    else if (r->end < r->start)
        it->ratio = -1;
    else
        it->ratio = 0;

    return it;
}

array_access * translator::complete_access
( array_view * view, const semantic::type_ptr & result_type)
{
    int padding = 0;
    if (result_type->is(semantic::type::stream))
    {
        padding = result_type->as<semantic::stream>().dimensionality() -
                (view->pattern.input_dimension() - view->current_iteration);
    }
    assert(padding >= 0);

    auto access = new array_access(view->target);
    access->pattern = this->access(view, padding);

    return access;
}

array_ptr translator::make_array(primitive_type type)
{
    ostringstream name;
    name << "A_" << m_arrays.size();

    auto a = make_shared<array>(name.str(), type);
    m_arrays.push_back(a);

    return a;
}

statement *
translator::make_statement()
{
    ostringstream name;
    name << "S_" << m_statements.size();

    auto stmt = new statement;
    m_statements.push_back(stmt);
    stmt->name = name.str();

    return stmt;
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
    stmt->array = make_array(expr->type);
    stmt->array->size = domain;
    stmt->write_relation = mapping::identity(domain.size());

    return stmt;
}

translator::array_view *
translator::make_current_view( array_ptr array )
{
    auto view = new array_view(array);
    view->pattern = mapping::identity(array->size.size());
    view->current_iteration = current_dimension();
    return view;
}

expression * translator::update_accesses(expression *expr, const mapping & map )
{
    if (auto operation = dynamic_cast<primitive_expr*>(expr))
    {
        for (auto & sub_expr : operation->operands)
        {
            sub_expr = update_accesses(sub_expr, map);
        }
        return expr;
    }
    if (auto dependency = dynamic_cast<array_access*>(expr))
    {
        // FIXME: duplicate, for the sake of consistency with stream_view below.

        //cout << "before: " << endl << dependency->pattern;
        dependency->pattern = dependency->pattern * map;
        //cout << "after: " << endl  << dependency->pattern;
        return expr;
    }
    if (auto view = dynamic_cast<array_view*>(expr))
    {
        if (debug_transform::is_enabled())
        {
            cout << "[poly] Applying map:" << endl << map;
            cout << "[poly] ..onto:" << endl << view->pattern;
        }

        assert(map.output_dimension() ==
               view->pattern.input_dimension() - view->current_iteration);

        int d = view->current_iteration;

        mapping m2 = mapping::identity(d + map.input_dimension(),
                                       d + map.output_dimension());

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
        if (debug_transform::is_enabled())
        {
            cout << "[poly] ..via:" << endl << m2;
        }

        auto new_pattern = view->pattern * m2;
        if (!new_pattern.input_dimension())
            new_pattern.resize(1, new_pattern.output_dimension());

        // FIXME: Only duplicate if shared (avoid memory leak).

        auto new_view = new array_view(view->target);
        new_view->pattern = new_pattern;
        new_view->current_iteration = view->current_iteration;

        if (debug_transform::is_enabled())
            cout << "[poly] ..result: " << endl  << new_view->pattern;

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
