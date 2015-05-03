#include "polyhedral-gen.hpp"
#include "../common/types.hpp"
#include "../common/error.hpp"
#include "../common/environment.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace polyhedral {

model_generator::model_generator() {}


model model_generator::generate
(const semantic::symbol & sym,
 const vector<semantic::type_ptr> & args)
{
    model m;

    m_model = &m;

    {
        assert(m_context.level() == 0);
        context::scope_holder root_scope(m_context);
        if (debug::is_enabled())
            cout << "[poly] Entering root scope." << endl;


        switch(sym.type)
        {
        case semantic::symbol::expression:
        {
            if (debug::is_enabled())
                cout << "[poly] Entering expression scope: " << sym.name << endl;

            context::scope_holder expr_scope(m_context);

            generate_array(sym.source_expression());

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
                m_context.bind(sym.parameter_names[i], generate_input(args[i], i) );
            }

            generate_array(sym.source_expression());

            if (debug::is_enabled())
                cout << "[poly] Exiting function scope: "  << sym.name << endl;

            break;
        }
        }

        if (debug::is_enabled())
            cout << "[poly] Exiting root scope." << endl;
    }

    m_model = nullptr;

    return m;
}

model_generator::array_view_ptr
model_generator::generate_input
(const semantic::type_ptr & type, int index)
{
    auto type_struct = semantic::structure(type);

    auto array = add_array(type_struct.type);
    array->size = type_struct.size;

    auto stmt = add_statement();
    stmt->expr = make_shared<input_access>(type_struct.type, index);
    stmt->array = array;
    stmt->domain = array->size;
    stmt->write_relation = mapping::identity(array->size.size());

    assert(m_domain.size() == 0);

    return make_shared<array_view>(array, mapping::identity(m_domain.size()));
}

model_generator::array_view_ptr
model_generator::generate_array(ast::node_ptr node)
{
    auto type_struct = semantic::structure(node->semantic_type);

    int dimensions = m_domain.size() + type_struct.size.size();
    m_transform.push(mapping::identity(dimensions));

    expression_ptr expr = generate_expression(node);

    m_transform.pop();

    auto a = add_array(expr->type);
    a->size = m_domain;
    a->size.insert(a->size.end(), type_struct.size.begin(), type_struct.size.end());

    auto s = add_statement();
    s->expr = expr;
    s->domain = a->size;
    s->array = a;
    s->write_relation = mapping::identity(a->size.size());

    auto view =
            make_shared<array_view>(a, mapping::identity(m_domain.size()));
    view->current_dim = m_domain.size();

    return view;
}

expression_ptr model_generator::generate_expression(ast::node_ptr node)
{
    // TODO: get constants from type

    switch(node->type)
    {
    case ast::expression_block:
        return generate_block(node);
    case ast::identifier:
        return generate_id(node);
    case ast::integer_num:
    {
        int value = node->as_leaf<int>()->value;
        return make_shared<constant<int>>(value);
    }
    case ast::real_num:
    {
        double value = node->as_leaf<double>()->value;
        return make_shared<constant<double>>(value);
    }
    case ast::boolean:
    {
        bool value = node->as_leaf<bool>()->value;
        return make_shared<constant<bool>>(value);
    }
    case ast::range:
    {
        return generate_range(node);
    }
    case ast::hash_expression:
    {
        assert(node->semantic_type->is(semantic::type::integer_num));
        const auto & integer = node->semantic_type->as<semantic::integer_num>();
        return make_shared<constant<int>>(integer.constant_value());
    }
    case ast::slice_expression:
        return generate_slice(node);
    case ast::transpose_expression:
        return generate_transpose(node);
    case ast::for_expression:
        return generate_mapping(node);
    case ast::negate:
    case ast::oppose:
    {
        return generate_unary_op(node);
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
        return generate_binary_op(node);
    }
    default:
        throw error("Unexpected AST node type.");
    }
}

expression_ptr model_generator::generate_block(ast::node_ptr node)
{
    const auto & stmt_list = node->as_list()->elements[0];
    const auto & expr = node->as_list()->elements[1];

    int base_ctx_level = m_context.level();

    if (stmt_list)
    {
        try
        {
            for ( const auto & stmt : node->as_list()->elements )
            {
                // TODO: do_statement(stmt);
                if (stmt->semantic_type->is(semantic::type::function))
                    m_context.enter_scope();
            }
        }
        catch(...)
        {
            m_context.roll_back_to(base_ctx_level);
            throw;
        }
    }

    auto result = generate_expression(expr);

    m_context.roll_back_to(base_ctx_level);

    return result;
}

expression_ptr model_generator::generate_id(ast::node_ptr node)
{
    string id = node->as_leaf<string>()->value;

    auto context_item = m_context.find(id);
    if (context_item)
    {
        const auto & view = context_item.value();

        mapping relation(transform().output_dimension(),
                         view->array->size.size());

        // Copy relation established so far

        relation.copy(view->relation);

        // Set remaining to identity

        int remaining_ins = transform().output_dimension() - view->relation.input_dimension();
        assert(view->current_dim + remaining_ins == relation.output_dimension());

        for(int i = 0; i < remaining_ins; ++i)
        {
            relation.coefficient(view->relation.input_dimension() + i,
                                 view->current_dim + i) = 1;
        }

        // Apply current transform

        relation = relation * transform();

        auto access = make_shared<array_access>(view->array, relation);
        return access;
    }
    else
    {
        throw error("Not implemented.");
    }
}

expression_ptr model_generator::generate_range(ast::node_ptr node)
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

    int start = start_int_t.constant_value();
    int end = end_int_t.constant_value();

    if (start == end)
        return make_shared<constant<int>>(start);

    auto it = make_shared<iterator_access>(primitive_type::integer);

    mapping relation(m_domain.size() + 1, 1);
    relation.constant(0) = start;
    relation.coefficient(m_domain.size(), 0) = end > start ? 1 : -1;

    it->relation = relation * transform();

    return it;
}

expression_ptr model_generator::generate_slice(ast::node_ptr node)
{
    using namespace semantic;

    const auto & object_node = node->as_list()->elements[0];
    const auto & selectors_node = node->as_list()->elements[1];

    const auto & object_type =
            object_node->semantic_type->as<semantic::stream>();

    mapping slicing(transform().output_dimension(),
                    m_domain.size() + object_type.dimensionality());

    int in_dim = 0;
    int out_dim = 0;

    for (;in_dim < m_domain.size(); ++in_dim, ++out_dim)
    {
        slicing.coefficient(in_dim, out_dim) = 1;
    }

    for( const auto & selector_node : selectors_node->as_list()->elements )
    {
        int offset;

        // TODO: detect affine expressions of loop indeces and parameters

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

            break;
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

    for (; in_dim < slicing.input_dimension(); ++in_dim, ++out_dim)
    {
        slicing.coefficients(out_dim, in_dim) = 1;
    }

    transform() = slicing * transform();

    return generate_expression(object_node);
}

expression_ptr model_generator::generate_transpose(ast::node_ptr node)
{
    const auto & object_node = node->as_list()->elements[0];
    const auto & dims_node = node->as_list()->elements[1];

    semantic::stream & object_type =
            object_node->semantic_type->as<semantic::stream>();

    int dimension = transform().output_dimension();
    assert(dimension == m_domain.size() + object_type.dimensionality());

    vector<int> order(dimension,-1);
    vector<bool> used_dims(dimension,false);

    int in_dim = 0;
    for(;in_dim < m_domain.size(); ++in_dim)
    {
        order[in_dim] = in_dim;
    }

    for ( const auto & dim_node : dims_node->as_list()->elements )
    {
        int out_dim = dim_node->as_leaf<int>()->value - 1;
        out_dim += m_domain.size();
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

    transform() = transposition * transform();

    return generate_expression(object_node);
}

expression_ptr model_generator::generate_mapping(ast::node_ptr node)
{
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    auto & result_type = node->semantic_type->as<semantic::stream>();

    vector<array_view_ptr> sources;

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();

        auto source = generate_array(iter.domain);

        assert(source->relation.input_dimension() == m_domain.size());

        // Expand

        int in_dim = m_domain.size();
        int out_dim = source->current_dim;

        {
            int in_dims = std::max(source->relation.input_dimension(), in_dim+1);
            int out_dims = std::max(source->relation.output_dimension(), out_dim+1);
            cout << "source relation: " << in_dims << " -> " << out_dims << endl;
            source->relation.resize(in_dims, out_dims);
            cout << "source relation true: "
                 << source->relation.input_dimension()
                 << " -> " << source->relation.output_dimension() << endl;
        }

        // Apply stride

        source->relation.coefficient(in_dim, out_dim) = iter.hop;

        // Update current dim

        assert(iter.size >= 1);
        if (iter.size == 1)
            ++source->current_dim;

        sources.push_back(source);
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

    expression_ptr result = generate_block(body_node);
    assert(result->type == result_type.element_type);

    m_domain.pop_back();

    return result;
}

expression_ptr model_generator::generate_unary_op(ast::node_ptr node)
{
    expression_ptr operand = generate_expression(node->as_list()->elements[0]);

    // create operation

    auto type = primitive_type_for(node->semantic_type);
    auto result = make_shared<primitive_expr>(type);

    result->operands.push_back(operand);

    switch(node->type)
    {
    case ast::negate:
    case ast::oppose:
        result->op = primitive_op::negate;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    return result;
}

expression_ptr model_generator::generate_binary_op(ast::node_ptr node)
{
    expression_ptr operand1 = generate_expression(node->as_list()->elements[0]);

    expression_ptr operand2 = generate_expression(node->as_list()->elements[1]);

    // create operation

    auto type = semantic::primitive_type_for(node->semantic_type);
    auto result = make_shared<primitive_expr>(type);

    result->operands.push_back(operand1);
    result->operands.push_back(operand2);

    switch(node->type)
    {
    case ast::add:
        result->op = primitive_op::add;
        break;
    case ast::subtract:
        result->op = primitive_op::subtract;
        break;
    case ast::multiply:
        result->op = primitive_op::multiply;
        break;
    case ast::divide:
        result->op = primitive_op::divide;
        break;
    case ast::divide_integer:
        result->op = primitive_op::divide_integer;
        break;
    case ast::modulo:
        result->op = primitive_op::modulo;
        break;
    case ast::raise:
        result->op = primitive_op::raise;
        break;
    case ast::lesser:
        result->op = primitive_op::compare_l;
        break;
    case ast::lesser_or_equal:
        result->op = primitive_op::compare_leq;
        break;
    case ast::greater:
        result->op = primitive_op::compare_g;
        break;
    case ast::greater_or_equal:
        result->op = primitive_op::compare_geq;
        break;
    case ast::equal:
        result->op = primitive_op::compare_eq;
        break;
    case ast::not_equal:
        result->op = primitive_op::compare_neq;
        break;
    case ast::logic_and:
        result->op = primitive_op::logic_and;
        break;
    case ast::logic_or:
        result->op = primitive_op::logic_or;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    return result;
}

array_ptr model_generator::add_array(primitive_type type)
{
    ostringstream array_name;
    array_name << "A" << m_model->arrays.size();
    auto a = make_shared<array>(array_name.str(), type);
    m_model->arrays.push_back(a);
    return a;
}

statement_ptr model_generator::add_statement()
{
    ostringstream stmt_name;
    stmt_name << "S" << m_model->statements.size();
    auto s = make_shared<statement>(stmt_name.str());
    m_model->statements.push_back(s);
    return s;
}

}
}
