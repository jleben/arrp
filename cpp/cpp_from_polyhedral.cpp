#include "cpp_from_polyhedral.hpp"
#include "../common/error.hpp"

using namespace std;

namespace stream {
namespace cpp_gen {

using index_type = cpp_from_polyhedral::index_type;

cpp_from_polyhedral::cpp_from_polyhedral
(const polyhedral::model & model,
 const unordered_map<string,buffer> & buffers):
    m_model(model),
    m_buffers(buffers)
{}

void cpp_from_polyhedral::generate_statement
(const string & name, const index_type & index, builder* ctx)
{
    auto stmt_ref = std::find_if(m_model.statements.begin(), m_model.statements.end(),
                                 [&](polyhedral::statement_ptr s){ return s->name == name; });
    assert(stmt_ref != m_model.statements.end());

    generate_statement((*stmt_ref).get(), index, ctx);
}

void cpp_from_polyhedral::generate_statement
(polyhedral::statement *stmt, const index_type & index, builder* ctx)
{
    expression_ptr expr;

    if (dynamic_cast<polyhedral::input_access*>(stmt->expr.get()))
    {
        // FIXME: different iteration and data domains
        generate_input_access(stmt, index, ctx);
    }
    else if (!stmt->array)
    {
        generate_output_access(stmt, index, ctx);
    }
    else
    {
        expr = generate_expression(stmt->expr, index, ctx);

        auto array_index = mapped_index(index, stmt->write_relation, ctx);
        auto dst = generate_buffer_access(stmt->array, array_index, ctx);
        auto store = make_shared<bin_op_expression>(op::assign, dst, expr);

        ctx->add(make_shared<expr_statement>(store));
    }
}

expression_ptr cpp_from_polyhedral::generate_expression
(polyhedral::expression_ptr expr, const index_type & index, builder * ctx)
{
    using namespace polyhedral;

    expression_ptr result;

    if (auto operation = dynamic_cast<primitive_expr*>(expr.get()))
    {
        result = generate_primitive(operation, index, ctx);
    }
    else if (auto input = dynamic_cast<input_access*>(expr.get()))
    {
        int input_num = input->index;
        auto input_name = ctx->current_function()->parameters[input_num]->name;
        result = make_shared<id_expression>(input_name);
    }
    else if (auto iterator = dynamic_cast<iterator_access*>(expr.get()))
    {
        assert(iterator->dimension >= 0 && iterator->dimension < index.size());
        auto val = index[iterator->dimension];
        if (iterator->ratio != 1)
            val = make_shared<bin_op_expression>(op::mult, val, literal(iterator->ratio));
        if (iterator->offset)
            val = make_shared<bin_op_expression>(op::add, val, literal(iterator->offset));
        return val;
    }
    else if (auto read = dynamic_cast<array_access*>(expr.get()))
    {
        auto target_index = mapped_index(index, read->pattern, ctx);
        result = generate_buffer_access(read->target, target_index, ctx);
    }
    else if ( auto const_int = dynamic_cast<constant<int>*>(expr.get()) )
    {
        result = literal(const_int->value);
    }
    else if ( auto const_double = dynamic_cast<constant<double>*>(expr.get()) )
    {
        result = literal(const_double->value);
    }
    else if ( auto const_bool = dynamic_cast<constant<bool>*>(expr.get()) )
    {
        result = literal(const_bool->value);
    }
    else
    {
        throw std::runtime_error("Unexpected expression type.");
    }

    // FIXME: Do we need to convert anything?
    // result = convert(result, expr->type);

    return result;
}

expression_ptr cpp_from_polyhedral::generate_primitive
(polyhedral::primitive_expr * expr, const index_type & index, builder * ctx)
{
    switch(expr->op)
    {
    case primitive_op::logic_and:
    {
        auto lhs = generate_expression(expr->operands[0], index, ctx);
        auto rhs = generate_expression(expr->operands[1], index, ctx);
        return make_shared<bin_op_expression>(op::logic_and, lhs, rhs);
    }
    case primitive_op::logic_or:
    {
        auto lhs = generate_expression(expr->operands[0], index, ctx);
        auto rhs = generate_expression(expr->operands[1], index, ctx);
        return make_shared<bin_op_expression>(op::logic_or, lhs, rhs);
    }
    case primitive_op::conditional:
    {
        string id;
        auto type = type_for(expr->type);
        ctx->add(make_shared<expr_statement>(ctx->new_var(type, id)));
        auto id_expr = make_shared<id_expression>(id);

        auto condition_expr = generate_expression(expr->operands[0], index, ctx);

        auto true_block = new block_statement;
        auto false_block = new block_statement;

        ctx->push(&true_block->statements);
        auto true_expr = generate_expression(expr->operands[1], index, ctx);
        auto true_assign = make_shared<bin_op_expression>(op::assign, id_expr, true_expr);
        ctx->add(make_shared<expr_statement>(true_assign));

        ctx->pop();

        ctx->push(&false_block->statements);
        auto false_expr = generate_expression(expr->operands[2], index, ctx);
        auto false_assign = make_shared<bin_op_expression>(op::assign, id_expr, true_expr);
        ctx->add(make_shared<expr_statement>(false_assign));

        ctx->pop();

        auto if_stmt = make_shared<if_statement>
                (condition_expr, statement_ptr(true_block), statement_ptr(false_block));

        ctx->add(if_stmt);

        return id_expr;
    }
    default:
        break;
    }

    vector<expression_ptr> operands;
    operands.reserve(expr->operands.size());
    for (auto & operand_expr : expr->operands)
    {
        operands.push_back( generate_expression(operand_expr, index, ctx) );
    }

    switch(expr->op)
    {
    case primitive_op::negate:
    {
        expression_ptr operand = operands[0];
        if (expr->type == primitive_type::boolean)
            return make_shared<un_op_expression>(op::logic_neg, operand);
        else
            return make_shared<un_op_expression>(op::u_minus, operand);
    }
    case primitive_op::add:
        return make_shared<bin_op_expression>(op::add, operands[0], operands[1]);
    case primitive_op::subtract:
        return make_shared<bin_op_expression>(op::sub, operands[0], operands[1]);
    case primitive_op::multiply:
        return make_shared<bin_op_expression>(op::mult, operands[0], operands[1]);
    case primitive_op::compare_g:
        return make_shared<bin_op_expression>(op::greater, operands[0], operands[1]);
    case primitive_op::compare_geq:
        return make_shared<bin_op_expression>(op::greater_or_equal, operands[0], operands[1]);
    case primitive_op::compare_l:
        return make_shared<bin_op_expression>(op::lesser, operands[0], operands[1]);
    case primitive_op::compare_leq:
        return make_shared<bin_op_expression>(op::lesser_or_equal, operands[0], operands[1]);
    case primitive_op::compare_eq:
        return make_shared<bin_op_expression>(op::equal, operands[0], operands[1]);
    case primitive_op::compare_neq:
        return make_shared<bin_op_expression>(op::not_equal, operands[0], operands[1]);
    case primitive_op::divide:
    {
        if (expr->operands[0]->type != primitive_type::real)
            operands[0] = make_shared<cast_expression>(type_for(primitive_type::real), operands[0]);

        return make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
    }
    case primitive_op::divide_integer:
    {
        auto result = make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
        if ( expr->operands[0]->type == primitive_type::integer &&
             expr->operands[1]->type == primitive_type::integer )
        {
            return result;
        }
        else
        {
            return make_shared<cast_expression>(type_for(primitive_type::integer), result);
        }
    }
    case primitive_op::modulo:
    {
        return make_shared<call_expression>("remainder", operands[0], operands[1]);
    }
    case primitive_op::raise:
    {
        return make_shared<call_expression>("pow", operands[0], operands[1]);
    }
    case primitive_op::floor:
    {
        if (expr->operands[0]->type == primitive_type::integer)
            return operands[0];
        return make_shared<call_expression>("floor", operands[0]);
    }
    case primitive_op::ceil:
    {
        if (expr->operands[0]->type == primitive_type::integer)
            return operands[0];
        return make_shared<call_expression>("ceil", operands[0]);
    }
    case primitive_op::abs:
    {
        return make_shared<call_expression>("abs", operands[0]);
    }
    case primitive_op::max:
    {
        return make_shared<call_expression>("max", operands[0], operands[1]);
    }
    case primitive_op::min:
    {
        return make_shared<call_expression>("min", operands[0], operands[1]);
    }
    case primitive_op::log:
    {
        return make_shared<call_expression>("log", operands[0]);
    }
    case primitive_op::log2:
    {
        return make_shared<call_expression>("log2", operands[0]);
    }
    case primitive_op::log10:
    {
        return make_shared<call_expression>("log10", operands[0]);
    }
    case primitive_op::exp:
    {
        return make_shared<call_expression>("exp", operands[0]);
    }
    case primitive_op::exp2:
    {
        return make_shared<call_expression>("exp2", operands[0]);
    }
    case primitive_op::sqrt:
    {
        return make_shared<call_expression>("sqrt", operands[0]);
    }
    case primitive_op::sin:
    {
        return make_shared<call_expression>("sin", operands[0]);
    }
    case primitive_op::cos:
    {
        return make_shared<call_expression>("cos", operands[0]);
    }
    default:
        ostringstream text;
        text << "Unexpected primitive op: " << expr->op;
        throw error(text.str());
    }
}

void cpp_from_polyhedral::generate_input_access
(polyhedral::statement * stmt, const index_type & index, builder * ctx)
{
    int input_num = reinterpret_cast<polyhedral::input_access*>(stmt->expr.get())->index;

    if (stmt->flow_dim < 0)
    {
        auto input_name = ctx->current_function()->parameters[input_num]->name;
        auto input_id = make_shared<id_expression>(input_name);
        auto value = make_shared<array_access_expression>(input_id, index);

        auto array_index = mapped_index(index, stmt->write_relation, ctx);
        auto dst = generate_buffer_access(stmt->array, array_index, ctx);
        auto store = make_shared<bin_op_expression>(op::assign, dst, value);

        ctx->add(make_shared<expr_statement>(store));
    }
    else
    {
        index_type dst_index = index;
        int dim_dif = stmt->array->buffer_size.size() - dst_index.size();
        dst_index.resize(dst_index.size() + dim_dif, literal((int)0));

        expression_ptr dst = generate_buffer_access(stmt->array, dst_index, ctx);
        dst = make_shared<un_op_expression>(op::address, dst);
        auto call = make_shared<call_expression>("input", literal(input_num), dst);
        ctx->add(call);
        //cout << "Would generate input call for " << stmt->name << endl;
    }
}

void cpp_from_polyhedral::generate_output_access
(polyhedral::statement * stmt, const index_type & index, builder * ctx)
{
    auto access = dynamic_cast<polyhedral::array_access*>(stmt->expr.get());
    assert(access);

    index_type source_index = index;
    int dim_dif = access->target->buffer_size.size() - source_index.size();
    source_index.resize(source_index.size() + dim_dif, literal((int)0));

    expression_ptr src = generate_buffer_access(access->target, source_index, ctx);
    src = make_shared<un_op_expression>(op::address, src);
    auto call = make_shared<call_expression>("output", src);
    ctx->add(call);
}

expression_ptr cpp_from_polyhedral::generate_buffer_access
(polyhedral::array_ptr array, const index_type & index, builder * ctx)
{
    index_type buffer_index = index;

    cpp_gen::buffer & buffer_info = m_buffers[array->name];

    expression_ptr buffer = make_shared<id_expression>(array->name);
    auto state_arg_name = ctx->current_function()->parameters.back()->name;
    auto state_arg = make_shared<id_expression>(state_arg_name);

    if (!buffer_info.on_stack)
    {
        buffer = make_shared<bin_op_expression>(op::member_of_pointer, state_arg, buffer);
    }

    if (array->buffer_size.size() == 1 && array->buffer_size[0] == 1)
        return buffer;

    // Add buffer phase

    if (m_in_period && buffer_info.has_phase)
    {
        assert(array->flow_dim >= 0);

        auto phase_id = make_shared<id_expression>(array->name + "_ph");
        auto phase = make_shared<bin_op_expression>(op::member_of_pointer,
                                                    state_arg,
                                                    phase_id);

        expression_ptr & i = buffer_index[array->flow_dim];
        i = make_shared<bin_op_expression>(op::add, i, phase);
    }

    for (int dim = 0; dim < buffer_index.size(); ++dim)
    {
        // FIXME: is using stmt->buffer_period for domain size OK?
        int domain_size = dim == array->flow_dim ? array->period : array->size[dim];
        int buffer_size = array->buffer_size[dim];
        expression_ptr & i = buffer_index[dim];

        if (buffer_size == 1)
        {
            i = literal((int)0);
            continue;
        }

        bool may_wrap =
                (buffer_size < domain_size) ||
                (dim == array->flow_dim && buffer_info.has_phase);
        if (may_wrap)
        {
            // FIXME: use modulo instead of remainder
            auto size = literal(buffer_size);
            i = make_shared<bin_op_expression>(op::rem, i, size);
        }
    }

    auto buffer_elem = make_shared<array_access_expression>(buffer, buffer_index);

    return buffer_elem;
}

cpp_from_polyhedral::index_type
cpp_from_polyhedral::mapped_index
( const index_type & index,
  const polyhedral::mapping & map,
  builder * )
{
    assert(index.size() == map.input_dimension());

    index_type target_index;
    target_index.reserve(map.output_dimension());

    for(int out_dim = 0; out_dim < map.output_dimension(); ++out_dim)
    {
        expression_ptr val;
        for (int in_dim = 0; in_dim < map.input_dimension(); ++in_dim)
        {
            int coefficient = map.coefficient(in_dim, out_dim);
            if (coefficient == 0)
                continue;
            auto term = index[in_dim];
            if (coefficient != 1)
                term = make_shared<bin_op_expression>(op::mult, term, literal(coefficient));
            if (val)
                val = make_shared<bin_op_expression>(op::add, val, term);
            else
                val = term;
        }
        int c = map.constant(out_dim);
        if (val && c != 0)
            val = make_shared<bin_op_expression>(op::add, val, literal(c));
        if (!val)
            val = literal(c);

        target_index.push_back(val);
    }

    return target_index;
}


}
}
