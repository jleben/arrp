#ifndef STREAM_LANG_UNIT_TEST_INCLUDED
#define STREAM_LANG_UNIT_TEST_INCLUDED

#include "../../frontend/types.hpp"
#include "../../frontend/ast.hpp"
#include "../../frontend/environment.hpp"
#include "../../polyhedral/model.hpp"

#include <iostream>
#include <vector>

namespace stream {
namespace unit_test {

using std::vector;
using std::istream;

ast::node_ptr syntactic_analysis(istream & code);

bool symbolic_analysis
(istream & code, semantic::environment * env = nullptr);

semantic::type_ptr semantic_analysis
(istream & code,
 const string & symbol,
 vector<semantic::type_ptr> arguments = vector<semantic::type_ptr>());

vector<polyhedral::statement*> polyhedral_model
(istream & code,
 const string & symbol,
 vector<semantic::type_ptr> arguments = vector<semantic::type_ptr>());

enum result {
    success = 0,
    failure
};

inline result failure_msg(const string & msg)
{
    using namespace std;
    cerr << "FAILED: " << msg << endl;
    return failure;
}

inline result success_msg()
{
    using namespace std;
    cerr << "OK." << endl;
    return success;
}

}
}

#endif // STREAM_LANG_UNIT_TEST_INCLUDED
