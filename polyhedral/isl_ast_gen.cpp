/*
Compiler for language for stream processing

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

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

#include "isl_ast_gen.hpp"

#include <isl/ast_build.h>

#include <iostream>

using namespace std;

namespace stream {
namespace polyhedral {

isl_ast_node * after_for(isl_ast_node * node, isl_ast_build *build, void *user)
{
    isl::printer printer(isl::context(isl_ast_build_get_ctx(build)));

    isl::union_map for_sched(isl_ast_build_get_schedule(build));

    cout << "For loop: "; printer.print(for_sched); cout << endl;


    return node;
}

ast_isl make_isl_ast( const schedule & sched )
{
    isl::printer printer(sched.sched.ctx());

    ast_isl output;

    auto build = isl_ast_build_from_context(sched.params.copy());
    //build = isl_ast_build_set_after_each_for(build, &after_for, nullptr);

    if (!sched.prelude.is_empty())
        output.prelude = isl_ast_build_node_from_schedule_map(build, sched.prelude.copy());
    if (!sched.period.is_empty())
        output.period = isl_ast_build_node_from_schedule_map(build, sched.period.copy());

    isl_ast_build_free(build);

    return output;
}

}
}
