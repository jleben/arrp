#include "cpp_from_polyhedral.hpp"
#include "collect_names.hpp"
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

static expression_ptr to_complex64(expression_ptr e, primitive_type t)
{
    if (t == primitive_type::complex64)
        return e;

    auto f = make_id("complex<double>");

    if (t == primitive_type::complex32)
        return call(f, {e});
    else
        return call(f, {e, literal((double)0)});
}
#if 0
static expression_ptr to_complex32(expression_ptr e, primitive_type t)
{
    if (t == primitive_type::complex32)
        return e;

    auto f = make_id("complex<float>");

    if (t == primitive_type::complex64)
        return call(f, {e});
    else
        return call(f, {e, literal((double)0)});
}
#endif

static expression_ptr to_real64(expression_ptr e, primitive_type t)
{
    if (t == primitive_type::real64)
        return e;
    else
        return cast(double_type(), e);
}

static expression_ptr to_real32(expression_ptr e, primitive_type t)
{
    if (t == primitive_type::real32)
        return e;
    else
        return cast(float_type(), e);
}

static expression_ptr to_type(expression_ptr e, primitive_type a, primitive_type r)
{
    if (a == r)
        return e;

    return cast(type_for(r), e);
}

void cpp_from_polyhedral::generate_statement
(const string & name, const index_type & index, builder* ctx)
{
    auto stmt_ref = std::find_if(m_model.statements.begin(), m_model.statements.end(),
                                 [&](polyhedral::stmt_ptr s){ return s->name == name; });

    if (stmt_ref == m_model.statements.end())
        return;

    generate_statement((*stmt_ref).get(), index, ctx);
}

void cpp_from_polyhedral::generate_statement
(polyhedral::statement *stmt, const index_type & index, builder* ctx)
{
    m_current_stmt = stmt;

    auto expr = generate_expression(stmt->expr, index, ctx);

    ctx->add(expr);
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
    else if (auto access = dynamic_cast<polyhedral::array_access*>(expr.get()))
    {
        index_type target_index;
        for (auto & e : access->indexes)
            target_index.push_back(generate_expression(e, index, ctx));

        result = generate_buffer_access(access->array, target_index, ctx);
    }
    else if ( auto const_int = dynamic_cast<functional::int_const*>(expr.get()) )
    {
        // FIXME:
        // Smallest signed 64 bit int X can not be literally represented.
        // It should be represented as 'Y - 1' where Y = X + 1.
        // Also: Print a warning or error if value is not representable at all?

        bool is_signed = is_signed_int(const_int->type->scalar()->primitive);

        if (is_signed and const_int->value() == mpz_class("-9223372036854775808"))
        {
            result = binop(op::sub, literal("-9223372036854775807"), literal(1));
        }
        else
        {
            string text = const_int->text();
            if (not is_signed)
                text += 'u';
            result = literal(text);
        }
    }
    else if ( auto const_double = dynamic_cast<functional::constant<double>*>(expr.get()) )
    {
        auto v = const_double->value;
        if (expr->type->scalar()->primitive == primitive_type::real32)
            result = literal((float)v);
        else
            result = literal(v);
    }
    else if ( auto const_bool = dynamic_cast<functional::bool_const*>(expr.get()) )
    {
        result = literal(const_bool->value);
    }
    else if ( auto const_complex = dynamic_cast<functional::complex_const*>(expr.get()) )
    {
        auto v = const_complex->value;
        if (expr->type->scalar()->primitive == primitive_type::complex32)
            result = literal(complex<float>((float)v.real(),(float)v.imag()));
        else
            result = literal(v);
    }
    else if (auto call = dynamic_cast<polyhedral::external_call*>(expr.get()))
    {
        vector<expression_ptr> args;
        for (auto & arg : call->args)
            args.push_back(generate_expression(arg, index, ctx));

        // FIXME: don't hardcode "io"
        auto callee = make_shared<bin_op_expression>
                (op::member_of_pointer, make_id("io"), make_id(call->name));
        return make_shared<call_expression>(callee, args);
    }
    else if (auto assign = dynamic_cast<polyhedral::assignment*>(expr.get()))
    {
        auto dest = generate_expression(assign->destination, index, ctx);
        auto value = generate_expression(assign->value, index, ctx);
        auto store = make_shared<bin_op_expression>(op::assign, dest, value);
        return store;
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
    case primitive_op::subtract:
    case primitive_op::multiply:
    case primitive_op::divide:
    {
        using t = primitive_type;

        auto & lhs = operands[0];
        auto & rhs = operands[1];
        auto lhs_t = expr->operands[0]->type->scalar()->primitive;
        auto rhs_t = expr->operands[1]->type->scalar()->primitive;
        auto r_t = expr->type->scalar()->primitive;

        if ( r_t == t::complex64 )
        {
            if (is_complex(lhs_t))
                lhs = to_complex64(lhs, lhs_t);
            else
                lhs = to_real64(lhs, lhs_t);

            if (is_complex(rhs_t))
                rhs = to_complex64(rhs, rhs_t);
            else
                rhs = to_real64(rhs, rhs_t);
        }
        else if ( r_t == t::complex32 )
        {
            if (!is_complex(lhs_t))
                lhs = to_real32(lhs, lhs_t);
            if (!is_complex(rhs_t))
                rhs = to_real32(rhs, rhs_t);
        }
        else if (expr->kind == primitive_op::divide)
        {
            if (is_integer(lhs_t) && is_integer(rhs_t))
            {
                lhs = cast(double_type(), lhs);
            }
        }

        switch(expr->kind)
        {
        case primitive_op::add:
            return make_shared<bin_op_expression>(op::add, operands[0], operands[1]);
        case primitive_op::subtract:
            return make_shared<bin_op_expression>(op::sub, operands[0], operands[1]);
        case primitive_op::multiply:
            return make_shared<bin_op_expression>(op::mult, operands[0], operands[1]);
        case primitive_op::divide:
            return make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
        default:
            throw error("Unexpected.");
        }
    }
    case primitive_op::bitwise_not:
        return make_shared<un_op_expression>(op::bit_neg, operands[0]);
    case primitive_op::bitwise_and:
        return make_shared<bin_op_expression>(op::bit_and, operands[0], operands[1]);
    case primitive_op::bitwise_or:
        return make_shared<bin_op_expression>(op::bit_or, operands[0], operands[1]);
    case primitive_op::bitwise_xor:
        return make_shared<bin_op_expression>(op::bit_xor, operands[0], operands[1]);
    case primitive_op::bitwise_lshift:
        return make_shared<bin_op_expression>(op::bit_left, operands[0], operands[1]);
    case primitive_op::bitwise_rshift:
        return make_shared<bin_op_expression>(op::bit_right, operands[0], operands[1]);
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
    case primitive_op::divide_integer:
    {
        auto result = make_shared<bin_op_expression>(op::div, operands[0], operands[1]);
        if ( is_integer(prim_type(expr->operands[0])) &&
             is_integer(prim_type(expr->operands[1])) )
        {
            return result;
        }
        else
        {
            return cast(int_type(), result);
        }
    }
    case primitive_op::modulo:
    {
        // FIXME: Implement true modulo?
        return binop(op::rem, operands[0], operands[1]);
    }
    case primitive_op::raise:
    {
        return make_shared<call_expression>("pow", operands[0], operands[1]);
    }
    case primitive_op::floor:
    {
        if (is_integer(prim_type(expr->operands[0])))
            return operands[0];
        return make_shared<call_expression>("floor", operands[0]);
    }
    case primitive_op::ceil:
    {
        if (is_integer(prim_type(expr->operands[0])))
            return operands[0];
        return make_shared<call_expression>("ceil", operands[0]);
    }
    case primitive_op::abs:
    {
        return make_shared<call_expression>("abs", operands[0]);
    }
    case primitive_op::max:
    {
        auto r_t = prim_type(expr);
        auto lhs = to_type(operands[0], prim_type(expr->operands[0]), r_t);
        auto rhs = to_type(operands[1], prim_type(expr->operands[1]), r_t);
        return make_shared<call_expression>("max", lhs, rhs);
    }
    case primitive_op::min:
    {
        auto r_t = prim_type(expr);
        auto lhs = to_type(operands[0], prim_type(expr->operands[0]), r_t);
        auto rhs = to_type(operands[1], prim_type(expr->operands[1]), r_t);
        return make_shared<call_expression>("min", lhs, rhs);
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
        auto arg_t = prim_type(expr->operands[0]);
        expression_ptr result = make_shared<call_expression>("exp2", operands[0]);
        if (is_integer(arg_t))
            result = cast(type_for(arg_t), result);
        return result;
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
    case primitive_op::real:
    {
        auto method_id = make_shared<id_expression>("real");
        auto method = binop(op::member_of_reference, operands[0], method_id);
        return call(method, {});
    }
    case primitive_op::imag:
    {
        auto method_id = make_shared<id_expression>("imag");
        auto method = binop(op::member_of_reference, operands[0], method_id);
        return call(method, {});
    }
    case primitive_op::to_int8:
    {
        return cast(btype("int8_t"), operands[0]);
    }
    case primitive_op::to_uint8:
    {
        return cast(btype("uint8_t"), operands[0]);
    }
    case primitive_op::to_int16:
    {
        return cast(btype("int16_t"), operands[0]);
    }
    case primitive_op::to_uint16:
    {
        return cast(btype("uint16_t"), operands[0]);
    }
    case primitive_op::to_int:
    case primitive_op::to_int32:
    {
        return cast(btype("int32_t"), operands[0]);
    }
    case primitive_op::to_uint32:
    {
        return cast(btype("uint32_t"), operands[0]);
    }
    case primitive_op::to_int64:
    {
        return cast(btype("int64_t"), operands[0]);
    }
    case primitive_op::to_uint64:
    {
        return cast(btype("uint64_t"), operands[0]);
    }
    case primitive_op::to_real32:
    {
        if (expr->operands[0]->type->scalar()->primitive != primitive_type::real32)
            return cast(float_type(), operands[0]);
        else
            return operands[0];
    }
    case primitive_op::to_real64:
    {
        if (expr->operands[0]->type->scalar()->primitive != primitive_type::real64)
            return cast(double_type(), operands[0]);
        else
            return operands[0];
    }
    case primitive_op::to_complex32:
    {
        auto t = expr->operands[0]->type->scalar()->primitive;
        if (t != primitive_type::complex32)
        {
            auto f = make_shared<id_expression>("complex<float>");
            if (t == primitive_type::complex64)
                return call(f, {operands[0]});
            else
                return call(f, {operands[0], literal((float)0)});
        }
        else
        {
            return operands[0];
        }
    }
    case primitive_op::to_complex64:
    {
        auto t = expr->operands[0]->type->scalar()->primitive;
        if (t != primitive_type::complex64)
        {
            auto f = make_shared<id_expression>("complex<double>");
            if (t == primitive_type::complex32)
                return call(f, {operands[0]});
            else
                return call(f, {operands[0], literal((double)0)});
        }
        else
        {
            return operands[0];
        }
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

    cpp_gen::buffer & buffer_info = m_buffers.at(array->name);

    expression_ptr buffer = make_shared<id_expression>(array_name);

    if (index.empty())
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

        auto stmt_offset = m_current_stmt->array_access_offset.find(array.get());
        if (stmt_offset != m_current_stmt->array_access_offset.end())
            offset += stmt_offset->second;

        if (offset != 0)
        {
            expression_ptr & i = buffer_index[0];
            i = make_shared<bin_op_expression>(op::add, i, literal(offset));
        }
    }

    index_type compressed_index;

    for (int dim = 0; dim < buffer_index.size(); ++dim)
    {
        int buffer_size = buffer_info.dimension_size[dim];

        if (buffer_size == 1)
            continue;

        expression_ptr i = buffer_index[dim];

        if (buffer_info.dimension_needs_wrapping[dim])
        {
            bool size_is_power_of_two =
                    buffer_size == (int)std::pow(2, (int)std::log2(buffer_size));

            // FIXME: use modulo instead of remainder
            if (size_is_power_of_two)
            {
                assert(buffer_size > 0);
                auto mask = literal(buffer_size-1);
                i = binop(op::bit_and, i, mask);
            }
            else
            {
                auto size = literal(buffer_size);
                i = make_shared<bin_op_expression>(op::rem, i, size);
            }
        }

        compressed_index.push_back(i);
    }

    if (compressed_index.empty())
        return buffer;

    // Loop-invariant code motion

    if (m_move_loop_invariant_code)
    {
        for (auto & dim_index : compressed_index)
        {
            auto lim = move_loop_invariant_code(dim_index, ctx);

            if (lim.highest_level < ctx->current_block_level() && lim.is_profitable)
            {
                dim_index = move_code(dim_index, lim.highest_level, ctx);
            }
        }
    }

    auto buffer_elem = make_shared<array_access_expression>(buffer, compressed_index);

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

cpp_from_polyhedral::loop_invariant_motion
cpp_from_polyhedral::move_loop_invariant_code(expression_ptr e, builder * ctx)
{
    // FIXME: We support only a limited set of expression types

    loop_invariant_motion lim;

    if (auto id = dynamic_pointer_cast<id_expression>(e))
    {
        lim.is_profitable = false;

        for (int level = 1; level < ctx->block_count(); ++level)
        {
            auto & block = ctx->block(level);
            if (!block.induction_var.empty() && block.induction_var == id->name)
            {
                lim.highest_level = level;
                break;
            }
        }
    }
    else if (auto unop = dynamic_pointer_cast<un_op_expression>(e))
    {
        lim.is_profitable = true;

        auto lim_rhs = move_loop_invariant_code(unop->rhs, ctx);

        lim.highest_level = lim_rhs.highest_level;
    }
    else if (auto binop = dynamic_pointer_cast<bin_op_expression>(e))
    {
        lim.is_profitable = true;

        auto lim_lhs = move_loop_invariant_code(binop->lhs, ctx);
        auto lim_rhs = move_loop_invariant_code(binop->rhs, ctx);

        lim.highest_level = std::max(lim_lhs.highest_level, lim_rhs.highest_level);

        if (lim.highest_level > lim_lhs.highest_level && lim_lhs.is_profitable)
            binop->lhs = move_code(binop->lhs, lim_lhs.highest_level, ctx);
        if (lim.highest_level > lim_rhs.highest_level && lim_rhs.is_profitable)
            binop->rhs = move_code(binop->rhs, lim_rhs.highest_level, ctx);
    }
    else if (auto call = dynamic_pointer_cast<call_expression>(e))
    {
        // FIXME: This assumes the call has no side effects (e.g. std::max)

        lim.is_profitable = true;

        vector<loop_invariant_motion> lim_args;

        for (auto & arg : call->args)
        {
            auto lim_arg = move_loop_invariant_code(arg, ctx);
            lim.highest_level = std::max(lim.highest_level, lim_arg.highest_level);
            lim_args.push_back(lim_arg);
        }

        for (int i = 0; i < call->args.size(); ++i)
        {
            auto & lim_arg = lim_args[i];
            auto & arg = call->args[i];
            if (lim.highest_level > lim_arg.highest_level && lim_arg.is_profitable)
                arg = move_code(arg, lim_arg.highest_level, ctx);
        }
    }

    return lim;
}

expression_ptr cpp_from_polyhedral::move_code(expression_ptr e, int block_level, builder * ctx)
{
    if (block_level < 0 || block_level >= ctx->block_count())
        return e;

    // FIXME: We assume the expression has int type,
    // since we only move array index expressions.

    auto tmp_name = ctx->new_var_id();
    // FIXME: Use "auto" type.
    auto tmp_decl = decl_expr(int_type(), tmp_name, e);

    ctx->block(block_level).stmts->push_back(make_shared<expr_statement>(tmp_decl));

    if (verbose<cpp_target>::enabled())
    {
        cout << "Moved code to level " << block_level << ":" << endl;
        cpp_gen::state s;
        tmp_decl->generate(s, cout);
        cout << endl;
    }

    return make_id(tmp_name);
}

}

}
