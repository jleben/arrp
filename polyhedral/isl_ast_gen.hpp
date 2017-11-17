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

#ifndef STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED
#define STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED

#include "../common/ph_model.hpp"
#include "../utility/debug.hpp"

#include <isl/ast_build.h>
#include <isl/id.h>

namespace stream {
namespace polyhedral {

class ast_gen
{
public:
    struct options
    {
        bool separate_loops = false;
        bool parallel = false;
    };

    ast_gen(model &, schedule &, const options &);

    ast_isl generate();

private:
    isl::union_map compute_order();

    static isl_id * invoke_before_for
    (isl_ast_build *build, void *user)
    {
        return reinterpret_cast<ast_gen*>(user)->before_for(build);
    }

    static isl_ast_node * invoke_after_for
    (isl_ast_node *node, isl_ast_build *build, void *user)
    {
        return reinterpret_cast<ast_gen*>(user)->after_for(node, build);
    }

    isl_id * before_for(isl_ast_build *);
    isl_ast_node * after_for(isl_ast_node *node, isl_ast_build *);
    bool current_schedule_dimension_is_parallel(isl_ast_build *);
    void store_parallel_accesses_for_current_dimension(isl_ast_build *);

    model & m_model;
    model_summary m_model_summary;
    schedule & m_schedule;
    options m_options;
    isl::printer m_printer;
    isl::union_map m_order;

    bool m_allow_parallel_for = false;
    bool m_in_parallel_for = false;

    int m_deepest_loop = 0;
    int m_current_loop = 0;
};



}
}


#endif // STREAM_POLYHEDRAL_ISL_AST_GENERATION_INCLUDED

