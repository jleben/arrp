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

#include "cpp_target.hpp"
//#include "cpp_from_cloog.hpp"
#include "cpp_from_polyhedral.hpp"
#include "cpp_from_isl.hpp"
#include "../utility/cpp-gen.hpp"

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;

namespace stream {
namespace cpp_gen {

static int volume( const vector<int> & extent )
{
    if (extent.empty())
        return 0;
    int v = 1;
    for(int e : extent)
        v *= e;
    return v;
}
#if 0
func_sig_ptr input_func_sig(inline_mode inlined)
{
    auto fp_t = make_shared<basic_type>("float");
    auto int_t = make_shared<basic_type>("int");
    vector<variable_decl_ptr> params =
    {
        decl(int_t,""),
        decl(pointer(fp_t),"")
    };

    return make_shared<func_signature>("input", params, inlined);
}

func_sig_ptr output_func_sig(inline_mode inlined)
{
    auto fp_t = make_shared<basic_type>("float");
    vector<variable_decl_ptr> params =
    {
        decl(pointer(fp_t),"")
    };

    return make_shared<func_signature>("output", params, inlined);
}

static base_type_ptr state_type()
{
    static base_type_ptr t(make_shared<basic_type>("state"));
    return t;
}

variable_decl_ptr variable_for(const semantic::type_ptr & t, const string & name)
{
    switch(t->get_tag())
    {
    case semantic::type::boolean:
        return make_shared<variable_decl>(make_shared<basic_type>("bool"), name);
    case semantic::type::integer_num:
        return make_shared<variable_decl>(make_shared<basic_type>("int"), name);
    case semantic::type::real_num:
        return make_shared<variable_decl>(make_shared<basic_type>("float"), name);
    case semantic::type::stream:
    {
        auto & stream = t->as<semantic::stream>();

        auto elem_type = type_for(stream.element_type);

        vector<int> size = stream.size;

        // FIXME: Omit infinite inputs from args?
        for(int & dim : size)
            if (dim == semantic::stream::infinite)
                dim = 0;

        return make_shared<array_decl>(elem_type, name, size);
    }
    default:
        throw error("Unexpected type.");
    }
}
#endif

#if 0
func_sig_ptr signature_for(const string & name)
{
    auto sig = new func_signature;
    sig->is_inline = true;
    sig->type = make_shared<basic_type>("void");
    sig->name = name;

#if 0
    for (unsigned int input_idx = 0; input_idx < args.size(); ++input_idx)
    {
        ostringstream name;
        name << "in";
        name << input_idx;

        auto param = variable_for(args[input_idx], name.str());

        sig->parameters.push_back(param);
    }
#endif

    auto state_param_t = make_shared<pointer_type>(state_type());
    auto state_param = make_shared<variable_decl>(state_param_t, "s");
    sig->parameters.push_back(state_param);

    return func_sig_ptr(sig);
}
#endif

variable_decl_ptr buffer_decl(const buffer & buf,
                              name_mapper & namer, int alignment = 0)
{
    assert(!buf.dimension_size.empty());

    auto elem_type = type_for(buf.type);

    vector<int> compressed_size;
    for (auto & s : buf.dimension_size)
        if (s != 1)
            compressed_size.push_back(s);

    if (compressed_size.empty())
    {
        return decl(elem_type, namer(buf.name));
    }
    else
    {
        auto decl = make_shared<array_decl>(elem_type, namer(buf.name), compressed_size);
        decl->alignment = alignment;
        return decl;
    }
}

shared_ptr<custom_decl> io_decl(const polyhedral::io_channel & io)
{
    auto decl = make_shared<custom_decl>();
    ostringstream text;
    text << "typedef";
    if (io.type->is_scalar())
    {
        auto t = type_for(io.type->scalar()->primitive);
        text << " " << t->name;
        text << " " << (io.name + "_type");
    }
    else if (io.type->is_array())
    {
        auto at = io.type->array();
        auto et = type_for(at->element);
        if (at->size[0] > 0)
        {
            text << " " << et->name;
            text << " " << (io.name + "_type");
            for (auto & s : at->size)
                text << '[' << s << ']';
        }
        else
        {
            text << " arrp::stream_type<";
            text << et->name;
            for (int i = 1; i < at->size.size(); ++i)
                text << '[' << at->size[i] << ']';
            text << '>';
            text << " " << (io.name + "_type");
        }
    }

    decl->text = text.str();

    return decl;
}

shared_ptr<custom_decl> io_period_count_decl(const polyhedral::io_channel & io)
{
    ostringstream text;
    text << "static constexpr int ";
    text << (io.name + "_period_size = ");

    int size;
    if (io.array->period == 0)
    {
        size = 0;
    }
    else
    {
        size = 1;
        assert(io.array->size[0] < 0);
        for(int d = 1; d < io.array->size.size(); ++d)
        {
            assert(io.array->size[d] > 0);
            size *= io.array->size[d];
        }
        size *= io.array->period;
    }

    text << size;

    auto decl = make_shared<custom_decl>();
    decl->text = text.str();
    return decl;
}

shared_ptr<custom_decl> io_latency_decl(const vector<polyhedral::io_channel> & channels)
{
    ostringstream text;
    text << "static unordered_map<string,int> latency() { return {";

    for (auto & channel : channels)
    {
        text << " { \"" << channel.name << "\", " << channel.latency << " },";
    }

    text << "}; }";

    auto decl = make_shared<custom_decl>();
    decl->text = text.str();
    return decl;
}

class_node * state_type_def(const polyhedral::model & model,
                            unordered_map<string,buffer> & buffers,
                            name_mapper & namer,
                            int data_alignment)
{
    auto def = new class_node(class_class, "program");
    def->template_parameters.push_back("IO");

    def->sections.resize(2);
    def->sections[0].access = public_access;
    def->sections[1].access = private_access;

    auto io_type = make_shared<basic_type>("IO");

    auto & public_sec = def->sections[0];

    {
        public_sec.members.push_back(make_shared<data_field>(decl(pointer(io_type), "io")));

        auto init_func =
                make_shared<func_decl>(make_shared<func_signature>("prelude"));
        public_sec.members.push_back(init_func);
        auto proc_func =
                make_shared<func_decl>(make_shared<func_signature>("period"));
        public_sec.members.push_back(proc_func);
    }

    auto & private_sec = def->sections[1];

    for (auto array : model.arrays)
    {
        const auto & buf = buffers.at(array->name);
        if (buf.on_stack)
            continue;
        auto field = make_shared<data_field>(buffer_decl(buf,namer,data_alignment));
        private_sec.members.push_back(field);
    }

    for (auto array : model.arrays)
    {
        if (!buffers.at(array->name).has_phase)
            continue;
        auto int_t = make_shared<basic_type>("int");
        auto field = decl(int_t, namer(array->name + "_ph"));
        field->value = literal((int)0);
        private_sec.members.push_back(make_shared<data_field>(field));
    }

    return def;
}

unordered_map<string,buffer>
buffer_analysis(const polyhedral::model & model, const compiler::options & opt)
{
    using polyhedral::array;

    std::vector<array*> buffers_on_stack;
    std::vector<array*> buffers_in_memory;

    unordered_map<string,buffer> buffers;

    for (const auto & array : model.arrays)
    {
        buffer buf;

        buf.name = array->name;

        buf.type = array->type;

        buf.period_offset = array->period;

        // Compute buffer size

        for(int dim = 0; dim < array->buffer_size.size(); ++dim)
        {
            int size = array->buffer_size[dim];

            if (size <= 1)
            {
                buf.dimension_size.push_back(size);
                buf.dimension_needs_wrapping.push_back(false);
                continue;
            }

            bool may_need_wrapping = false;

            if (dim == 0 && array->is_infinite)
            {
                if (opt.buffer_data_shifting)
                {
                    int period_size = array->last_period_access - array->first_period_access + 1;
                    int period_overlap_size = array->last_period_access
                            - (array->first_period_access + array->period)
                            + 1;

                    if (period_overlap_size > 0)
                    {
                        // Keep it at shifting at most 10% of accessed items per period,
                        // asymptotically.

                        int num_periods = std::ceil(period_overlap_size / (0.1 * period_size));
                        num_periods = max(1, num_periods);

                        buf.data_shift.period_count = num_periods;
                        buf.data_shift.size = period_overlap_size;
                        buf.data_shift.source = array->first_period_access + num_periods * array->period;

                        cout << "Data shift:"
                             << " size = " << buf.data_shift.size
                             << " source = " << buf.data_shift.source
                             << " #periods = " << buf.data_shift.period_count
                             << endl;

                        buf.has_phase = true;
                    }
                    else
                    {
                        buf.data_shift.period_count = 1;
                    }

                    size = array->last_period_access
                            + buf.data_shift.period_count * buf.period_offset
                            + 1;
                }
                else
                {
                    may_need_wrapping = true;

                    // Round to the next power of two, to avoid modulo when indexing
                    if (opt.data_size_power_of_two)
                        size = (int) pow(2, ceil(log2(size)));

                    buf.has_phase = array->period % size != 0;
                }
            }
            else
            {
                may_need_wrapping = array->size[dim] > size;

                // Round to the next power of two, to avoid modulo when indexing
                if (may_need_wrapping && opt.data_size_power_of_two)
                    size = (int) pow(2, ceil(log2(size)));
            }

            buf.dimension_size.push_back(size);
            buf.dimension_needs_wrapping.push_back(may_need_wrapping);
        }

        buf.size = volume(buf.dimension_size);

        buffers.emplace(array->name, buf);

        if (array->inter_period_dependency)
        {
            buffers_in_memory.push_back(array.get());
        }
        else
        {
            buffers_on_stack.push_back(array.get());
        }
    }

    auto buffer_size_is_smaller =
            [&](polyhedral::array * a, polyhedral::array * b) -> bool
    { return buffers.at(a->name).size < buffers.at(b->name).size; };

    std::sort(buffers_on_stack.begin(), buffers_on_stack.end(), buffer_size_is_smaller);

    int stack_size = 0;

    for(int idx = 0; idx < buffers_on_stack.size(); ++idx)
    {
        polyhedral::array *array = buffers_on_stack[idx];
        buffer & b = buffers.at(array->name);

        int elem_size = size_for(array->type);

        int mem_size = b.size * elem_size;
        // FIXME: use user option for max stack size
        if (stack_size + mem_size < 1024)
        {
            b.on_stack = true;
            stack_size += mem_size;
        }
        else
        {
            buffers_in_memory.push_back(array);
        }
    }

    for(int idx = 0; idx < buffers_in_memory.size(); ++idx)
    {
        polyhedral::array *array = buffers_in_memory[idx];
        buffer & b = buffers.at(array->name);
        b.on_stack = false;
    }

    return buffers;
}


static void advance_buffers(const polyhedral::model & model,
                            unordered_map<string,buffer> & buffers,
                            builder * ctx,
                            name_mapper & namer,
                            bool init)
{
    assert_or_throw(!init);

    for (const auto & array : model.arrays)
    {
        const buffer & buf = buffers.at(array->name);

        if (buf.has_phase)
        {
            cout << "Buffer " << buf.name << " has phase." << endl;

            int buffer_size = buf.dimension_size[0];

            auto phase = make_shared<id_expression>(namer(array->name + "_ph"));

            if (buf.data_shift.size)
            {
                cout << "Buffer " << buf.name << " shifts data." << endl;

                auto phase_change = binop(op::assign_add, phase, literal(buf.period_offset));
                ctx->add(phase_change);

                int max_phase = buf.data_shift.period_count * buf.period_offset;

                auto shift_block = make_shared<block_statement>();

                auto buf_ptr = make_id("d");

                {
                    auto buf_address = cast(pointer(type_for(buf.type)), make_id(buf.name));
                    auto buf_ptr_decl = decl_expr(auto_type(), *buf_ptr, buf_address);
                    shift_block->statements.push_back(stmt(buf_ptr_decl));
                }
                {
                    auto extent = buf.dimension_size;
                    extent[0] = 1;
                    int factor = volume(extent);

                    int source_address = buf.data_shift.source * factor;
                    int end_address = (buf.data_shift.source + buf.data_shift.size) * factor;
                    int dest_address = (buf.data_shift.source - buf.data_shift.period_count * buf.period_offset) * factor;

                    auto source = binop(op::add, buf_ptr, literal(source_address));
                    auto end = binop(op::add, buf_ptr, literal(end_address));
                    auto dest = binop(op::add, buf_ptr, literal(dest_address));

                    auto data_shift = call(make_id("std::copy"), { source, end, dest });
                    shift_block->statements.push_back(stmt(data_shift));
                }
                {
                    auto phase_reset = binop(op::assign, phase, literal(0));
                    shift_block->statements.push_back(stmt(phase_reset));
                }

                auto time_to_shift = binop(op::greater_or_equal, phase, literal(max_phase));
                auto conditional_shift = make_shared<if_statement>(time_to_shift, shift_block, nullptr);

                ctx->add(conditional_shift);
            }
            else
            {
                auto next_phase = binop(op::add, phase, literal(buf.period_offset));

                assert(buffer_size > 0);
                bool size_is_power_of_two =
                        buffer_size == (int)std::pow(2, (int)std::log2(buffer_size));

                if (size_is_power_of_two)
                {
                    auto mask = literal(buffer_size-1);
                    next_phase = binop(op::bit_and, next_phase, mask);
                }
                else
                {
                    next_phase = binop(op::rem, next_phase, literal(buffer_size));
                }

                auto phase_change = binop(op::assign, phase, next_phase);

                ctx->add(phase_change);
            }
        }
    }
}
#if 0
void add_remainder_function(cpp_gen::module &module, namespace_node & nmspc)
{
    auto int_type = make_shared<basic_type>("int");
    auto double_type = make_shared<basic_type>("double");

    builder build(&module);

    {
        auto x_arg = make_shared<variable_decl>(int_type, "x");
        auto y_arg = make_shared<variable_decl>(int_type, "y");
        auto f_sig = make_shared<func_signature>();
        f_sig->name = "remainder";
        f_sig->type = int_type;
        f_sig->parameters = {x_arg,y_arg};
        auto f = make_shared<func_def>(f_sig);
        f->is_inline = true;

        build.set_current_function(f.get());

        auto x = make_id("x");
        auto y = make_id("y");
        auto m = make_id("m");
        build.add(assign(decl_expr(int_type, *m), binop(op::rem, x, y)));

        auto zero = literal((int)0);
        auto m_not_zero = binop(op::not_equal, m, zero);
        auto m_neg = binop(op::lesser, m, zero);
        auto y_neg = binop(op::lesser, y, zero);
        auto sign_m_not_y = binop(op::not_equal, m_neg, y_neg);
        auto do_correct = binop(op::logic_and, m_not_zero, sign_m_not_y);
        auto m_corrected = binop(op::add, m, y);
        auto result = make_shared<if_expression>(do_correct, m_corrected, m);

        build.add(make_shared<return_statement>(result));

        nmspc.members.push_back(f);
    }

    {
        auto x_arg = make_shared<variable_decl>(double_type, "x");
        auto y_arg = make_shared<variable_decl>(double_type, "y");
        auto f_sig = make_shared<func_signature>();
        f_sig->name = "remainder";
        f_sig->type = double_type;
        f_sig->parameters = {x_arg,y_arg};
        auto f = make_shared<func_def>(f_sig);
        f->is_inline = true;

        auto x = make_shared<id_expression>("x");
        auto y = make_shared<id_expression>("y");
        expression_ptr q = make_shared<bin_op_expression>(op::div, x, y);
        q = make_shared<call_expression>("floor", q);
        auto b = make_shared<bin_op_expression>(op::mult, q, y);
        auto result = make_shared<bin_op_expression>(op::sub, x, b);

        f->body.statements.push_back(make_shared<return_statement>(result));

        nmspc.members.push_back(f);
    }
}
#endif
#if 0
func_sig_ptr output_getter_signature(const polyhedral::array_ptr & out_array)
{
    auto sig = make_shared<func_signature>();
    sig->is_inline = true;
    sig->name = "get_output";
    sig->type = make_shared<pointer_type>(type_for(out_array->type));
    sig->parameters.push_back( decl(pointer(state_type()), "s") );
    return sig;
}

void add_output_getter_func(cpp_gen::module &module, namespace_node & nmspc,
                            const polyhedral::array_ptr & out_array)
{
    builder ctx(&module);

    auto sig = output_getter_signature(out_array);
    auto func = make_shared<func_def>(sig);
    ctx.set_current_function(func.get());

    auto out_id = make_shared<id_expression>(out_array->name);

    auto state_arg_name = ctx.current_function()->parameters.back()->name;
    auto state_arg = make_shared<id_expression>(state_arg_name);
    expression_ptr out = make_shared<bin_op_expression>(op::member_of_pointer, state_arg, out_id);

    if (out_array->buffer_size.size() == 1 && out_array->buffer_size[0] == 1)
        out = make_shared<un_op_expression>(op::address, out);
    else
        out = make_shared<cast_expression>(sig->type, out);

    ctx.add(make_shared<return_statement>(out));

    nmspc.members.push_back(func);
}
#endif
void generate(const string & name,
              const polyhedral::model & model,
              const polyhedral::ast_isl & ast,
              std::ostream & src_stream,
              const compiler::options & opt)
{
    unordered_map<string,buffer> buffers = buffer_analysis(model, opt);

    cpp_gen::name_mapper name_mapper;
    module m;
    builder b(&m);
    //cpp_from_cloog cloog(&b);
    cpp_from_isl isl(&b);
    cpp_from_polyhedral poly(model, buffers, name_mapper);
    poly.set_move_loop_invariant_code(opt.loop_invariant_code_motion);

    m.members.push_back(make_shared<include_dir>("cmath"));
    m.members.push_back(make_shared<include_dir>("algorithm"));
    m.members.push_back(make_shared<include_dir>("complex"));
    m.members.push_back(make_shared<include_dir>("unordered_map"));
    m.members.push_back(make_shared<include_dir>("arrp.hpp"));

    m.members.push_back(make_shared<using_decl>("namespace std"));

    auto nmspc = make_shared<namespace_node>();
    nmspc->name = name;
    m.members.push_back(nmspc);

    //add_remainder_function(m,*nmspc);

    auto traits = make_shared<class_node>(struct_class, "traits");
    traits->sections.resize(1);

    traits->sections[0].members.push_back(io_latency_decl(model.inputs));

    for (auto & io : model.inputs)
    {
        traits->sections[0].members.push_back(io_decl(io));
        traits->sections[0].members.push_back(io_period_count_decl(io));
    }
    for (auto & io : model.outputs)
    {
        traits->sections[0].members.push_back(io_decl(io));
        traits->sections[0].members.push_back(io_period_count_decl(io));
    }

    nmspc->members.push_back(traits);

    // FIXME: rather include header:
    nmspc->members.push_back
            (namespace_member_ptr(state_type_def(model, buffers, name_mapper, opt.data_alignment)));

    // FIXME: not of much use with infinite I/O
    //add_output_getter_func(m, *nmspc, model.arrays.back());

    auto stmt_func = [&]
            ( const string & name,
            const vector<expression_ptr> & index,
            builder * ctx)
    {
        poly.generate_statement(name, index, ctx);
    };

    auto id_func = std::bind(&cpp_from_polyhedral::generate_buffer_phase,
                             &poly, placeholders::_1, &b);

    //cloog.set_stmt_func(stmt_func);
    //cloog.set_id_func(id_func);
    isl.set_stmt_func(stmt_func);
    isl.set_id_func(id_func);

    {
        auto sig = make_shared<func_signature>("program<IO>::prelude", explicit_inline);
        sig->template_parameters.push_back("IO");

        auto func = make_shared<func_def>(sig);

        if (ast.prelude)
        {
            b.set_current_function(sig.get());

            b.push(&func->body.statements);

            for (auto array : model.arrays)
            {
                const auto & buf = buffers.at(array->name);
                if (buf.on_stack)
                    b.add(make_shared<var_decl_expression>
                          (buffer_decl(buf,name_mapper,opt.data_alignment)));
            }

            isl.generate(ast.prelude);

            //advance_buffers(model, buffers, &b, name_mapper, true);

            b.pop();
        }

        nmspc->members.push_back(func);
    }

    {
        auto sig = make_shared<func_signature>("program<IO>::period", explicit_inline);
        sig->template_parameters.push_back("IO");

        auto func = make_shared<func_def>(sig);

        if (ast.period)
        {
            b.set_current_function(sig.get());
            poly.set_in_period(true);

            b.push(&func->body.statements);

            for (auto array : model.arrays)
            {
                const auto & buf = buffers.at(array->name);
                if (buf.on_stack)
                    b.add(make_shared<var_decl_expression>
                          (buffer_decl(buf,name_mapper,opt.data_alignment)));
            }

            isl.generate(ast.period);

            advance_buffers(model, buffers, &b, name_mapper, false);

            b.pop();
        }

        nmspc->members.push_back(func);
    }

    {
        cpp_gen::options opt;
        opt.indentation_size = 2;
        cpp_gen::state gen_state(opt);
        m.generate(gen_state, src_stream);
    }
#if 0
    {
        module header;
        auto nmspc = make_shared<namespace_node>();
        nmspc->name = name;
        header.members.push_back(nmspc);

        nmspc->members.push_back(namespace_member_ptr(state_type_def(model,buffers)));

        {
            auto sig = signature_for("prelude");
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            auto sig = signature_for("period");
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            auto sig = output_getter_signature(model.arrays.back());
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            // FIXME: Input type
            auto sig = input_func_sig();
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            // FIXME: Output type
            auto sig = output_func_sig();
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            cpp_gen::state gen;
            header.generate(gen, hdr_stream);
        }
    }
#endif
}

}
}
