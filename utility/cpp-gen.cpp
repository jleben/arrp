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

void program::generate(cpp_gen::state &state, ostream &stream)
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

    switch(key)
    {
    case class_class:
        stream << "class";
        break;
    case struct_class:
        stream << "struct";
        break;
    }

    if (!name.empty())
        stream << ' ' << name;
    state.new_line(stream);
    stream << "{";

    for(auto section : sections)
    {
        state.new_line(stream);
        section->generate(state, stream);
    }

    state.new_line(stream);
    stream << "};";

}

void class_section_node::generate(cpp_gen::state & state, ostream & stream)
{
    switch(access)
    {
    case public_access:
        stream << "public:";
        break;
    case private_access:
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

void basic_type_node::generate(cpp_gen::state & state, ostream & stream)
{
    stream << name;
    if (is_const)
        stream << " const";
}

void pointer_type_node::generate(cpp_gen::state & state, ostream & stream)
{
    base_type->generate(state, stream);
    if (is_const)
        stream << " const";
    stream << " *";
}

void reference_type_node::generate(cpp_gen::state & state, ostream & stream)
{
    base_type->generate(state, stream);
    stream << " &";
}

void variable_decl_node::generate(cpp_gen::state & state, ostream & stream)
{
    type->generate(state, stream);
    if (!name.empty())
        stream << ' ' << name;
}

void func_signature_node::generate(cpp_gen::state & state, ostream & stream)
{
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

void func_decl_node::generate(cpp_gen::state & state, ostream & stream)
{
    signature->generate(state, stream);
    stream << ';';
}

void id_expression::generate(cpp_gen::state & state, ostream & stream)
{
    stream << name;
}

void un_op_expression::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "(" << op;
    rhs->generate(state, stream);
    stream << ")";
}

void bin_op_expression::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "(";
    lhs->generate(state, stream);
    stream << ' ' << op << ' ';
    rhs->generate(state, stream);
    stream << ")";

}

void call_expression::generate(cpp_gen::state & state, ostream & stream)
{
    stream << func_name << "(";
    for (unsigned int a = 0; a < args.size(); ++a)
    {
        if (a > 0)
            stream << ", ";
        args[a]->generate(state, stream);
    }
    stream << ")";
}

void block_statement::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "{";
    state.increase_indentation();
    for (auto stmt : statements)
    {
        state.new_line(stream);
        stmt->generate(state, stream);
        stream << ";";
    }
    state.decrease_indentation();
    state.new_line(stream);
    stream << "}";
}

void expr_statement::generate(cpp_gen::state & state, ostream & stream)
{
    expr->generate(state, stream);
}

void if_statement::generate(cpp_gen::state & state, ostream & stream)
{
    stream << "if (";
    condition->generate(state, stream);
    stream << ") ";

    state.new_line(stream);

    body->generate(state, stream);
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

    state.new_line(stream);

    body->generate(state, stream);
}

void func_def_node::generate(cpp_gen::state & state, ostream & stream)
{
    signature->generate(state, stream);
    state.new_line(stream);
    body.generate(state, stream);
}

} // namespace cpp_gen
} // namespace stream
