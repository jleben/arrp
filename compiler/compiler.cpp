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

using namespace std;

namespace stream {
namespace compiler {

result::code compile(const arguments & args)
{
    if (args.input_filename.empty())
    {
        cerr << "streamc: error: Missing argument: input filename." << endl;
        return result::command_line_error;
    }

    ifstream source_file(args.input_filename);
    if (!source_file.is_open())
    {
        cerr << "streamc: error: Failed to open input file: '"
             << args.input_filename << "'." << endl;
        return result::io_error;
    }

    string input_dir;
    {
        string::size_type last_sep_pos = args.input_filename.rfind('/');
        if(last_sep_pos == string::npos)
            last_sep_pos = args.input_filename.rfind('\\');
        if (last_sep_pos == string::npos)
            last_sep_pos = 0;

        input_dir = args.input_filename.substr(0, last_sep_pos);
    }

    module_source source;
    source.dir = input_dir;
    source.path = args.input_filename;

    return compile_module(source, source_file, args);
}

result::code compile_module
(const module_source & source, istream & text, const arguments & args)
{
    module_parser parser;
    parser.set_import_dirs(args.import_dirs);

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

        if (verbose<functional::model>::enabled())
        {
            functional::printer printer;
            for (const auto & id : ids)
            {
                printer.print(id, cout);
                cout << endl;
            }
        }

        // FIXME: choice of function to compile
        auto criteria = [&main_module](functional::id_ptr id) -> bool {
            return id->name == "main";
        };
        auto id_it = std::find_if(ids.begin(), ids.end(), criteria);
        if (id_it == ids.end())
        {
            throw error("No function named \"main\" in module \""
                        + main_module->name + "\".");
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

            functional::polyhedral_gen gen;
            auto ph_model = gen.process(array_ids);
            gen.add_output(ph_model, "output", id);

            // Compute polyhedral schedule

            polyhedral::scheduler poly_scheduler( ph_model );

            auto schedule = poly_scheduler.schedule(args.optimize_schedule,
                                                    args.sched_reverse);

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
                avoid_modulo(schedule, ph_model, args.split_statements);
            }

            // Generate AST for schedule

            auto ast = polyhedral::make_isl_ast(schedule, args.separate_loops);

            if (verbose<polyhedral::ast_isl>::enabled())
            {
                isl::printer printer(ph_model.context);
                printer.set_format(isl::printer::c_format);
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

            if (!args.cpp_output_filename.empty())
            {
                {
                    string cpp_filename = args.cpp_output_filename + ".cpp";
                    ofstream cpp_file(cpp_filename);
                    if (!cpp_file.is_open())
                    {
                        cerr << "Could not open C++ output file: "
                             << cpp_filename << endl;
                        return result::io_error;
                    }

                    string hpp_filename = args.cpp_output_filename + ".h";
                    ofstream hpp_file(hpp_filename);
                    if (!hpp_file.is_open())
                    {
                        cerr << "Could not open C++ header output file: "
                             << hpp_filename << endl;
                        return result::io_error;
                    }

                    cpp_gen::generate(args.cpp_output_filename,
                                      ph_model,
                                      ast,
                                      cpp_file,
                                      hpp_file);
                }
            }
        }
    }
    catch (functional::func_reduce_error & e)
    {
        cerr << "** ERROR " << e.location << ": " << e.what() << endl;
        print_code_range(cerr, e.location.path(), e.location.range);
        while(!e.trace.empty())
        {
            auto location = e.trace.top();
            e.trace.pop();
            cout << ".. from " << location << endl;
        }
        return result::semantic_error;
    }
    catch (source_error & e)
    {
        cerr << "** ERROR " << e.location << ": " << e.what() << endl;
        print_code_range(cerr, e.location.path(), e.location.range);
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
