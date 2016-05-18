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

variable_decl_ptr buffer_decl(polyhedral::array_ptr array,
                              name_mapper & namer)
{
    auto elem_type = type_for(array->type);
    if (array->buffer_size.size() == 1 && array->buffer_size[0] == 1)
        return decl(elem_type, namer(array->name));
    else
        return make_shared<array_decl>(elem_type, namer(array->name), array->buffer_size);
}

class_node * state_type_def(const polyhedral::model & model,
                            unordered_map<string,buffer> & buffers,
                            name_mapper & namer)
{
    auto def = new class_node(class_class, "state");
    def->template_parameters.push_back("IO");
    def->alignment = 16; // for vectorization;

    def->sections.resize(2);
    def->sections[0].access = public_access;
    def->sections[1].access = private_access;

    auto io_type = make_shared<basic_type>("IO");
    {
        auto & sec = def->sections[0];

        sec.members.push_back(make_shared<data_field>(decl(pointer(io_type), "io")));

        auto init_func =
                make_shared<func_decl>(make_shared<func_signature>("initialize"));
        sec.members.push_back(init_func);
        auto proc_func =
                make_shared<func_decl>(make_shared<func_signature>("process"));
        sec.members.push_back(proc_func);
    }

    auto & sec = def->sections[1];

    for (auto array : model.arrays)
    {
        if (buffers[array->name].on_stack)
            continue;
        sec.members.push_back(make_shared<data_field>(buffer_decl(array,namer)));
    }

    for (auto array : model.arrays)
    {
        if (!buffers[array->name].has_phase)
            continue;
        auto int_t = make_shared<basic_type>("int");
        auto field = decl(int_t, namer(array->name + "_ph"));
        field->value = literal((int)0);
        sec.members.push_back(make_shared<data_field>(field));
    }

    return def;
}

unordered_map<string,buffer>
buffer_analysis(const polyhedral::model & model)
{
    using polyhedral::array;

    std::vector<array*> buffers_on_stack;
    std::vector<array*> buffers_in_memory;

    unordered_map<string,buffer> buffers;

    for (const auto & array : model.arrays)
    {
        buffer buf;
        buf.size = volume(array->buffer_size);

        if(array->is_infinite)
        {
            int flow_size = array->buffer_size[0];
            buf.has_phase =
                    //array->period_offset % flow_size  != 0 ||
                    array->period % flow_size  != 0;
        }
        else
        {
            buf.has_phase = false;
        }

        buffers[array->name] = buf;

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
    { return buffers[a->name].size < buffers[b->name].size; };

    std::sort(buffers_on_stack.begin(), buffers_on_stack.end(), buffer_size_is_smaller);

    int stack_size = 0;

    for(int idx = 0; idx < buffers_on_stack.size(); ++idx)
    {
        polyhedral::array *array = buffers_on_stack[idx];
        buffer & b = buffers[array->name];

        int elem_size = 0;
        switch(array->type)
        {
        case primitive_type::integer:
            elem_size = 4; break;
        case primitive_type::real:
            elem_size = 8; break;
        case primitive_type::boolean:
            elem_size = 4; break;
        default:
        {
            ostringstream msg;
            msg << "Unexpected type for array " << array->name
                << " = " << array->type;
            throw error(msg.str());
        }
        }

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
        buffer & b = buffers[array->name];
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
    for (const auto & array : model.arrays)
    {
        const buffer & buf = buffers[array->name];

        if (!buf.has_phase)
            continue;

        int offset = init ?
                    array->period_offset : array->period;
        int buffer_size = array->buffer_size[0];

        auto phase = make_shared<id_expression>(namer(array->name + "_ph"));

        auto next_phase = make_shared<bin_op_expression>(op::add, phase, literal(offset));
        next_phase = make_shared<bin_op_expression>(op::rem, next_phase, literal(buffer_size));

        auto phase_change = make_shared<bin_op_expression>(op::assign, phase, next_phase);

        ctx->add(phase_change);
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
              std::ostream & hdr_stream)
{
    unordered_map<string,buffer> buffers = buffer_analysis(model);

    cpp_gen::name_mapper name_mapper;
    module m;
    builder b(&m);
    //cpp_from_cloog cloog(&b);
    cpp_from_isl isl(&b);
    cpp_from_polyhedral poly(model, buffers, name_mapper);

    m.members.push_back(make_shared<include_dir>("cmath"));
    m.members.push_back(make_shared<include_dir>("algorithm"));
    m.members.push_back(make_shared<using_decl>("namespace std"));

    auto nmspc = make_shared<namespace_node>();
    nmspc->name = name;
    m.members.push_back(nmspc);

    //add_remainder_function(m,*nmspc);

    // FIXME: rather include header:
    nmspc->members.push_back(namespace_member_ptr(state_type_def(model,buffers,name_mapper)));

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
        auto sig = make_shared<func_signature>("state<IO>::initialize", explicit_inline);
        sig->template_parameters.push_back("IO");

        auto func = make_shared<func_def>(sig);

        if (ast.prelude)
        {
            b.set_current_function(sig.get());

            b.push(&func->body.statements);

            for (auto array : model.arrays)
            {
                if (buffers[array->name].on_stack)
                    b.add(make_shared<var_decl_expression>(buffer_decl(array,name_mapper)));
            }

            isl.generate(ast.prelude);

            //advance_buffers(model, buffers, &b, name_mapper, true);

            b.pop();
        }

        nmspc->members.push_back(func);
    }

    {
        auto sig = make_shared<func_signature>("state<IO>::process", explicit_inline);
        sig->template_parameters.push_back("IO");

        auto func = make_shared<func_def>(sig);

        if (ast.period)
        {
            b.set_current_function(sig.get());
            poly.set_in_period(true);

            b.push(&func->body.statements);

            for (auto array : model.arrays)
            {
                if (buffers[array->name].on_stack)
                    b.add(make_shared<var_decl_expression>(buffer_decl(array,name_mapper)));
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
            auto sig = signature_for("initialize");
            nmspc->members.push_back(make_shared<func_decl>(sig));
        }
        {
            auto sig = signature_for("process");
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
