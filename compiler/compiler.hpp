#ifndef STREAM_LANG_COMPILER_INCLUDED
#define STREAM_LANG_COMPILER_INCLUDED

#include "options.hpp"
#include "../common/module.hpp"
#include "../common/ph_model.hpp"

#include <iostream>

namespace stream {
namespace compiler {

using std::istream;

namespace result {
enum code
{
    ok = 0,
    command_line_error,
    io_error,
    syntactic_error,
    symbolic_error,
    semantic_error,
    generator_error
};
}

result::code compile(const options &);

result::code compile_module(const module_source &, istream & text, const options &);

void print_buffer_sizes(const vector<stream::polyhedral::array_ptr> &);

}
}

#endif // STREAM_LANG_COMPILER_INCLUDED
