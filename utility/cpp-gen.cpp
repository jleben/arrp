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

} // namespace cpp_gen
} // namespace stream
