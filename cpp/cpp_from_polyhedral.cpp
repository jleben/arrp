#include "cpp_from_polyhedral.hpp"
#include "../common/error.hpp"

using namespace std;

namespace stream {
namespace cpp_gen {

using index_type = cpp_from_polyhedral::index_type;

cpp_from_polyhedral::cpp_from_polyhedral
(const polyhedral::model & model,
 const unordered_map<string,buffer> & buffers,
 name_mapper & nm):
    m_model(model),
    m_buffers(buffers),
    m_name_mapper(nm)
{}

static primitive_type prim_type(const functional::expression * expr)
{
    auto scalar = dynamic_pointer_cast<functional::scalar_type>(expr->type);
    assert_or_throw(bool(scalar));
    return scalar->primitive;
}

static primitive_type prim_type(const functional::expr_ptr & expr)
{
    return prim_type(expr.get());
}

void cpp_from_polyhedral::generate_statement
(const string & name, const index_type & index, builder* ctx)
{
    auto stmt_ref = std::find_if(m_model.statements.begin(), m_model.statements.end(),
                                 [&](polyhedral::stmt_ptr s){ return s->name == name; });
    assert(stmt_ref != m_model.statements.end());

    generate_statement((*stmt_ref).get(), index, ctx);
}

void cpp_from_polyhedral::generate_statement
(polyhedral::statement *stmt, const index_type & index, builder* ctx)
{
    m_current_stmt = stmt;

    expression_ptr expr;

    {
        expr = generate_expression(stmt->expr, index, ctx);

        if (stmt->write_relation.array)
        {
            auto array_index = mapped_index(index, stmt->write_relation.matrix, ctx);
            auto dst = generate_buffer_access(stmt->write_relation.array, array_index, ctx);
            auto store = make_shared<bin_op_expression>(op::assign, dst, expr);
            ctx->add(store);
        }
        else
        {
            ctx->add(expr);
        }
    }
}

expression_ptr cpp_from_polyhedral::generate_expression
(functional::expr_ptr expr, const index_type & index, builder * ctx)
{
    using namespace polyhedral;

    expression_ptr result;

    if (auto operation = dynamic_cast<functional::primitive*>(expr.get()))
    {
        result = generate_primitive(operation, index, ctx);
    }
    else if (auto iterator = dynamic_cast<polyhedral::iterator_read*>(expr.get()))
    {
        assert(iterator->index >= 0 && iterator->index < index.size());
        return index[iterator->index];
    }
    else if (auto read = dynamic_cast<polyhedral::array_read*>(expr.get()))
    {
        // Assuming read size is [1];
        auto target_index = mapped_index(index, read->relation.matrix, ctx);
        result = generate_buffer_access(read->relation.array, target_index, ctx);
    }
    else if ( auto const_int = dynamic_cast<functional::constant<int>*>(expr.get()) )
    {
        result = literal(const_int->value);
    }
    else if ( auto const_double = dynamic_cast<functional::constant<double>*>(expr.get()) )
    {
        result = literal(const_double->value);
    }
    else if ( auto const_bool = dynamic_cast<functional::constant<bool>*>(expr.get()) )
    {
        result = literal(const_bool->value);
    }
    else if (auto call = dynamic_cast<polyhedral::external_call*>(expr.get()))
    {
        auto array_index = mapped_index(index, call->source.matrix, ctx);
        auto array_access = generate_buffer_access(call->source.array, array_index, ctx);
        auto array_address = make_shared<un_op_expression>(op::address, array_access);
        // FIXME: don't hardcode "io"
        auto callee = make_shared<bin_op_expression>
                (op::member_of_pointer, make_id("io"), make_id(call->name));
        return make_shared<call_expression>(callee, array_address);
    }
    else
    {
        throw error("Unexpected expression type.");
    }

    return result;
}

expression_ptr cpp_from_polyhedral::generate_primitive
(functional::primitive * expr, const index_type & index, builder * ctx)
{
    switch(expr->kind)
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
        auto type = type_for(prim_type(expr));
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
        auto false_assign = make_shared<bin_op_expression>(op::assign, id_expr, false_expr);
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

    switch(expr->kind)
    {
    case primitive_op::negate:
    {
        expression_ptr operand = operands[0];
        if (prim_type(expr) == primitive_type::boolean)
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
        if ( prim_type(expr->operands[0]) != primitive_type::real &&
             prim_type(expr->operands[1]) != primitive_type::real )
            operands[0] = make_shared<cast_expression>
                    (type_for(primitive_type::real), operands[0]);

        return make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
    }
    case primitive_op::divide_integer:
    {
        auto result = make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
        if ( prim_type(expr->operands[0]) == primitive_type::integer &&
             prim_type(expr->operands[1]) == primitive_type::integer )
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
        if (prim_type(expr->operands[0]) == primitive_type::integer)
            return operands[0];
        return make_shared<call_expression>("floor", operands[0]);
    }
    case primitive_op::ceil:
    {
        if (prim_type(expr->operands[0]) == primitive_type::integer)
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
    case primitive_op::tan:
    {
        return make_shared<call_expression>("tan", operands[0]);
    }
    case primitive_op::asin:
    {
        return make_shared<call_expression>("asin", operands[0]);
    }
    case primitive_op::acos:
    {
        return make_shared<call_expression>("acos", operands[0]);
    }
    case primitive_op::atan:
    {
        return make_shared<call_expression>("atan", operands[0]);
    }
    default:
        ostringstream text;
        text << "Unexpected primitive op: " << expr->kind;
        throw error(text.str());
    }
}

expression_ptr cpp_from_polyhedral::generate_buffer_access
(polyhedral::array_ptr array, const index_type & index, builder * ctx)
{
    index_type buffer_index = index;
    string array_name = m_name_mapper(array->name);

    cpp_gen::buffer & buffer_info = m_buffers[array->name];

    expression_ptr buffer = make_shared<id_expression>(array_name);

    if (array->buffer_size.size() == 1 && array->buffer_size[0] == 1)
        return buffer;

    // Add buffer phase

    if (m_in_period && buffer_info.has_phase)
    {
        assert(array->is_infinite);

        auto phase = make_shared<id_expression>(array_name + "_ph");

        expression_ptr & i = buffer_index[0];
        i = make_shared<bin_op_expression>(op::add, i, phase);
    }

    if (m_in_period)
    {
        int offset = 0;

        offset += array->period_offset;

        auto stmt_offset = m_current_stmt->array_access_offset.find(array.get());
        if (stmt_offset != m_current_stmt->array_access_offset.end())
            offset += stmt_offset->second;

        if (offset != 0)
        {
            expression_ptr & i = buffer_index[0];
            i = make_shared<bin_op_expression>(op::add, i, literal(offset));
        }
    }

    for (int dim = 0; dim < buffer_index.size(); ++dim)
    {
        // FIXME: is using stmt->buffer_period for domain size OK?
        bool dim_is_streaming = array->is_infinite && dim == 0;
        int buffer_size = array->buffer_size[dim];
        expression_ptr & i = buffer_index[dim];

        if (buffer_size == 1)
        {
            i = literal((int)0);
            continue;
        }

        bool may_wrap = false;
        if (dim_is_streaming)
        {
            may_wrap = m_current_stmt->streaming_needs_modulo;
        }
        else
        {
            int array_size = array->size[dim];
            may_wrap = buffer_size < array_size;
        }

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

expression_ptr
cpp_from_polyhedral::generate_buffer_phase
(const string & id, builder * ctx)
{
    auto info = m_model.phase_ids.find(id);
    if (info == m_model.phase_ids.end())
        return nullptr;

    auto array = info->second;
    auto phase = make_shared<id_expression>(m_name_mapper(array->name + "_ph"));
    return phase;
}

cpp_from_polyhedral::index_type
cpp_from_polyhedral::mapped_index
( const index_type & index,
  const polyhedral::affine_matrix & map,
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
