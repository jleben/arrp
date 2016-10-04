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

#include "cpp-gen.hpp"

namespace stream {
namespace cpp_gen {

using namespace std;

void module::generate(cpp_gen::state &state, ostream &stream)
{
    for (auto member : members)
    {
        member->generate(state, stream);
        state.new_line(stream);
    }
}

void namespace_node::generate(cpp_gen::state &state, ostream &stream)
{
    state.new_line(stream);
    stream << "namespace ";
    if (!name.empty())
        stream << name << ' ';
    stream << "{";

    for (auto member : members)
    {
        state.new_line(stream);
        member->generate(state, stream);
    }

    state.new_line(stream);
    stream << "} // namespace";
    if (!name.empty())
        stream << ' ' << name;
    state.new_line(stream);
}

void extern_c_node::generate(cpp_gen::state &state, ostream &stream)
{
    state.new_line(stream);
    stream << "extern \"C\" {";

    for (auto member : members)
    {
        state.new_line(stream);
        member->generate(state, stream);
    }

    state.new_line(stream);
    stream << "} // extern \"C\"";
    state.new_line(stream);
}


void class_node::generate(cpp_gen::state & state, ostream & stream)
{
    state.new_line(stream);

    if (template_parameters.size())
    {
        stream << "template <";
        int i = 0;
        for(auto & param : template_parameters)
        {
            stream << "typename " << param;
            if (i > 0)
                stream << ", ";
        }
        stream << ">";
        state.new_line(stream);
    }

    switch(key)
    {
    case class_class:
        stream << "class";
        break;
    case struct_class:
        stream << "struct";
        break;
    }

    if (alignment != 0)
        stream << " alignas(" << alignment << ")";

    if (!name.empty())
        stream << ' ' << name;

    if (!sections.empty())
        state.new_line(stream);

    stream << "{";

    for(auto & section : sections)
    {
        section.generate(state, stream);
    }

    if (!sections.empty())
        state.new_line(stream);

    stream << "};";
}

void class_section::generate(cpp_gen::state & state, ostream & stream)
{
    switch(access)
    {
    case public_access:
        state.new_line(stream);
        stream << "public:";
        break;
    case private_access:
        state.new_line(stream);
        stream << "private:";
        break;
    default:
        break;
    }

    state.increase_indentation();

    for(auto member : members)
    {
        state.new_line(stream);
        member->generate(state, stream);
    }

    state.decrease_indentation();
}

void basic_type::generate(cpp_gen::state & state, ostream & stream)
{
    stream << name;
    if (is_const)
        stream << " const";
}

void pointer_type::generate(cpp_gen::state & state, ostream & stream)
{
    base->generate(state, stream);
    if (is_const)
        stream << " const";
    stream << " *";
}

void array_type::generate(cpp_gen::state & state, ostream & stream)
{
    base->generate(state, stream);
    stream << '[' << size << ']';
}

void reference_type_node::generate(cpp_gen::state & state, ostream & stream)
{
    base->generate(state, stream);
    stream << " &";
}

void variable_decl::generate(cpp_gen::state & state, ostream & stream)
{
    if (alignment)
        stream << "alignas(" << alignment << ") ";
    type->generate(state, stream);
    if (!name.empty())
        stream << ' ' << name;
    if (value)
    {
        stream << " = ";
        value->generate(state, stream);
    }
}

void array_decl::generate(cpp_gen::state & state, ostream & stream)
{
    variable_decl::generate(state, stream);
    for(auto dim : size)
    {
        stream << "[" << dim << "]";
    }
}

void custom_decl::generate(state &, ostream & stream)
{
    stream << text << ';';
}

void func_signature::generate(cpp_gen::state & state, ostream & stream)
{
    if (template_parameters.size())
    {
        stream << "template <";
        int i = 0;
        for(auto & param : template_parameters)
        {
            stream << "typename " << param;
            if (i > 0)
                stream << ", ";
        }
        stream << ">";
        state.new_line(stream);
    }

    if (inlining == explicit_inline)
        stream << "inline ";

    type->generate(state, stream);

    if (!name.empty())
        stream << ' ' << name;

    if (parameters.empty())
    {
        stream << "()";
    }
    else
    {
        stream << "( ";
        for (auto param : parameters)
        {
            param->generate(state, stream);
            if (param != parameters.back())
                stream << ", ";
        }
        stream << " )";
    }
}

void func_decl::generate(cpp_gen::state & state, ostream & stream)
{
    signature->generate(state, stream);
    stream << ';';
}

void id_expression::generate(cpp_gen::state & state, ostream & stream)
{
    stream << name;
}


static unordered_map<int, int> op_precedence_map()
{
    vector< vector<op> > rank =
    {
        { op::scope_resolution },
        { op::post_incr, op::post_decr,
          op::function_call,
          op::array_subscript,
          op::member_of_reference, op::member_of_pointer },
        { op::pre_incr, op::pre_decr,
          op::u_plus, op::u_minus,
          op::logic_neg, op::bit_neg,
          op::cast,
          op::dereference,
          op::address },
        { op::mult, op::div, op::rem },
        { op::add, op::sub },
        { op::bit_left, op::bit_right },
        { op::lesser, op::lesser_or_equal,
          op::greater, op::greater_or_equal },
        { op::equal, op::not_equal },
        { op::bit_and },
        { op::bit_xor },
        { op::bit_or },
        { op::logic_and },
        { op::logic_or },
        { op::assign,
          op::assign_add, op::assign_sub,
          op::assign_mult, op::assign_div, op::assign_rem,
          op::conditional },
    };

    unordered_map<int, int> m;
    for (unsigned int r = 0; r < rank.size(); ++r)
    {
        auto & ops = rank[r];
        for(auto & op : ops)
            m[static_cast<int>(op)] = r + 1;
    }

    return m;
}

static int precedence(cpp_gen::op op)
{
    static auto m = op_precedence_map();
    return m[static_cast<int>(op)];
}

static int precedence(expression_ptr expr)
{
    if (auto unop = dynamic_cast<un_op_expression*>(expr.get()))
    {
        return precedence(unop->op);
    }
    else if (auto binop = dynamic_cast<bin_op_expression*>(expr.get()))
    {
        return precedence(binop->op);
    }
    else if (dynamic_cast<array_access_expression*>(expr.get()))
    {
        return precedence(op::array_subscript);
    }
    else
    {
        return 0;
    }
}

static string op_text(cpp_gen::op op_type)
{
    switch(op_type)
    {
    case op::scope_resolution:
        return "::";
    case op::member_of_reference:
        return ".";
    case op::member_of_pointer:
        return "->";
    case op::dereference:
        return "*";
    case op::address:
        return "&";
    case op::logic_neg:
        return "!";
    case op::logic_and:
        return "&&";
    case op::logic_or:
        return "||";
    case op::bit_neg:
        return "~";
    case op::bit_and:
        return "&";
    case op::bit_or:
        return "|";
    case op::bit_xor:
        return "^";
    case op::bit_left:
        return "<<";
    case op::bit_right:
        return ">>";
    case op::lesser:
        return "<";
    case op::lesser_or_equal:
        return "<=";
    case op::greater:
        return ">";
    case op::greater_or_equal:
        return ">=";
    case op::equal:
        return "==";
    case op::not_equal:
        return "!=";
    case op::mult:
        return "*";
    case op::div:
        return "/";
    case op::rem:
        return "%";
    case op::add:
        return "+";
    case op::sub:
        return "-";
    case op::u_plus:
        return "+";
    case op::u_minus:
        return "-";
    case op::post_incr:
        return "++";
    case op::post_decr:
        return "--";
    case op::pre_incr:
        return "++";
    case op::pre_decr:
        return "--";
    case op::assign:
        return "=";
    case op::assign_add:
        return "+=";
    case op::assign_sub:
        return "-=";
    case op::assign_mult:
        return "*=";
    case op::assign_div:
        return "/=";
    case op::assign_rem:
        return "%=";
    default:
        return string();
    }
}

void un_op_expression::generate(cpp_gen::state & state, ostream & stream)
{
    bool wrap_rhs = false;
    if (dynamic_cast<bin_op_expression*>(rhs.get()))
        wrap_rhs = false;
    else if(precedence(op) < precedence(rhs))
        wrap_rhs = true;

    stream << op_text(op);

    if (wrap_rhs)
        stream << "(";
    rhs->generate(state, stream);
    if (wrap_rhs)
        stream << ")";
}


void bin_op_expression::generate(cpp_gen::state & state, ostream & stream)
{
    bool wrap_lhs = false;
    if (precedence(op) < precedence(lhs))
            wrap_lhs = true;

    bool wrap_rhs = false;
    if (dynamic_cast<un_op_expression*>(rhs.get()))
        wrap_rhs = false;
    else if (precedence(op) <= precedence(rhs))
        wrap_rhs = true;

    if (wrap_lhs)
        stream << "(";
    lhs->generate(state, stream);
    if (wrap_lhs)
        stream << ")";

    stream << op_text(op);

    if (wrap_rhs)
        stream << "(";
    rhs->generate(state, stream);
    if (wrap_rhs)
        stream << ")";
}

void if_expression::generate(cpp_gen::state & state, ostream & stream)
{
    condition->generate(state, stream);
    stream << " ? ";
    true_expr->generate(state, stream);
    stream << " : ";
    false_expr->generate(state, stream);
}

void call_expression::generate(cpp_gen::state & state, ostream & stream)
{
    callee->generate(state, stream);
    stream << "(";
    for (unsigned int a = 0; a < args.size(); ++a)
    {
        if (a > 0)
            stream << ", ";
        args[a]->generate(state, stream);
    }
    stream << ")";
}

void cast_expression::generate(cpp_gen::state & state, ostream & stream)
{
    type->generate(state, stream);
    stream << "(";
    expr->generate(state, stream);
    stream << ")";
}

void array_access_expression::generate(cpp_gen::state & state, ostream & stream)
{
    bool wrap = false;
    if (precedence(op::array_subscript) < precedence(id))
        wrap = true;

    if (wrap)
        stream << "(";
    id->generate(state, stream);
    if (wrap)
        stream << ")";

    for(auto i : index)
    {
        stream << "[";
        i->generate(state, stream);
        stream << "]";
    }
}

void statement::generate_nested(state & state, ostream & stream)
{
    state.increase_indentation();
    state.new_line(stream);
    generate(state, stream);
    state.decrease_indentation();
}

void block_statement::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "{";
    state.increase_indentation();
    for (auto stmt : statements)
    {
        state.new_line(stream);
        stmt->generate(state, stream);
    }
    state.decrease_indentation();
    state.new_line(stream);
    stream << "}";
}

void block_statement::generate_nested(state & state, ostream & stream)
{
    state.new_line(stream);
    generate(state, stream);
}

void expr_statement::generate(cpp_gen::state & state, ostream & stream)
{
    expr->generate(state, stream);
    stream << ";";
}

void if_statement::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "if (";
    condition->generate(state, stream);
    stream << ") ";

    true_part->generate_nested(state, stream);

    if (false_part)
    {
        state.new_line(stream);
        stream  << "else";

        false_part->generate_nested(state, stream);
    }

#if 0
    if (dynamic_pointer_cast<block_statement>(true_part))
    {
        state.new_line(stream);
        true_part->generate(state, stream);
    }
    else
    {
        bool brace = false;
        if ( dynamic_pointer_cast<if_statement>(true_part) ||
             dynamic_pointer_cast<for_statement>(true_part) )
            brace = true;

        if (brace)
        {
            state.new_line(stream);
            stream << '{';
        }
        state.increase_indentation();
        state.new_line(stream);
        true_part->generate(state, stream);
        state.decrease_indentation();
        if (brace)
        {
            state.new_line(stream);
            stream << '}';
        }
    }

    if (!false_part)
        return;

    state.new_line(stream);
    stream  << "else";

    if (dynamic_pointer_cast<block_statement>(false_part))
    {
        state.new_line(stream);
        false_part->generate(state, stream);
    }
    else
    {
        bool brace = false;
        if ( dynamic_pointer_cast<if_statement>(false_part) ||
             dynamic_pointer_cast<for_statement>(false_part) )
            brace = true;

        if (brace)
        {
            state.new_line(stream);
            stream << '{';
        }
        state.increase_indentation();
        state.new_line(stream);
        false_part->generate(state, stream);
        state.decrease_indentation();
        if (brace)
        {
            state.new_line(stream);
            stream << '}';
        }
    }
#endif
}

void for_statement::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "for (";
    initialization->generate(state, stream);
    stream << "; ";
    condition->generate(state, stream);
    stream << "; ";
    update->generate(state, stream);
    stream << ") ";

    body->generate_nested(state, stream);
#if 0
    if (!dynamic_pointer_cast<block_statement>(body))
    {
        state.increase_indentation();
        state.new_line(stream);
        body->generate(state, stream);
        state.decrease_indentation();
    }
    else
    {
        bool brace = false;
        if ( dynamic_pointer_cast<if_statement>(body) ||
             dynamic_pointer_cast<for_statement>(body) )
            brace = true;

        state.new_line(stream);
        body->generate(state, stream);
    }
#endif
}

void complex_statement::generate_nested(state & state, ostream & stream)
{
    state.new_line(stream);
    stream << "{";
    state.increase_indentation();
    state.new_line(stream);
    generate(state, stream);
    state.decrease_indentation();
    state.new_line(stream);
    stream << "}";
}

void return_statement::generate(cpp_gen::state &state, ostream & stream)
{
    stream << "return";
    if (value)
    {
        stream << ' ';
        value->generate(state, stream);
        stream << ";";
    }
}

void func_def::generate(cpp_gen::state & state, ostream & stream)
{
    if (is_inline)
        stream << "inline ";
    signature->generate(state, stream);
    state.new_line(stream);
    body.generate(state, stream);
}

} // namespace cpp_gen
} // namespace stream
