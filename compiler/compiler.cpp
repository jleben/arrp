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
#include "../frontend/func_reducer.hpp"
#include "../frontend/array_reduction.hpp"
#include "../frontend/array_transpose.hpp"
#include "../frontend/type_check.hpp"
#include "../new-type-system/type_check.hpp"
#include "../frontend/ph_model_gen.hpp"
#include "../polyhedral/scheduling.hpp"
#include "../polyhedral/storage_alloc.hpp"
#include "../polyhedral/modulo_avoidance.hpp"
#include "../polyhedral/isl_ast_gen.hpp"
#include "../cpp/cpp_target.hpp"

#include <isl-cpp/printer.hpp>

#include <json++/json.hh>

#include <fstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <numeric>

using namespace std;

namespace stream {
namespace compiler {

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
        unordered_set<functional::id_ptr> ids;

        {
            functional::generator fgen;
            auto id_vector = fgen.generate(parser.modules());
            ids.insert(id_vector.begin(), id_vector.end());
        }

        if (verbose<functional::model>::enabled())
        {
            functional::printer printer;
            printer.set_print_scopes(true);
            for (const auto & id : ids)
            {
                printer.print(id, cout);
                cout << endl;
            }
        }

        {
            arrp::built_in_types builtins;
            arrp::type_checker type_checker(&builtins);
            type_checker.process(ids);

            return result::ok;
        }

        // FIXME: choice of function to compile
        auto criteria = [&main_module](functional::id_ptr id) -> bool {
            return id->name == "main";
        };
        auto id_it = std::find_if(ids.begin(), ids.end(), criteria);
        if (id_it == ids.end())
        {
            auto msg = "No function named \"main\" in module \""
                    + main_module->name + "\".";
            throw source_error(msg, code_location(main_module));
        }
        auto id = *id_it;

        unordered_set<functional::id_ptr> array_ids;

        functional::name_provider func_name_provider(':');

        {
            functional::func_reducer reducer(func_name_provider);
            id = reducer.reduce(id, {});
            array_ids = reducer.ids();

            if (verbose<functional::model>::enabled())
            {
                cout << "-- Reduced functions:" << endl;
                functional::printer printer;
                printer.set_print_scopes(false);
                for (const auto & id : array_ids)
                {
                    printer.print(id, cout);
                    cout << endl;
                }
            }
        }

        if (verbose<functional::type_checker>::enabled())
        {
            cout << "Type = " << *id->expr->type << endl;
        }

        if (id->expr->type->is_function())
        {
            cerr << "Functions not supported in output. "
                 << "All functions must be fully applied."
                 << endl;
            return result::semantic_error;
        }

        {
            functional::array_reducer reducer(func_name_provider);
            id = reducer.process(id);
            array_ids = reducer.ids();
            array_ids.insert(id);

            if (verbose<functional::model>::enabled())
            {
                cout << "-- Reduced arrays:" << endl;
                functional::printer printer;
                printer.set_print_scopes(false);
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
                printer.set_print_scopes(false);
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
                functional::polyhedral_gen gen;
                ph_model = gen.process(array_ids);
                gen.add_output(ph_model, "output", id);
            }

            // Compute polyhedral schedule

            polyhedral::schedule schedule(ph_model.context);

            {
                polyhedral::scheduler::options sched_opts;
                sched_opts.cluster = opts.schedule.cluster;
                sched_opts.period_offset = opts.schedule.period_offset;
                sched_opts.period_scale = opts.schedule.period_scale;

                polyhedral::scheduler poly_scheduler( ph_model );
                schedule = poly_scheduler.schedule(sched_opts);
            }

            // Allocate storage (buffers)

            polyhedral::storage_allocator storage_alloc( ph_model );
            storage_alloc.allocate(schedule);

            // Print buffers

            if (verbose<polyhedral::storage_output>::enabled())
            {
                print_buffer_sizes(ph_model.arrays);
            }

            // Modulo avoidance

            {
                // FIXME: It's broken and incomplete
                //avoid_modulo(schedule, ph_model, opts.split_statements);
            }

            // Generate AST for schedule

            auto ast = polyhedral::make_isl_ast(schedule, opts.separate_loops);

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

            // Generate C++ output

            if (opts.cpp.enabled)
            {
                string file_base = opts.cpp.filename;
                if (file_base.empty())
                    file_base = main_module->name;

                string cpp_filename = file_base + ".cpp";
                ofstream cpp_file(cpp_filename);
                if (!cpp_file.is_open())
                {
                    cerr << "Could not open C++ output file: "
                         << cpp_filename << endl;
                    return result::io_error;
                }

                string nmspace = opts.cpp.nmspace;
                if (nmspace.empty())
                    nmspace = main_module->name;
#if 0
                string hpp_filename = file_base + ".h";
                ofstream hpp_file(hpp_filename);
                if (!hpp_file.is_open())
                {
                    cerr << "Could not open C++ header output file: "
                         << hpp_filename << endl;
                    return result::io_error;
                }
#endif

                cpp_gen::generate(nmspace,
                                  ph_model,
                                  ast,
                                  cpp_file);
            }
        }
    }
    catch (source_error & e)
    {
        list<code_location> reverse_locations;
        while(!e.trace.empty())
        {
            auto location = e.trace.top();
            if (!location.module)
                break;
            reverse_locations.push_front(location);
            e.trace.pop();
        }
        for (auto & location : reverse_locations)
            cout << ".. From " << location << endl;
        cerr << "** ERROR: " << e.location << ": " << e.what() << endl;
        if (e.location.module)
            print_code_range(cerr, e.location.module->source, e.location.range);
        return result::semantic_error;
    }
    /*
    catch(error & e)
    {
        cout << "ERROR: " << e.what() << endl;
        return result::semantic_error;
    }*/

    return result::ok;
}

void print_buffer_sizes(const vector<stream::polyhedral::array_ptr> & arrays)
{
    cout << endl << "== Buffer sizes ==" << endl;
    for (const auto & array : arrays)
    {
        cout << array->name << ": ";
        cout << "[ ";
        for (auto b : array->buffer_size)
        {
            cout << b << " ";
        }
        cout << "]";
        int flat_size = 0;
        if (!array->buffer_size.empty())
            flat_size = std::accumulate(array->buffer_size.begin(),
                                        array->buffer_size.end(),
                                        1, std::multiplies<int>());
        cout << " = " << flat_size;
        cout << endl;
    }
}


} // namespace compiler
} // namespace stream
