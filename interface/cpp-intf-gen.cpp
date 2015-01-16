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

#include "cpp-intf-gen.hpp"
#include "../utility/cpp-gen.hpp"

#include <stdexcept>

using namespace std;

namespace stream {
namespace cpp_interface {

using namespace stream::cpp_gen;

base_type_node *int_type() { return new basic_type_node("int"); }
pointer_type_node *int_ptr_type()
{
    return new pointer_type_node(int_type());
}

base_type_node *int32_type() { return new basic_type_node("int32_t"); }
base_type_node *int32_ptr_type() { return new pointer_type_node(int32_type()); }

base_type_node *int64_type() { return new basic_type_node("int64_t"); }
base_type_node *int64_ptr_type() { return new pointer_type_node(int64_type()); }

base_type_node *double_type() { return new basic_type_node("double"); }
pointer_type_node *double_ptr_type()
{
    return new pointer_type_node(double_type());
}

base_type_node *void_type() { return new basic_type_node("void"); }
pointer_type_node *void_ptr_type()
{
    return new pointer_type_node(void_type());
}

func_decl_node *decl_alloc_func()
{
    auto buf_ptr_t = new pointer_type_node(new basic_type_node("buffer"));

    func_signature_node *sig = new func_signature_node;
    sig->name = "allocate";
    sig->type = void_type();
    sig->parameters.push_back(new variable_decl_node(buf_ptr_t));

    return new func_decl_node(sig);
}

func_decl_node *decl_get_output_func(polyhedral::statement *output_stmt)
{
    auto buf_ptr_type = new pointer_type_node(new basic_type_node("buffer"));

    type_node *ret_type = nullptr;
    switch(output_stmt->expr->type)
    {
    case polyhedral::boolean:
        ret_type = int32_ptr_type(); break;
    case polyhedral::integer:
        ret_type = int32_ptr_type(); break;
    case polyhedral::real:
        ret_type = double_ptr_type(); break;
    }
    assert(ret_type);

    func_signature_node *sig = new func_signature_node;
    sig->name = "get_output";
    sig->type = ret_type;
    sig->parameters.push_back(new variable_decl_node(buf_ptr_type));

    return new func_decl_node(sig);
}

func_decl_node *decl_process_func(const string & name,
                                  const vector<semantic::type_ptr> & args)
{
    func_signature_node *func = new func_signature_node;
    func->name = name;
    func->type = void_type();
    for(const auto & arg : args)
    {
        variable_decl_node *param;
        switch(arg->get_tag())
        {
        case semantic::type::boolean:
            param = new variable_decl_node(int32_type());
            break;
        case semantic::type::integer_num:
            param = new variable_decl_node(int32_type());
            break;
        case semantic::type::real_num:
            param = new variable_decl_node(double_type());
            break;
        case semantic::type::stream:
            param = new variable_decl_node(double_ptr_type());
            break;
        default:
            throw std::runtime_error("Unexpected function argument type.");
        }
        func->parameters.push_back(param);
    }
    auto buffer_ptr_type =
            new pointer_type_node(new basic_type_node("buffer"));

    func->parameters.push_back(new variable_decl_node(buffer_ptr_type));

    return new func_decl_node(func);
}

cpp_gen::program * create
( const string & name,
  const vector<semantic::type_ptr> & args,
  const vector<polyhedral::statement*> & stmts,
  const dataflow::model &flow_model )
{
    program *prog = new program;

    namespace_node *namespc = new namespace_node;
    namespc->name = name;

    namespc->members.push_back(new using_decl("std::int32_t"));

    class_node *buffer_struct = new class_node(struct_class, "buffer");
    class_section_node *buffer_section = new class_section_node;
    buffer_section->members.push_back(
                new data_field_decl_node(int32_ptr_type(), "bool_buffer") );
    buffer_section->members.push_back(
                new data_field_decl_node(int32_ptr_type(), "int_buffer") );
    buffer_section->members.push_back(
                new data_field_decl_node(double_ptr_type(), "real_buffer") );
    buffer_section->members.push_back(
                new data_field_decl_node(int64_ptr_type(), "phase_buffer") );
    buffer_struct->sections.push_back(buffer_section);

    namespc->members.push_back(buffer_struct);

    extern_c_node *extern_c = new extern_c_node;
    extern_c->members.push_back(decl_alloc_func());
    extern_c->members.push_back(decl_process_func("initialize", args));
    extern_c->members.push_back(decl_process_func("process", args));
    extern_c->members.push_back(decl_get_output_func(stmts.back()));

    namespc->members.push_back(extern_c);

    prog->members.push_back(namespc);

    return prog;
}

}
}
