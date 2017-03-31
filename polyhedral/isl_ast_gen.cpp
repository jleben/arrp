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

ast_gen::ast_gen(model & m, schedule & s, const options & opt):
    m_model(m),
    m_model_summary(m),
    m_schedule(s),
    m_options(opt),
    m_order(compute_order())
{

}

isl::union_map ast_gen::compute_order()
{
    auto writes = m_model_summary.write_relations.in_domain(m_model_summary.domains);
    auto reads = m_model_summary.read_relations.in_domain(m_model_summary.domains);

    isl::union_map array_to_buffer(m_model.context);

    for (auto & array : m_model.arrays)
    {
        auto as = array->domain.get_space();
        auto m = isl::basic_map::universe(isl::space::from(as,as));
        int n_dim = array->domain.dimensions();
        auto s = m.get_space();
        for (int d = 0; d < n_dim; ++d)
        {
            int buf_size = array->buffer_size[d];
            m.add_constraint(s.out(d) == s.in(d) % buf_size);
        }

        array_to_buffer |= m;
    }

    writes.map_range_through(array_to_buffer);
    reads.map_range_through(array_to_buffer);

    isl::union_map order(m_model.context);

    order |= reads.inverse()(writes);
    order |= writes.inverse()(writes);
    order |= m_model_summary.order_relations;

    return order;
}

ast_isl ast_gen::generate()
{
    auto ctx = m_model.context;

    if (verbose<ast_gen>::enabled())
        print_isl_ast_options(ctx);

    if (true)
    {
        //isl_options_set_ast_build_separation_bounds
                //(ctx.get(), ISL_AST_BUILD_SEPARATION_BOUNDS_IMPLICIT);
        //isl_options_set_ast_build_scale_strides(ctx, 0);
    }

    ast_isl output;

    auto build = isl_ast_build_from_context(m_schedule.params.copy());

    if (m_options.separate_loops)
    {
        if (m_schedule.tree.get())
        {
            auto root = isl_schedule_get_root(m_schedule.prelude_tree.get());
            root = mark_loop_type_separate(ctx, root);
            m_schedule.tree = isl_schedule_node_get_schedule(root);
            isl_schedule_node_free(root);
        }

        if (m_schedule.prelude_tree.get())
        {
            auto root = isl_schedule_get_root(m_schedule.prelude_tree.get());
            root = mark_loop_type_separate(ctx, root);

            m_schedule.prelude_tree = isl_schedule_node_get_schedule(root);

            isl_schedule_node_free(root);
        }

        if (m_schedule.period_tree.get())
        {
            auto root = isl_schedule_get_root(m_schedule.period_tree.get());
            root = mark_loop_type_separate(ctx, root);

            m_schedule.period_tree = isl_schedule_node_get_schedule(root);

            isl_schedule_node_free(root);
        }
    }

    build = isl_ast_build_set_before_each_for(build, &ast_gen::invoke_before_for, this);
    build = isl_ast_build_set_after_each_for(build, &ast_gen::invoke_after_for, this);

    if (m_schedule.tree.get())
    {
        if (verbose<ast_gen>::enabled())
            cout << endl << "** Building AST for entire program." << endl;
        m_in_parallel_for = false;
        output.full =
                isl_ast_build_node_from_schedule(build, m_schedule.tree.copy());
    }
    if (m_schedule.prelude_tree.get())
    {
        if (verbose<ast_gen>::enabled())
            cout << endl << "** Building AST for prelude." << endl;
        m_in_parallel_for = false;
        output.prelude =
                isl_ast_build_node_from_schedule(build, m_schedule.prelude_tree.copy());
    }
    if (m_schedule.period_tree.get())
    {
        if (verbose<ast_gen>::enabled())
            cout << endl << "** Building AST for period." << endl;
        m_in_parallel_for = false;
        output.period =
                isl_ast_build_node_from_schedule(build, m_schedule.period_tree.copy());
    }

    isl_ast_build_free(build);

    return output;
}

isl_id * ast_gen::before_for(isl_ast_build *builder)
{
    auto id = ast_node_info::create_on_id(m_model.context);

    if (!m_options.parallel)
        return id;

    if (verbose<ast_gen>::enabled())
        cout << "-- Attempting to parallelize for loop.." << endl;

    if (m_in_parallel_for)
    {
        if (verbose<ast_gen>::enabled())
            cout << "   Already in parallel for." << endl;
        return id;
    }

    bool is_parallel = current_schedule_dimension_is_parallel(builder);
    if (!is_parallel)
    {
        if (verbose<ast_gen>::enabled())
            cout << "   Order constraints prevent parallelization." << endl;
        return id;
    }

    if (verbose<ast_gen>::enabled())
        cout << "   Parallelized." << endl;

    auto info = ast_node_info::get_from_id(id);
    info->is_parallel_for = true;
    m_in_parallel_for = true;
    return id;
}

isl_ast_node * ast_gen::after_for(isl_ast_node *node, isl_ast_build *)
{
    auto id = isl_ast_node_get_annotation(node);
    if (!id)
        return node;

    auto info = ast_node_info::get_from_id(id);
    if (info && info->is_parallel_for)
        m_in_parallel_for = false;

    id = isl_id_free(id);

    return node;
}

bool ast_gen::current_schedule_dimension_is_parallel(isl_ast_build * builder)
{
    // FIXME: Storage optimization may introduce additional dependencies!
    // FIXME: isl says some objects are not dereferenced.

    isl::union_map schedule = isl_ast_build_get_schedule(builder);
    isl::space schedule_space = isl_ast_build_get_schedule_space(builder);
    int dimension = schedule_space.dimension(isl::space::output) - 1;

    isl::printer printer(m_model.context);

    if (verbose<ast_gen>::enabled())
    {
        cout << "  Schedule: " << endl;
        printer.print_each_in(schedule);
        cout << "  Current dimension = " << dimension << endl;
    }


    auto dependencies = m_order;

    dependencies.map_domain_through(schedule);
    dependencies.map_range_through(schedule);

    if (dependencies.is_empty())
    {
        return true;
    }

    auto schedule_deps = dependencies.single_map();

    if (verbose<ast_gen>::enabled())
    {
        cout << "  Schedule dependencies: " << endl;
        printer.print_each_in(schedule_deps);
    }

    for (int i = 0; i < dimension; i++)
        schedule_deps = isl_map_equate(schedule_deps.copy(),
                                       isl_dim_out, i,
                                       isl_dim_in, i);

    if (verbose<ast_gen>::enabled())
    {
        cout << "  Unsatisfied schedule dependencies: " << endl;
        printer.print_each_in(schedule_deps);
    }

    auto all_zero_deps = isl::map::universe(schedule_deps.get_space());
    all_zero_deps = isl_map_equate(all_zero_deps.copy(),
                                   isl_dim_out, dimension,
                                   isl_dim_in, dimension);

    return schedule_deps.is_subset_of(all_zero_deps);
}

}
}
