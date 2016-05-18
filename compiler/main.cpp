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

#include "arg_parser.hpp"
#include "compiler.hpp"
#include "../common/ast.hpp"
#include "../common/functional_model.hpp"
#include "../frontend/module_parser.hpp"
#include "../frontend/functional_gen.hpp"
#include "../frontend/func_reducer.hpp"
#include "../frontend/array_reduction.hpp"
#include "../frontend/array_transpose.hpp"
#include "../frontend/ph_model_gen.hpp"
#include "../polyhedral/modulo_avoidance.hpp"
#include "../polyhedral/scheduling.hpp"
#include "../polyhedral/isl_ast_gen.hpp"
#include "../polyhedral/storage_alloc.hpp"
#include "../cpp/cpp_target.hpp"

using namespace std;
using namespace stream;
using namespace stream::compiler;

namespace stream {
namespace compiler {

void print_help(const arguments & args)
{
    using namespace std;

    cout << "Usage:" << endl
         << "  streamc <input file> [<option>...]" << endl;

    cout << "Options:" << endl;

    cout << "  --generate or --gen or -g <symbol> [<type>...] :" << endl
         << "  \tGenerate output for top-level function or expression <symbol>" << endl
         << "  \twith given argument types." << endl
         << "  \tEach following argument <type> is used as the type of" << endl
         << "  \ta function parameter in generic function instantiation." << endl
         << "  \tA <type> can be one of the following:" << endl
         << "  \t- \"int\" - integer number," << endl
         << "  \t- \"real\" - real number," << endl
         << "  \t- a stream, e.g. \"[10,4,5]\""
         << " - each number represents size in one dimension." << endl
            ;

    cout << "  --output or -o <name>: Generate LLVM IR output file with <name>"
            " (default: 'out.ll')." << endl;

    cout << "  --cpp <name> : Generate C++ header file with <name>." << endl;

    cout << "  --meta or -m <name> : Generate JSON description file with <name>." << endl;

    auto verbose_topics = args.verbose_topics();
    cout << " --verbose or -v <topic> : Enable verbose output for <topic>." << endl
         << "  \tAvailable topics:" << endl;
    for (const auto & topic : verbose_topics)
        cout << "  \t- " << topic << endl;

    cout << "  --no-debug or -D <topic> : Disable debugging output for <topic>." << endl;
}

}
}

int main(int argc, char *argv[])
{
    arguments args(argc-1, argv+1);

    args.add_verbose_topic<module_parser>("parsing");
    args.add_verbose_topic<ast::output>("ast");
    args.add_verbose_topic<functional::model>("func-model");
    args.add_verbose_topic<functional::generator>("func-model-gen");
    args.add_verbose_topic<functional::type_checker>("type-check");
    args.add_verbose_topic<functional::func_reducer>("func-reduction");
    args.add_verbose_topic<functional::array_reducer>("array-reduction");
    args.add_verbose_topic<functional::array_transposer>("array-transpose");
    args.add_verbose_topic<functional::polyhedral_gen>("ph-model-gen");
    args.add_verbose_topic<polyhedral::model>("ph-model");
    args.add_verbose_topic<polyhedral::modulo_avoidance>("mod-avoid");
    args.add_verbose_topic<polyhedral::scheduler>("ph-scheduling");
    args.add_verbose_topic<polyhedral::ast_isl>("ph-ast");
    args.add_verbose_topic<polyhedral::ast_gen>("ph-ast-gen");
    args.add_verbose_topic<polyhedral::storage_allocator>("storage-alloc");
    args.add_verbose_topic<polyhedral::storage_output>("storage");
    args.add_verbose_topic<cpp_gen::renaming>("renaming");

    try {
        args.parse();
    }
    catch (arguments::abortion &)
    {
        return result::ok;
    }
    catch (arguments::error & e)
    {
        cerr << e.msg() << endl;
        return result::command_line_error;
    }

    return compile(args);
}
