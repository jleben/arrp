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
#include "../frontend/module_parser.hpp"
#include "../common/ast_printer.hpp"
#include "../common/func_model_printer.hpp"
#include "../frontend/error.hpp"
#include "../frontend/driver.hpp"
#include "../frontend/functional_gen.hpp"
#include "../frontend/reference_analysis.hpp"
#include "../frontend/func_reduction.hpp"
#include "../frontend/folding.hpp"
#include "../frontend/scope_cleanup.hpp"
#include "../frontend/type_check.hpp"
#include "../frontend/collect_ids.hpp"
#include "../frontend/array_reduction.hpp"
#include "../frontend/array_inflate.hpp"
#include "../frontend/array_transpose.hpp"
#include "../frontend/ph_model_gen.hpp"
#include "../polyhedral/scheduling.hpp"
#include "../polyhedral/storage_alloc.hpp"
//#include "../polyhedral/modulo_avoidance.hpp"
#include "../polyhedral/isl_ast_gen.hpp"
#include "../cpp/cpp_target.hpp"
#include "report.hpp"
#include "../interface/raw/generator.h"
#include "../interface/jack/generator.h"
#include "../interface/puredata/generate.h"
#include "../utility/filesystem.hpp"
#include "../utility/subprocess.hpp"

#include <isl-cpp/printer.hpp>
#include <isl-cpp/utility.hpp>
#include <json++/json.hh>

#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>

using namespace std;

namespace stream {
namespace compiler {

void compute_io_latencies(polyhedral::model & ph_model, polyhedral::schedule & schedule);
void report_io(const polyhedral::model & ph_model);

result::code compile(const options & opts)
{
    if (opts.input_filename.empty())
    {
        module_source source;
        source.text = string(istreambuf_iterator<char>(std::cin), {});

        istringstream source_stream(source.text);
        return compile_module(source, source_stream, opts);
    }
    else
    {
        module_source source;
        source.path = opts.input_filename;

        ifstream source_file(opts.input_filename);
        if (!source_file.is_open())
        {
            cerr << "arrp: error: Failed to open input file: '"
                 << opts.input_filename << "'." << endl;
            return result::io_error;
        }

        {
            string::size_type last_sep_pos = opts.input_filename.rfind('/');
            if(last_sep_pos == string::npos)
                last_sep_pos = opts.input_filename.rfind('\\');
            if (last_sep_pos == string::npos)
                last_sep_pos = 0;

            source.dir = opts.input_filename.substr(0, last_sep_pos);
        }

        return compile_module(source, source_file, opts);
    }
}

result::code compile_module
(const module_source & source, istream & text, const options & opts)
{
    module_parser parser;
    parser.set_import_dirs(opts.import_dirs);
    parser.set_import_extensions(opts.import_extensions);

    module * main_module;

    try {
        main_module = parser.parse(source, text);
    } catch (stream::parser_error &) {
        return result::syntactic_error;
    } catch (io_error & e) {
        cerr << e.what() << endl;
        return result::io_error;
    }

    try
    {
        vector<functional::id_ptr> ids;

        {
            functional::generator fgen;
            ids = fgen.generate(parser.modules());
        }

        {
            functional::reference_analysis refs;
            refs.process(ids);
        }

        if (verbose<functional::model>::enabled())
        {
            cerr << "-- Functional model:" << endl;
            functional::printer printer;
            for (const auto & id : ids)
            {
                printer.print(id, cout);
                cout << endl;
            }
            cerr << "--" << endl;
        }

        unordered_set<functional::id_ptr> output_ids;

        for (auto & id : ids)
        {
            if (id->is_output)
                output_ids.insert(id);
        }

        if (output_ids.empty())
        {
            throw source_error("No output defined.", code_location(main_module));
        }

        functional::id_ptr main_id;

        for (auto & id : ids)
        {
            if (!id->is_output)
                continue;
            if (main_id)
                throw source_error("Output defined multiple times.", id->location);
            main_id = id;
        }

        if (!main_id || !main_id->expr)
        {
            auto msg = "No output defined.";
            throw source_error(msg, code_location(main_module));
        }

        functional::name_provider func_name_provider(':');

        {
            arrp::func_reduction func_reducer(func_name_provider);
            func_reducer.reduce(main_id);
        }

        functional::scope global_scope;
        global_scope.ids = ids;

        {
            arrp::scope_cleanup cleanup;
            cleanup.clean(global_scope, main_id);
        }

        if (verbose<functional::model>::enabled())
        {
            cerr << "-- Reduced functions:" << endl;
            functional::printer printer;
            printer.print(global_scope, cout);
            cerr << "--" << endl;
        }

        if (dynamic_pointer_cast<functional::function>(main_id))
        {
            throw source_error("Output must not be a function.", main_id->location);
        }

        {
            arrp::folding folding(func_name_provider);
            folding.process(main_id);

            arrp::scope_cleanup cleanup;
            cleanup.clean(global_scope, main_id);
        }

        if (verbose<functional::model>::enabled())
        {
            cerr << "-- Folded:" << endl;
            functional::printer printer;
            printer.print(global_scope, cout);
            cerr << "--" << endl;
        }

        unordered_set<functional::id_ptr> array_ids;

        {
            functional::type_checker type_checker(func_name_provider);
            type_checker.process(global_scope);
            array_ids = type_checker.ids();

            // Type check does some folding, so we should clean up scopes
            arrp::scope_cleanup cleanup;
            cleanup.clean(global_scope, main_id);
        }

        if (verbose<functional::model>::enabled())
        {
            cerr << "-- Typed:" << endl;
            functional::printer printer;
            printer.print(global_scope, cout);
        }

        // Convert all local ids to global ids
        {
            arrp::lift_local_ids lift_local_ids(global_scope);

            arrp::array_inflate inflater;
            inflater.process(global_scope);

            if (verbose<functional::model>::enabled())
            {
                cout << "-- Lifted local ids:" << endl;
                functional::printer printer;
                printer.print(global_scope, cout);
            }
        }

        {
            functional::array_reducer reducer(func_name_provider);
            array_ids = reducer.process(array_ids);

            arrp::collect_ids collect_ids;
            array_ids = collect_ids.collect(main_id);

            if (verbose<functional::model>::enabled())
            {
                cout << "-- Reduced arrays:" << endl;
                functional::printer printer;
                for (const auto & id : array_ids)
                {
                    printer.print(id, cout);
                    cout << endl;
                }
            }
        }

        {
            functional::array_transposer transposer;
            transposer.process(array_ids);
            if (verbose<functional::model>::enabled())
            {
                cout << "-- Transposed arrays:" << endl;
                functional::printer printer;
                for (const auto & id : array_ids)
                {
                    printer.print(id, cout);
                    cout << endl;
                }
            }
        }

        {
            // Create polyhedral model

            polyhedral::model ph_model;

            {
                functional::polyhedral_gen::options ph_opts;
                ph_opts.atomic_io = opts.atomic_io;
                ph_opts.ordered_io = opts.ordered_io;

                functional::polyhedral_gen gen(ph_opts);
                ph_model = gen.process(array_ids);

                gen.add_output(ph_model, main_id, opts.atomic_io, opts.ordered_io);
            }

            // Drop statement instances which write elements which are never read

            {
                isl::printer printer(ph_model.context);
                polyhedral::model_summary summary(ph_model);

                auto read_elements = summary.read_relations.in_domain(summary.domains).range();
                auto useful_domains = summary.write_relations.in_domain(summary.domains).inverse()(read_elements);

                for (auto stmt : ph_model.statements)
                {
                    if (stmt->is_input_or_output)
                        continue;
                    stmt->domain &= useful_domains.set_for(stmt->domain.get_space());
                    stmt->domain.coalesce();
                    if (verbose<functional::polyhedral_gen>::enabled())
                    {
                        cout << "Restricted statement domain: ";
                        printer.print(stmt->domain);
                        cout << endl;
                    }
                }
            }

            if (opts.clocked_io)
            {
                functional::add_io_clock(ph_model);
            }

            // Compute polyhedral schedule

            polyhedral::schedule schedule(ph_model.context);

            {
                polyhedral::scheduler::options sched_opts;
                sched_opts.cluster = opts.schedule.cluster;
                sched_opts.periodic_tile_direction = opts.schedule.periodic_tile_direction;
                sched_opts.period_offset = opts.schedule.period_offset;
                sched_opts.period_scale = opts.schedule.period_scale;
                sched_opts.tile_size = opts.schedule.tile_size;
                sched_opts.tile_parallelism = opts.schedule.tile_parallelism;
                sched_opts.intra_tile_permutation = opts.schedule.intra_tile_permutation;

                polyhedral::scheduler poly_scheduler( ph_model );
                schedule = poly_scheduler.schedule(sched_opts);
            }

            // Generate AST for schedule

            polyhedral::ast_isl ast;

            {
                polyhedral::ast_gen::options ast_opts;
                ast_opts.separate_loops = opts.separate_loops;
                ast_opts.parallel = opts.parallel;
                ast_opts.parallel_dim = opts.parallel_dim;
                ast_opts.vectorize = opts.vectorize;

                polyhedral::ast_gen ast_gen(ph_model, schedule, ast_opts);

                ast = ast_gen.generate();
            }

            // Allocate storage (buffers)

            polyhedral::storage_allocator storage_alloc( ph_model, opts.classic_storage_allocation );
            storage_alloc.allocate(schedule);

            compute_io_latencies(ph_model, schedule);

            // Modulo avoidance

            {
                // FIXME: It's broken and incomplete
                //avoid_modulo(schedule, ph_model, opts.split_statements);
            }

            if (verbose<polyhedral::ast_isl>::enabled())
            {
                isl::printer printer(ph_model.context);
                printer.set_format(isl::printer::c_format);
                if (ast.full)
                {
                    cout << "AST for full schedule:" << endl;
                    isl_printer_print_ast_node(printer.get(), ast.full);
                }
                if (ast.prelude)
                {
                    cout << "AST for prelude:" << endl;
                    isl_printer_print_ast_node(printer.get(), ast.prelude);
                }
                if (ast.period)
                {
                    cout << "AST for period:" << endl;
                    isl_printer_print_ast_node(printer.get(), ast.period);
                }
            }

            report_io(ph_model);

            string output_filename_base = opts.output_filename_base;
            if (output_filename_base.empty())
                output_filename_base = main_module->name;

            // Generate C++ output

            {
                string namespace_name = opts.cpp.nmspace;
                if (namespace_name.empty())
                    namespace_name = "arrp_module_" + main_module->name;

                string filename = output_filename_base + ".h";

                arrp::report()["cpp"]["namespace"] = namespace_name;
                arrp::report()["cpp"]["filename"] = filename;

                if (verbose<compiler::log>::enabled())
                    cerr << "Opening C++ output file: " << filename << endl;

                ofstream cpp_file(filename);
                if (!cpp_file.is_open())
                {
                    cerr << "Could not open C++ output file: "
                         << filename << endl;
                    return result::io_error;
                }

                cpp_gen::generate(namespace_name,
                                  ph_model,
                                  ast,
                                  cpp_file,
                                  opts);
            }

            if (opts.target_type == "generic")
            {
                arrp::generic_io::options output_opt;

                output_opt.base_file_name = output_filename_base;

                arrp::generic_io::generate(output_opt, arrp::report());
            }
            else if (opts.target_type == "jack")
            {
                arrp::jack_io::options output_opt;

                output_opt.base_file_name = output_filename_base;

                output_opt.client_name = opts.jack_io.name;
                if (output_opt.client_name.empty())
                    output_opt.client_name = "Arrp module " + main_module->name;

                arrp::jack_io::generate(output_opt, arrp::report());
            }
            else if (opts.target_type == "puredata")
            {
                arrp::puredata_io::options pd_opt;

                pd_opt.base_file_name = output_filename_base;

                pd_opt.pd_object_name = opts.puredata_io.name;
                if (pd_opt.pd_object_name.empty())
                    pd_opt.pd_object_name = "arrp_" + main_module->name;

                arrp::puredata_io::generate(pd_opt, arrp::report());
            }
        }
    }
    catch (source_error & e)
    {
        print(source_error::critical, e);
        return result::semantic_error;
    }
    catch(error & e)
    {
        cout << "ERROR: " << e.what() << endl;
        return result::generator_error;
    }

    if (!opts.report_file.empty())
    {
        ofstream out(opts.report_file);
        if (!out.is_open())
        {
            cerr << "Warning: Failed to open report file: " << opts.report_file << endl;
        }
        else
        {
            out << arrp::report().dump(4) << endl;
        }
    }

    return result::ok;
}

void compute_io_latencies(polyhedral::model & ph_model, polyhedral::schedule & schedule)
{
    arrp::json report;

    isl::space sched_space(nullptr);
    schedule.full.for_each([&](const isl::map & m){
        sched_space = m.get_space().range();
        return false;
    });

    const auto & out = ph_model.outputs.front();
    if (!out.statement->is_infinite)
    {
        if (verbose<io_latency>::enabled())
            cerr << "Output is not a stream." << endl;
    }
    else
    {
        for (auto & in : ph_model.inputs)
        {
            if (!in.statement->is_infinite)
            {
                report[in.name] = "not a stream";
                continue;
            }

            // Let r be the ratio between the input and output rate: r = in_rate / out_rate.
            // Then, for any output o, and latest preceding input i,
            // latency = i - o * r,
            // i.e. latency is the difference between actual i and expected i
            // if the rate was constant and there was no offset.

            //isl::printer p(ph_model.context);
            auto in_sched = schedule.full.map_for(isl::space::from(in.statement->domain.get_space(), sched_space));
            auto out_sched = schedule.full.map_for(isl::space::from(out.statement->domain.get_space(), sched_space));
            auto precedence = out_sched.cross(in_sched).in_range(isl::order_greater_than(sched_space).wrapped()).domain();
            //cout << "Precedence: "; p.print(precedence); cout << endl;
            int out_var = 0;
            int in_var = out.statement->domain.dimensions();
            auto space = precedence.get_space();
            auto latency = space.var(in_var) - isl::floor(space.var(out_var) * in.array->period / out.array->period);
            //cout << "Latency: "; p.print(latency); cout << endl;
            auto max_latency = precedence.maximum(latency);
            if (max_latency.is_infinity())
            {
                in.latency = -1;

                report[in.name] = "infinite";
            }
            else if (max_latency.is_integer())
            {
                in.latency = max_latency.integer();

                report[in.name] = max_latency.integer();
            }
            else
            {
                throw error("Could not compute input-output latency for input " + in.name);
            }
        }
    }

    if (verbose<io_latency>::enabled())
        arrp::report()["latencies"] = report;
}

arrp::json io_channel_report(const polyhedral::io_channel & channel)
{
    // FIXME: Input order does not correspond to Arrp source.

    arrp::json report;

    report["name"] = channel.name;
    report["is_stream"] = channel.array->is_infinite;

    ostringstream type;
    type << channel.array->type;
    report["type"] = type.str();

    // FIXME: Size and count does not take into account atomic IO.

    int64_t size = 1;
    for(auto & s : channel.array->size)
    {
        if (s >= 0)
            size *= s;
    }

    report["size"] = size;

    for(auto & s : channel.array->size)
    {
        if (s >= 0)
            report["dimensions"].push_back(s);
    }

    if (channel.array->is_infinite)
    {
        report["period_count"] = channel.array->period;
    }

    return report;
}

void report_io(const polyhedral::model & ph_model)
{
    arrp::json inputs_report;

    for (auto & in : ph_model.inputs)
    {
        inputs_report.push_back(io_channel_report(in));
    }

    arrp::json outputs_report;

    for (auto & out : ph_model.outputs)
    {
        outputs_report.push_back(io_channel_report(out));
    }

    arrp::report()["inputs"] = inputs_report;
    arrp::report()["outputs"] = outputs_report;
}


} // namespace compiler
} // namespace stream
