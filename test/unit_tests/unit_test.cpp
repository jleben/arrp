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

#include "unit_test.hpp"

#include "../../frontend/parser.h"
#include "../../frontend/environment.hpp"
#include "../../frontend/type_checker.hpp"
#include "../../polyhedral/translator.hpp"
#include "../../frontend/types.hpp"

#include <sstream>
#include <memory>

using namespace std;

namespace stream {
namespace unit_testing {

void test::run(istream &code, const string & symbol_name, const arg_list &args)
{
    m_symbol = symbol_name;
    m_args = args;
}

void test::run(istream &code)
{
    if (m_symbol.empty())
        throw failure("No target symbol specified.");

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

    auto sym_iter = env.find(m_symbol);
    if (sym_iter == env.end())
    {
        ostringstream text;
        text << "No symbol '" << m_symbol << "' available." << endl;
        throw failure(text.str());
    }

    semantic::symbol & sym = sym_iter->second;

    semantic::type_checker type_checker(env);
    semantic::type_ptr result_type =
            type_checker.check(sym, m_args);

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
    poly.translate(sym, m_args);

    test_polyhedral_model(poly.statements());
}

result test::try_run(istream & code)
{
    try {
        run(code);
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
