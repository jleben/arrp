#ifndef STREAM_LANG_COMPILER_INCLUDED
#define STREAM_LANG_COMPILER_INCLUDED

#include "arg_parser.hpp"
#include "../polyhedral/model.hpp"

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

result::code compile(const arguments &);

result::code compile_source(istream &, const arguments &);

result::code compile_polyhedral_model
(const vector<stream::polyhedral::statement*> &,
 const target_info &, const arguments &);

}
}

#endif // STREAM_LANG_COMPILER_INCLUDED
