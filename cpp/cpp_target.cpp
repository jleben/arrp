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
#include "cpp_from_cloog.hpp"
#include "cpp_from_polyhedral.hpp"
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

static base_type_ptr state_type()
{
    static base_type_ptr t(make_shared<basic_type>("state_t"));
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
        return make_shared<variable_decl>(make_shared<basic_type>("double"), name);
    case semantic::type::stream:
    {
        auto & stream = t->as<semantic::stream>();

        auto elem_type = type_for(stream.element_type);

        vector<int> size = stream.size;

        // FIXME:
        for(int & dim : size)
            if (dim == semantic::stream::infinite)
                dim = 0;

        return make_shared<array_decl>(elem_type, name, size);
    }
    default:
        throw error("Unexpected type.");
    }
}

func_sig_ptr signature_for(const string & name, const vector<semantic::type_ptr> & args)
{
    auto sig = new func_signature;
    sig->type = make_shared<basic_type>("void");
    sig->name = name;

    for (unsigned int input_idx = 0; input_idx < args.size(); ++input_idx)
    {
        ostringstream name;
        name << "in";
        name << input_idx;

        auto param = variable_for(args[input_idx], name.str());

        sig->parameters.push_back(param);
    }

    auto state_param_t = make_shared<pointer_type>(state_type());
    auto state_param = make_shared<variable_decl>(state_param_t, "state");
    sig->parameters.push_back(state_param);

    return func_sig_ptr(sig);
}

variable_decl_ptr buffer_decl(polyhedral::statement *stmt)
{
    auto elem_type = type_for(stmt->expr->type);
    if (stmt->buffer.size() == 1 && stmt->buffer[0] == 1)
        return decl(elem_type, stmt->name);
    else
        return make_shared<array_decl>(elem_type, stmt->name, stmt->buffer);
}

class_node * state_type_def(const vector<polyhedral::statement*> & stmts,
                            unordered_map<string,buffer> & buffers)
{
    auto def = new class_node(struct_class, "state_t");
    def->sections.resize(1);
    auto & sec = def->sections.back();

    for (auto stmt : stmts)
    {
        if (buffers[stmt->name].on_stack)
            continue;
        sec.members.push_back(make_shared<data_field>(buffer_decl(stmt)));
    }

    for (auto stmt : stmts)
    {
        if (!buffers[stmt->name].has_phase)
            continue;
        auto int_t = make_shared<basic_type>("int");
        auto decl = make_shared<variable_decl>(int_t, stmt->name + "_phase");
        sec.members.push_back(make_shared<data_field>(decl));
    }

    return def;
}

unordered_map<string,buffer>
buffer_analysis(const vector<polyhedral::statement*> & statements)
{

    std::vector<polyhedral::statement*> buffers_on_stack;
    std::vector<polyhedral::statement*> buffers_in_memory;

    unordered_map<string,buffer> buffers;

    for (polyhedral::statement *stmt : statements)
    {
        buffer buf;
        buf.size = volume(stmt->buffer);

        if(stmt->flow_dim >= 0)
        {
            int flow_size = stmt->buffer[stmt->flow_dim];
            buf.has_phase =
                    stmt->buffer_period_offset % flow_size  != 0 ||
                    stmt->buffer_period % flow_size  != 0;
        }
        else
        {
            buf.has_phase = false;
        }

        buffers[stmt->name] = buf;

        if (stmt->inter_period_dependency || stmt == statements.back())
            buffers_in_memory.push_back(stmt);
        else
            buffers_on_stack.push_back(stmt);
    }

    auto buffer_size_is_smaller =
            [&](polyhedral::statement * a, polyhedral::statement * b) -> bool
    { return buffers[a->name].size < buffers[b->name].size; };

    std::sort(buffers_on_stack.begin(), buffers_on_stack.end(), buffer_size_is_smaller);

    int stack_size = 0;

    for(int idx = 0; idx < buffers_on_stack.size(); ++idx)
    {
        polyhedral::statement *stmt = buffers_on_stack[idx];
        buffer & b = buffers[stmt->name];

        int elem_size = 0;
        switch(stmt->expr->type)
        {
        case primitive_type::integer:
            elem_size = 4;
        case primitive_type::real:
            elem_size = 8;
        case primitive_type::boolean:
            elem_size = 4;
        }

        int mem_size = b.size * elem_size;
        // FIXME: use user option for max stack size
        if (stack_size + mem_size < 4096)
        {
            b.on_stack = true;
            stack_size += mem_size;
        }
        else
        {
            buffers_in_memory.push_back(stmt);
        }
    }

    for(int idx = 0; idx < buffers_in_memory.size(); ++idx)
    {
        polyhedral::statement *stmt = buffers_in_memory[idx];
        buffer & b = buffers[stmt->name];
        b.on_stack = false;
    }

    return buffers;
}


static void advance_buffers(const vector<polyhedral::statement*> & statements,
                            unordered_map<string,buffer> & buffers,
                            builder * ctx, bool init)
{
    for (polyhedral::statement * stmt : statements)
    {
        const buffer & buf = buffers[stmt->name];

        if (!buf.has_phase)
            continue;

        int offset = init ?
                    stmt->buffer_period_offset : stmt->buffer_period;
        int buffer_size = stmt->buffer[stmt->flow_dim];

        auto phase_id = make_shared<id_expression>(stmt->name + "_phase");

        auto state_arg_name = ctx->current_function()->parameters.back()->name;
        auto state_arg = make_shared<id_expression>(state_arg_name);
        auto phase = make_shared<bin_op_expression>(op::member_of_pointer, state_arg, phase_id);

        auto next_phase = make_shared<bin_op_expression>(op::add, phase, literal(offset));
        next_phase = make_shared<bin_op_expression>(op::rem, next_phase, literal(buffer_size));

        auto phase_change = make_shared<bin_op_expression>(op::assign, phase, next_phase);

        ctx->add(phase_change);
    }
}

void add_remainder_function(cpp_gen::module &module)
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

        module.members.push_back(f);
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

        module.members.push_back(f);
    }
}

void generate(const string & name,
              const vector<semantic::type_ptr> & args,
              const vector<polyhedral::statement*> & poly_model,
              clast_stmt *finite_schedule,
              clast_stmt *periodic_schedule,
              std::ostream & stream)
{
    unordered_map<string,buffer> buffers = buffer_analysis(poly_model);

    module m;
    builder b(&m);
    cpp_from_cloog cloog(&b);
    cpp_from_polyhedral poly(poly_model, buffers);

    m.members.push_back(make_shared<include_dir>("cmath"));
    m.members.push_back(make_shared<using_decl>("namespace std"));

    add_remainder_function(m);

    m.members.push_back(module_member_ptr(state_type_def(poly_model,buffers)));

    auto stmt_func = [&]
            ( const string & name,
            const vector<expression_ptr> & index,
            builder * ctx)
    {
        poly.generate_statement(name, index, ctx);
    };

    cloog.set_stmt_func(stmt_func);

    if (finite_schedule)
    {
        auto sig = signature_for(name + "_finite", args);
        b.set_current_function(sig.get());

        auto func = make_shared<func_def>(sig);

        b.push(&func->body.statements);

        for (auto stmt : poly_model)
        {
            if (buffers[stmt->name].on_stack)
                b.add(make_shared<var_decl_expression>(buffer_decl(stmt)));
        }

        cloog.generate(finite_schedule);

        advance_buffers(poly_model, buffers, &b, true);

        b.pop();

        m.members.push_back(func);
    }

    if (periodic_schedule)
    {
        auto sig = signature_for(name + "_period", args);
        b.set_current_function(sig.get());
        poly.set_in_period(true);

        auto func = make_shared<func_def>(sig);

        b.push(&func->body.statements);

        for (auto stmt : poly_model)
        {
            if (buffers[stmt->name].on_stack)
                b.add(make_shared<var_decl_expression>(buffer_decl(stmt)));
        }

        cloog.generate(periodic_schedule);

        advance_buffers(poly_model, buffers, &b, false);

        b.pop();

        m.members.push_back(func);
    }

    cpp_gen::state gen_state;

    m.generate(gen_state, stream);
}

}
}
