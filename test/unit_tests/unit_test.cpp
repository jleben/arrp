#include "unit_test.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace unit_test {

ast::node_ptr syntactic_analysis(const string & code)
{
    istringstream code_stream(code);
    Parser parser(code_stream);
    if (parser.parse() == 0)
        return parser.ast();
    else
        return ast::node_ptr();
}

bool symbolic_analysis
(const string & code, semantic::environment * env)
{
    ast::node_ptr ast_root = syntactic_analysis(code);
    if (!ast_root)
        return false;

    if(env)
    {
        semantic::environment_builder env_builder(*env);
        return env_builder.process(ast_root);
    }
    else
    {
        semantic::environment local_env;
        semantic::environment_builder env_builder(local_env);
        return env_builder.process(ast_root);
    }
}

semantic::type_ptr semantic_analysis
(const string & code,
 const string & symbol, vector<semantic::type_ptr> arguments)
{
    semantic::environment env;
    if (!symbolic_analysis(code, &env))
        return semantic::type_ptr();

    auto sym_iter = env.find(symbol);
    if (sym_iter == env.end())
    {
        cerr << "ERROR: no symbol '" << symbol << "' available." << endl;
        return semantic::type_ptr();
    }

    semantic::type_checker type_checker(env);
    semantic::type_ptr result_type =
            type_checker.check(sym_iter->second, arguments);

    return result_type;
}

}
}
