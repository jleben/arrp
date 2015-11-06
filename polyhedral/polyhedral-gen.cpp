#include "polyhedral-gen.hpp"
#include "../common/types.hpp"
#include "../common/error.hpp"
#include "../common/environment.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace polyhedral {

model_generator::model_generator(const semantic::environment &env):
    m_env(env)
{}


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

            generate_array(sym.source_expression(), storage_required);

            if (debug::is_enabled())
                cout << "[poly] Exiting expression scope: "  << sym.name << endl;

            break;
        }
        case semantic::symbol::function:
        {
            if (debug::is_enabled())
                cout << "[poly] Entering function scope: " << sym.name << endl;

            context::scope_holder func_scope(m_context);

            cout << "...generating function inputs..." << endl;

            assert(sym.parameter_names.size() == args.size());
            for(int i = 0; i < args.size(); ++i)
            {
                m_context.bind(sym.parameter_names[i], generate_input(args[i], i) );
            }

            cout << "...generating function body..." << endl;

            generate_array(sym.source_expression(), storage_required);

            if (debug::is_enabled())
                cout << "[poly] Exiting function scope: "  << sym.name << endl;

            break;
        }
        }

        if (debug::is_enabled())
            cout << "[poly] Exiting root scope." << endl;
    }

    // FIXME: proceed...
    m_model = nullptr;
    return m;

    // FIXME: Should happen in frontend!
    // Check flow dimensions

    for(auto & stmt : m_model->statements)
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

    for(auto & a : m_model->arrays)
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

    assert(!m_model->statements.empty());
    auto & last_stmt = m_model->statements.back();
    if (last_stmt->flow_dim >= 0)
    {
        auto src = make_shared<array_access>(last_stmt->array);
        // NOTE: special pattern will be fixed later
        auto stmt = add_statement();
        stmt->domain = { infinite };
        stmt->flow_dim = 0;
        stmt->expr = src;
        // NOTE: no array
    }


    m_model = nullptr;

    return m;
}

expression_ptr
model_generator::generate_input
(const semantic::type_ptr & type, int index)
{
    auto type_struct = semantic::structure(type);

    expression_ptr result;

    if (type_struct.is_scalar())
    {
        result = make_shared<input_access>(type_struct.type, index);
    }
    else
    {
        array_var_vector vars(type_struct.size);
        auto expr = make_shared<input_access>(type_struct.type, index, vars);
        result = bind(vars, expr);
    }

    cout << "Input:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr
model_generator::generate_array(ast::node_ptr node, array_storage_mode mode)
{
    expression_ptr expr = generate_expression(node);

#if 0
    if (mode == storage_not_required)
    {
        // Is reduce redundant?
        expr = reduce(expr);

        expression_ptr raw_expr = expr;

        if (auto func = dynamic_cast<array_function*>(raw_expr.get()))
            raw_expr = func->expr;

        if (dynamic_cast<array_access*>(raw_expr.get()))
        {
            return expr;
        }
    }
#endif

    //array_var_vector vars(m_bound_array_vars.begin(), m_bound_array_vars.end());
    expr = bind(m_bound_array_vars, expr);

    //cout << "Array expr:" << endl;
    //m_printer.print(expr.get(), cout); cout << endl;

    expr = reduce(expr);

    array_var_vector vars;

    auto array = add_array(expr->type);
    if (auto func = dynamic_cast<array_function*>(expr.get()))
    {
        vars = func->vars;
        for (auto & var : func->vars)
            array->size.push_back(var->size);
    }
    else
    {
        // TODO: how about empty array->size?
        array->size = {1};
    }

    auto stmt = add_statement();
    stmt->expr = expr;
    stmt->array = array;
    stmt->write_index = vars; // identity;

    cout << "Statement:" << endl;
    m_printer.print(stmt, cout); cout << endl;

    // create array reader expression

    array_var_vector internal_vars( vars.begin() + m_bound_array_vars.size(),
                                    vars.end() );
    expression_ptr reader = make_shared<array_access>(array, vars);
    reader = bind(internal_vars, reader);

    cout << "Array reader:" << endl;
    m_printer.print(reader.get(), cout); cout << endl;

    return reader;
}

expression_ptr model_generator::generate_expression(ast::node_ptr node)
{
    // Use propagated constants:

    switch(node->semantic_type->get_tag())
    {
    case semantic::type::integer_num:
    {
        const semantic::integer_num &num = node->semantic_type->as<semantic::integer_num>();
        if (num.is_constant())
            return make_shared<constant<int>>(num.constant_value());
        break;
    }
    case semantic::type::real_num:
    {
        const semantic::real_num &num = node->semantic_type->as<semantic::real_num>();
        if (num.is_constant())
            return make_shared<constant<double>>(num.constant_value());
        break;
    }
    case semantic::type::boolean:
    {
        const semantic::boolean & b = node->semantic_type->as<semantic::boolean>();
        if (b.is_constant())
            return make_shared<constant<bool>>(b.constant_value());
        break;
    }
    default:;
    }

    switch(node->type)
    {
    case ast::expression_block:
        return generate_block(node);
    case ast::call_expression:
        return generate_call(node);
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
    case ast::if_expression:
        return generate_conditional(node);
    case ast::for_expression:
        return generate_mapping(node);
    case ast::reduce_expression:
        return generate_reduction(node);
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
            for ( const auto & stmt : stmt_list->as_list()->elements )
            {
                generate_definition(stmt);
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

expression_ptr
model_generator::generate_definition(ast::node_ptr node)
{
    ast::list_node *stmt = node->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & body_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    if (params_node)
    {
        // Function definition is obtained as semantic type at function call
        m_context.bind(id, nullptr);
        if (debug::is_enabled())
            cout << "[poly] Binding: " << id << endl;
        return nullptr;
    }


    auto result = generate_array(body_node);

    m_context.bind(id, result);

    return result;
}

expression_ptr model_generator::generate_call(ast::node_ptr node)
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

    const auto & id_node = node->as_list()->elements[0];
    const auto & args_node = node->as_list()->elements[1];

    // Get callee id
    assert(id_node->type == ast::identifier);
    const string & id = id_node->as_leaf<string>()->value;

    // Try primitive
    auto primitive_op_mapping = primitive_ops.find(id);
    if (primitive_op_mapping != primitive_ops.end())
    {
        auto expr = make_shared<primitive_expr>(primitive_type_for(node->semantic_type));
        expr->op = primitive_op_mapping->second;
        for (const auto & arg_node : args_node->as_list()->elements)
            expr->operands.push_back( generate_expression(arg_node) );

        cout << "Primitive call:" << endl;
        m_printer.print(expr.get(), cout); cout << endl;

        return expr;
    }

    // Process args
    std::vector<expression_ptr> args;
    for (const auto & arg_node : args_node->as_list()->elements)
    {
        args.push_back( generate_array(arg_node) );
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
    {
        m_context.bind(func->parameters[a], args[a]);
    }

    expression_ptr result = generate_expression(func->expression());

    if (debug::is_enabled())
        cout << "[poly] Exiting scope for call to: " << id << endl;

    cout << "Call:" << endl << id << " = ";
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_id(ast::node_ptr node)
{
    string id = node->as_leaf<string>()->value;

    auto context_item = m_context.find(id);

    expression_ptr expr;

    if (context_item)
    {
        expr = context_item.value();
    }
    else
    {
        // It must be in the environment

        if (debug::is_enabled())
            cout << "[poly] Entering root scope for id: " << id << endl;

        context::scope_holder root_scope(m_context, m_context.root_scope());

        // Clear domain
        vector<int> local_domain;
        std::swap(local_domain, m_domain);

        // Process statement
        const semantic::symbol & sym = m_env.at(id);
        assert(sym.source->type == ast::statement);

        expr = generate_definition(sym.source);

        // Restore domain
        std::swap(m_domain, local_domain);

        if (debug::is_enabled())
            cout << "[poly] Exiting root scope for id: " << id << endl;
    }

    cout << "Lookup:" << endl << id << " = ";
    m_printer.print(expr.get(), cout); cout << endl;

    return expr;
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

    int len = std::abs(end - start) + 1;

    auto it = make_shared<iterator_access>(primitive_type::integer);

    array_var_ptr var = make_shared<array_variable>(len);
    it->expr = var + start;

    auto result = bind({var}, it);

    cout << "Range:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_slice(ast::node_ptr node)
{
    using namespace semantic;

    const auto & object_node = node->as_list()->elements[0];
    const auto & selectors_node = node->as_list()->elements[1];

    const auto & object_type =
            object_node->semantic_type->as<semantic::stream>();

    struct slice
    {
        int offset;
        int size;
    };

    vector<slice> slices;


    for(int dim = 0; dim < selectors_node->as_list()->elements.size(); ++dim)
    {
        // TODO: detect affine expressions of loop indeces and parameters

        const auto & selector_node = selectors_node->as_list()->elements[dim];

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
                end = object_type.size[dim];
            }

            int offset = start - 1;
            int size = end - start + 1;
            assert(size > 0);

            slices.push_back(slice{offset, size});

            break;
        }
        default:
            assert(selector_node->semantic_type->is(semantic::type::integer_num));
            auto & offset_int_t = selector_node->semantic_type->as<semantic::integer_num>();
            assert(offset_int_t.is_constant());
            int offset = offset_int_t.constant_value() - 1;

            slices.push_back(slice{offset, 1});
        }
    }

    auto object = generate_expression(object_node);
    cout << "Slice object:" << endl;
    m_printer.print(object.get(), cout); cout << endl;

    array_var_vector new_vars;
    array_index_vector args;
    expression_ptr expr;

    for (int dim = 0; dim < (int)slices.size(); ++dim)
    {
        slice & s = slices[dim];
        if (s.size == 1)
        {
            args.push_back(s.offset);
        }
        else
        {
            auto new_var = make_shared<array_variable>(s.size);
            new_vars.push_back(new_var);
            args.push_back(new_var + s.offset);
        }
    }

    expr = bind(new_vars, apply(object, args));

    cout << "Slicing:" << endl;
    m_printer.print(expr.get(), cout); cout << endl;

    return expr;
}

expression_ptr model_generator::generate_transpose(ast::node_ptr node)
{
    const auto & object_node = node->as_list()->elements[0];
    const auto & dims_node = node->as_list()->elements[1];

    semantic::stream & object_type =
            object_node->semantic_type->as<semantic::stream>();

    vector<int> order;

    for ( const auto & dim_node : dims_node->as_list()->elements )
    {
        order.push_back(dim_node->as_leaf<int>()->value - 1);
    }

    array_var_vector vars(order.size());
    array_index_vector args(order.size());

    for (int in_dim = 0; in_dim < (int) order.size(); ++in_dim)
    {
        int out_dim = order[in_dim];
        int size = object_type.size[out_dim];
        vars[in_dim] = make_shared<array_variable>(size);
        args[out_dim] = vars[in_dim];
    }

    auto object_expr = apply(generate_expression(object_node), args);
    auto result = bind(vars, object_expr);

    cout << "Transpose:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_conditional(ast::node_ptr node)
{
    // TODO: convert to a primitive operation with 3 operands

    const auto & condition_node = node->as_list()->elements[0];
    const auto & true_node = node->as_list()->elements[1];
    const auto & false_node = node->as_list()->elements[2];

    assert(condition_node->semantic_type->is_scalar());

    // TODO: might wanna make a statement for the condition, to avoid
    // evaluating it for every item of true/false expression streams

    expression_ptr condition = generate_expression(condition_node);
    expression_ptr true_expr = generate_expression(true_node);
    expression_ptr false_expr = generate_expression(false_node);

    auto result = make_shared<primitive_expr>(primitive_type_for(node->semantic_type));
    result->operands = { condition, true_expr, false_expr };
    result->op = primitive_op::conditional;

    return result;
}

expression_ptr model_generator::generate_mapping(ast::node_ptr node)
{
    const auto & iterators_node = node->as_list()->elements[0];
    const auto & body_node = node->as_list()->elements[1];

    auto & result_type = node->semantic_type->as<semantic::stream>();

    vector<expression_ptr> windows;

    assert(result_type.size.size());

    int result_size = result_type.size[0];
    array_var_ptr mapping_index = make_shared<array_variable>(result_size);

    for (const auto & iterator_node : iterators_node->as_list()->elements)
    {
        semantic::iterator & iter =
                iterator_node->semantic_type->as<semantic::iterator>();

        cout << "...generating mapping source..." << endl;

        auto source = generate_array(iter.domain);

        array_index_vector args = { mapping_index * iter.hop };
        array_var_vector vars;

        if (iter.size > 1)
        {
            auto sub_win_index = make_shared<array_variable>(iter.size);
            args[0] = args[0] + sub_win_index;
            vars = { sub_win_index };
        }

        auto window = bind(vars, apply(source, args));
        windows.push_back(window);
    }

    cout << "...generating mapping body..." << endl;

    expression_ptr expr;

    {
        context::scope_holder mapping_scope(m_context);

        for(int i = 0; i < windows.size(); ++i)
        {
            semantic::iterator & iter =
                    iterators_node->as_list()->elements[i]
                    ->semantic_type->as<semantic::iterator>();
            m_context.bind(iter.id, windows[i]);
        }

        m_bound_array_vars.push_back(mapping_index);

        expr = generate_block(body_node);
        assert(expr->type == result_type.element_type);

        m_bound_array_vars.pop_back();
    }

    auto result = bind(mapping_index, expr);

    cout << "Mapping:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_reduction(ast::node_ptr node)
{
    const auto & accum_node = node->as_list()->elements[0];
    const auto & iter_node = node->as_list()->elements[1];
    const auto & source_node = node->as_list()->elements[2];
    const auto & body_node = node->as_list()->elements[3];
    string accum_id = accum_node->as_leaf<string>()->value;
    string iter_id = iter_node->as_leaf<string>()->value;

    auto source_type = semantic::structure(source_node->semantic_type);
    assert(source_type.size.size() == 1);
    assert(source_type.size.front() >= 2);

    int reduction_len = source_type.size[0];

    auto result_type = semantic::structure(node->semantic_type);

    auto reduction_array = add_array(result_type.type, m_bound_array_vars);
    reduction_array->size.push_back(reduction_len);

    auto source = generate_array(source_node);

    expression_ptr initializer = apply(source, array_index_expr(0));
    initializer = reduce(initializer);
    auto initializer_stmt = add_statement();
    initializer_stmt->expr = initializer;
    initializer_stmt->array = reduction_array;
    initializer_stmt->write_index = array_index_vector(m_bound_array_vars) << 0;


    auto reduction_index = make_shared<array_variable>(reduction_len - 1);

    auto accumulator =
            make_shared<array_access>(reduction_array,
                                      array_index_vector(m_bound_array_vars)
                                      << reduction_index);

    auto iterator = apply(source, reduction_index + 1);

    expression_ptr reduction_expr;

    {
        context::scope_holder reduction_scope(m_context);
        m_context.bind( accum_id, accumulator );
        m_context.bind( iter_id, iterator );

        m_bound_array_vars.push_back(reduction_index);

        reduction_expr = generate_block(body_node);

        m_bound_array_vars.pop_back();
    }

    reduction_expr = bind(array_var_vector(m_bound_array_vars) << reduction_index,
                          reduction_expr);
    // FIXME: reduction unnecessary?
    reduction_expr = reduce(reduction_expr);

    auto reduction_stmt = add_statement();
    reduction_stmt->expr = reduction_expr;
    reduction_stmt->array = reduction_array;
    reduction_stmt->write_index =
            array_index_vector(m_bound_array_vars) << (reduction_index + 1);

    cout << "Reduction initializer:" << endl;
    m_printer.print(initializer_stmt, cout); cout << endl;

    cout << "Reduction statement:" << endl;
    m_printer.print(reduction_stmt, cout); cout << endl;


    auto result =
            make_shared<array_access>(reduction_array,
                                      array_index_vector(m_bound_array_vars)
                                      << (reduction_len - 1));

    cout << "Reduction result:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_unary_op(ast::node_ptr node)
{
    // create operands

    auto operand_node = node->as_list()->elements[0];
    expression_ptr operand = generate_expression(operand_node);

    array_var_vector vars;

    auto struc = semantic::structure(node->semantic_type);
    if (!struc.is_scalar())
    {
        vars = array_var_vector(struc.size);
        assert (!operand_node->semantic_type->is_scalar());
        operand = apply(operand, vars);
    }

    // create operation

    auto type = primitive_type_for(node->semantic_type);
    auto result_expr = make_shared<primitive_expr>(type);

    result_expr->operands.push_back(operand);

    switch(node->type)
    {
    case ast::negate:
    case ast::oppose:
        result_expr->op = primitive_op::negate;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    // create result

    auto result = bind(vars, result_expr);

    cout << "Unary op:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::generate_binary_op(ast::node_ptr node)
{
    // create operands

    auto op1_node = node->as_list()->elements[0];
    auto op2_node = node->as_list()->elements[1];

    expression_ptr operand1 = generate_expression(op1_node);
    expression_ptr operand2 = generate_expression(op2_node);

    array_var_vector vars;

    auto struc = semantic::structure(node->semantic_type);
    if (!struc.is_scalar())
    {
        vars = array_var_vector(struc.size);
    }

    if (!op1_node->semantic_type->is_scalar())
    {
        operand1 = apply(operand1, vars);
    }
    if (!op2_node->semantic_type->is_scalar())
    {
        operand2 = apply(operand2, vars);
    }

    // create operation

    auto primitive_type = semantic::primitive_type_for(node->semantic_type);
    auto result_expr = make_shared<primitive_expr>(primitive_type);
    result_expr->operands.push_back(operand1);
    result_expr->operands.push_back(operand2);

    switch(node->type)
    {
    case ast::add:
        result_expr->op = primitive_op::add;
        break;
    case ast::subtract:
        result_expr->op = primitive_op::subtract;
        break;
    case ast::multiply:
        result_expr->op = primitive_op::multiply;
        break;
    case ast::divide:
        result_expr->op = primitive_op::divide;
        break;
    case ast::divide_integer:
        result_expr->op = primitive_op::divide_integer;
        break;
    case ast::modulo:
        result_expr->op = primitive_op::modulo;
        break;
    case ast::raise:
        result_expr->op = primitive_op::raise;
        break;
    case ast::lesser:
        result_expr->op = primitive_op::compare_l;
        break;
    case ast::lesser_or_equal:
        result_expr->op = primitive_op::compare_leq;
        break;
    case ast::greater:
        result_expr->op = primitive_op::compare_g;
        break;
    case ast::greater_or_equal:
        result_expr->op = primitive_op::compare_geq;
        break;
    case ast::equal:
        result_expr->op = primitive_op::compare_eq;
        break;
    case ast::not_equal:
        result_expr->op = primitive_op::compare_neq;
        break;
    case ast::logic_and:
        result_expr->op = primitive_op::logic_and;
        break;
    case ast::logic_or:
        result_expr->op = primitive_op::logic_or;
        break;
    default:
        throw runtime_error("Unexpected AST node type.");
    }

    // create result

    auto result = bind(vars, result_expr);

    cout << "Binary op:" << endl;
    m_printer.print(result.get(), cout); cout << endl;

    return result;
}

expression_ptr model_generator::reduce(expression_ptr expr)
{
    if (auto iter = dynamic_cast<iterator_access*>(expr.get()))
    {
        return make_shared<iterator_access>(reduce(iter->expr));
    }
    else if (auto array = dynamic_cast<array_access*>(expr.get()))
    {
        return make_shared<array_access>(array->target, reduce(array->index));
    }
    else if (auto input = dynamic_cast<input_access*>(expr.get()))
    {
        return make_shared<input_access>(input->type, input->index,
                                         reduce(input->array_index));
    }
    else if (auto op = dynamic_cast<primitive_expr*>(expr.get()))
    {
        vector<expression_ptr> reduced_operands;
        for (auto & operand : op->operands)
            reduced_operands.push_back(reduce(operand));
        return make_shared<primitive_expr>(op->type, op->op, reduced_operands);
    }
    // TODO: operators on array functions
    else
    {
        return expr;
    }
}

array_index_vector
model_generator::reduce(const array_index_vector & in)
{
    array_index_vector out;
    out.reserve(in.size());
    for(auto & expr : in)
        out.push_back(reduce(expr));
    return out;
}

array_index_expr
model_generator::reduce(const array_index_expr & e)
{
    array_index_expr e2;

    for (auto & term : e)
    {
        if (term.var)
        {
            if (auto bound = m_array_context.find(term.var))
            {
                e2 = e2 + bound.value() * term.scalar;
                continue;
            }
        }

        e2 = e2 + term;
    }

    return e2;
}

expression_ptr model_generator::apply(expression_ptr expr, const array_index_vector & args)
{
    if (args.empty())
        return expr;

    // assuming expression is reduced
    auto reduced_expr = expr;

    auto func = dynamic_pointer_cast<array_function>(reduced_expr);
    assert(func);

    // assuming args are reduced
    // auto args = reduce(app->args);

    const auto & vars = func->vars;
    assert(vars.size() >= args.size());

    array_context::scope_holder scope(m_array_context);

    for(int i = 0; i < (int) args.size(); ++i)
    {
        m_array_context.bind(vars[i], args[i]);
    }

    auto reduced_func_expr = reduce(func->expr);

    array_var_vector remaining_vars
            (vars.data() + args.size(),
             vars.data() + vars.size());

    return bind(remaining_vars, reduced_func_expr);
}

expression_ptr model_generator::bind(const array_var_vector & vars, expression_ptr expr)
{
    // assuming that expr is reduced

    auto reduced_expr = expr;

    if (vars.empty())
        return reduced_expr;

    if (auto nested_func = dynamic_cast<array_function*>(reduced_expr.get()))
    {
        array_var_vector combined_vars = vars;
        combined_vars.insert(combined_vars.end(),
                             nested_func->vars.begin(),
                             nested_func->vars.end());
        return make_shared<array_function>(combined_vars, nested_func->expr);
    }
    else
    {
        return make_shared<array_function>(vars, reduced_expr);
    }
}

array_ptr model_generator::add_array(primitive_type type)
{
    ostringstream array_name;
    array_name << "A" << m_model->arrays.size();
    auto a = make_shared<array>(array_name.str(), type);
    m_model->arrays.push_back(a);
    return a;
}

array_ptr model_generator::add_array(primitive_type type, const array_var_vector & vars)
{
    ostringstream array_name;
    array_name << "A" << m_model->arrays.size();
    auto a = make_shared<array>(array_name.str(), type);
    m_model->arrays.push_back(a);
    for (auto & var : vars)
        a->size.push_back(var->size);
    return a;
}

array_ptr model_generator::add_array(primitive_type type, const deque<array_var_ptr> & deq)
{
    array_var_vector vars(deq.begin(), deq.end());
    return add_array(type, vars);
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
