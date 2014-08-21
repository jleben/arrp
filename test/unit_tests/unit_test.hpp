#ifndef STREAM_LANG_UNIT_TEST_INCLUDED
#define STREAM_LANG_UNIT_TEST_INCLUDED

#include "../../frontend/parser.h"
#include "../../frontend/environment.hpp"
#include "../../frontend/type_checker.hpp"
#include "../../frontend/ir-generator.hpp"

namespace stream {
namespace unit_test {

ast::node_ptr syntactic_analysis(const string & code);

bool symbolic_analysis
(const string & code, semantic::environment * env = nullptr);

semantic::type_ptr semantic_analysis
(const string & code,
 const string & symbol,
 vector<semantic::type_ptr> arguments = vector<semantic::type_ptr>());

}
}

#endif // STREAM_LANG_UNIT_TEST_INCLUDED
