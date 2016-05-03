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

#include "ast_gen.hpp"

#include <cloog/isl/cloog.h>

using namespace std;

namespace stream {
namespace polyhedral {

clast_stmt *make_ast
(const isl::union_map & isl_schedule,
 const isl::set & isl_params,
 CloogState * state,
 CloogOptions * options)
{
    if (isl_schedule.is_empty())
        return nullptr;

    CloogUnionDomain *schedule =
            cloog_union_domain_from_isl_union_map(
                isl_schedule.copy());

    CloogDomain *context_domain = nullptr;


    context_domain = cloog_domain_from_isl_set(isl_params.copy());

    CloogInput *input =  cloog_input_alloc(context_domain, schedule);

    //cout << "--- Cloog input:" << endl;
    //cloog_input_dump_cloog(stdout, input, options);
    //cout << "--- End Cloog input ---" << endl;

    assert(input);

    clast_stmt *ast = cloog_clast_create_from_input(input, options);

    return ast;
}

ast make_ast( const schedule & sched )
{
    ast output;
    output.state = cloog_state_malloc();
    output.options = cloog_options_malloc(output.state);
    output.options->save_domains = true;
    output.prelude = make_ast(sched.prelude, sched.params, output.state, output.options);
    output.period = make_ast(sched.period, sched.params, output.state, output.options);
    return output;
}

}
}
