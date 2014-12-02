#include "unit_test.hpp"

#include "../../frontend/parser.h"
#include "../../frontend/environment.hpp"
#include "../../frontend/type_checker.hpp"
#include "../../frontend/ir-generator.hpp"
#include "../../polyhedral/translator.hpp"
#include "../../frontend/types.hpp"

#include <sstream>
#include <memory>

using namespace std;

namespace stream {
namespace unit_testing {

void test::run(istream &code, const string & symbol_name, const arg_list &args)
{
    // Parse

    Parser parser(code);
    if (parser.parse() != 0)
        throw failure("Syntactical error.");

    ast::node_ptr ast_root = parser.ast();
    assert(ast_root);

    semantic::environment env;
    semantic::environment_builder env_builder(env);
    if (!env_builder.process(ast_root))
        throw failure("Symbolic error.");

    auto sym_iter = env.find(symbol_name);
    if (sym_iter == env.end())
    {
        ostringstream text;
        text << "No symbol '" << symbol_name << "' available." << endl;
        throw failure(text.str());
    }

    semantic::symbol & sym = sym_iter->second;

    semantic::type_checker type_checker(env);
    semantic::type_ptr result_type =
            type_checker.check(sym, args);

    if (!result_type)
    {
        throw failure("Semantic error.");
    }

    test_type(result_type);

    if (result_type->is(semantic::type::function))
    {
        sym_iter = env.find(result_type->as<semantic::function>().name);
        assert(sym_iter != env.end());
        sym = sym_iter->second;
    }

    polyhedral::translator poly(env);
    poly.translate(sym, args);

    test_polyhedral_model(poly.statements());
}

result run( test & t, istream & code,
            const string & symbol, const test::arg_list & args )
{
    try {
        t.run(code, symbol, args);
    }
    catch (failure & e)
    {
        cerr << "FAILURE:" << endl;
        cerr << e.reason << endl;
        return unit_testing::failed;
    }

    cerr << "SUCCESS." << endl;
    return unit_testing::succeeded;
}

} // namespace unit_testing
} // namespace stream
