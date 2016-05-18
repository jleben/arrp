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
#include "../utility/debug.hpp"

#include <isl/ast_build.h>
#include <isl/schedule_node.h>

#include <isl-cpp/context.hpp>
#include <isl-cpp/schedule.hpp>

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

isl_schedule_node * mark_loop_type_separate(const isl::context & ctx, isl_schedule_node * node)
{
    isl::printer p(ctx);
    auto type = isl_schedule_node_get_type(node);
    if (type == isl_schedule_node_band)
    {
        int dims = isl_schedule_node_band_n_member(node);
        for (int d = 0; d < dims; ++d)
        {
            node = isl_schedule_node_band_member_set_ast_loop_type
                    (node, d, isl_ast_loop_separate);
        }
    }

    int n_children = isl_schedule_node_n_children(node);
    for (int c = 0; c < n_children; ++c)
    {
        node = isl_schedule_node_child(node, c);
        node = mark_loop_type_separate(ctx, node);
        node = isl_schedule_node_parent(node);
    }

    return node;
}

void print_isl_ast_options(const isl::context & ctx_cpp)
{
    isl_ctx * ctx = ctx_cpp.get();

    cout << "-- ISL PH AST options: --" << endl;

    printf("atomic upper bound = %d\n",
           isl_options_get_ast_build_atomic_upper_bound(ctx));
    printf("prefer pdiv = %d\n",
           isl_options_get_ast_build_prefer_pdiv(ctx));
    printf("detect min max = %d\n",
           isl_options_get_ast_build_detect_min_max(ctx));
    printf("explot nested bound = %d\n",
           isl_options_get_ast_build_exploit_nested_bounds(ctx));
    int sep_bounds =
            isl_options_get_ast_build_separation_bounds(ctx);
    printf("separation bounds = %s\n",
           sep_bounds == ISL_AST_BUILD_SEPARATION_BOUNDS_IMPLICIT ?
               "implicit" : "explicit");
    printf("group coscheduled = %d\n",
           isl_options_get_ast_build_group_coscheduled(ctx));
    printf("scale strides = %d\n",
           isl_options_get_ast_build_scale_strides(ctx));
    printf("allow else = %d\n",
           isl_options_get_ast_build_allow_else(ctx));
    printf("allow or = %d\n", isl_options_get_ast_build_allow_or(ctx));
}

ast_isl make_isl_ast( schedule & sched, bool separate_loops )
{
    auto ctx = sched.full.ctx();

    if (verbose<ast_gen>::enabled())
        print_isl_ast_options(ctx);

    if (true)
    {
        //isl_options_set_ast_build_separation_bounds
                //(ctx.get(), ISL_AST_BUILD_SEPARATION_BOUNDS_IMPLICIT);
        //isl_options_set_ast_build_scale_strides(ctx, 0);
    }

    ast_isl output;

    auto build = isl_ast_build_from_context(sched.params.copy());

    if (separate_loops)
    {
        if (sched.prelude_tree.get())
        {
            auto root = isl_schedule_get_root(sched.prelude_tree.get());
            root = mark_loop_type_separate(ctx, root);

            sched.prelude_tree = isl_schedule_node_get_schedule(root);

            isl_schedule_node_free(root);
        }

        if (sched.period_tree.get())
        {
            auto root = isl_schedule_get_root(sched.period_tree.get());
            root = mark_loop_type_separate(ctx, root);

            sched.period_tree = isl_schedule_node_get_schedule(root);

            isl_schedule_node_free(root);
        }
    }

    if (sched.prelude_tree.get())
    {
        output.prelude =
                isl_ast_build_node_from_schedule(build, sched.prelude_tree.copy());
    }
    if (sched.period_tree.get())
    {
        output.period =
                isl_ast_build_node_from_schedule(build, sched.period_tree.copy());
    }

    isl_ast_build_free(build);

    return output;
}

}
}
