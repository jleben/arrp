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
//#include "../common/polyhedral_model_printer.hpp"
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
//#include "../polyhedral/ast_gen.hpp"
#include "../polyhedral/isl_ast_gen.hpp"
//#include "../llvm/llvm_ir_from_cloog.hpp"
//#include "../llvm/llvm_from_polyhedral.hpp"
#include "../cpp/cpp_target.hpp"
//#include "../interface/cpp-intf-gen.hpp"

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
            return id->name == main_module->name + ".main";
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

#if 0
        {
            // Rename ids for C++ compatibility

            unordered_set<string> unique_names;
            vector<char> forbidden = { ':', '.' };
            for (auto & id : array_ids)
            {
                string name = id->name;
                for (auto & c : name)
                {
                    if (std::count(forbidden.begin(), forbidden.end(), c))
                    {
                        c = '_';
                    }
                }

                int i = 0;
                string unique_name = name;
                while (unique_names.find(unique_name) != unique_names.end())
                {
                    unique_name = name + to_string(++i);
                }

                if (verbose<cpp_gen::renaming>::enabled())
                    cout << "Renaming id: " << id->name << " -> " << unique_name << endl;

                id->name = unique_name;
            }
        }
#endif
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
#if 0
            auto ast = polyhedral::make_ast(schedule);
            if (args.print[arguments::ast_output])
            {
                cout << endl << "== Prelude AST ==" << endl;
                clast_pprint(stdout, ast.prelude, 0, ast.options);
                cout << endl << "== Period AST ==" << endl;
                clast_pprint(stdout, ast.period, 0, ast.options);
            }
#endif
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


#if STARTED_WORKING_ON_POLYHEDRAL_MODEL
    polyhedral::model_generator poly_gen(env);
    auto poly_model = poly_gen.generate( sym_iter->second, target.args );

    return result::generator_error;

    return compile_polyhedral_model(poly_model, args);
#endif
}

#if STARTED_WORKING_ON_POLYHEDRAL_MODEL
result::code compile_polyhedral_model
(const polyhedral::model & model,
 const arguments & args)
{

    // Print polyhedral model

    if (args.print[arguments::polyhedral_model_output])
    {
        polyhedral::printer poly_printer;
        cout << endl << "== Polyhedral Model ==" << endl;
        for( const auto & stmt : model.statements )
        {
            cout << endl;
            poly_printer.print(stmt.get(), cout);
        }
    }

    const target_info & target = args.target;

    // Construct AST from polyhedral and dataflow models

    polyhedral::ast_generator poly_ast_gen( model );
    poly_ast_gen.set_print_ast_enabled(args.print[arguments::target_ast_output]);

    auto ast = poly_ast_gen.generate();
    if (!ast.first && !ast.second)
    {
        cout << "No AST generated. Aborting." << endl;
        return result::generator_error;
    }

    // Print buffers

    if (args.print[arguments::buffer_size_output])
    {
        print_buffer_sizes(model.arrays);
    }

#if 0
    // Generate LLVM IR

    llvm::Module *module = new llvm::Module(args.input_filename,
                                            llvm::getGlobalContext());

    llvm_gen::llvm_from_cloog llvm_cloog(module);

    llvm_gen::llvm_from_polyhedral llvm_from_polyhedral
            (module, statements, nullptr);


    // Generate LLVM IR for finite part

    if (ast.first)
    {
        auto ctx = llvm_from_polyhedral.create_process_function
                (llvm_gen::initial_schedule, target.args);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_polyhedral.generate_statement(name, index, ctx, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.first,
                             ctx.start_block,
                             ctx.end_block );
    }

    // Generate LLVM IR for periodic part

    if (ast.second)
    {
        auto ctx = llvm_from_polyhedral.create_process_function
                (llvm_gen::periodic_schedule, target.args);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_polyhedral.generate_statement(name, index, ctx, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.second,
                             ctx.start_block,
                             ctx.end_block );
    }
#endif
    // Output C++ interface

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

            cpp_gen::generate(target.name, target.args,
                              model,
                              ast.first, ast.second,
                              cpp_file,
                              hpp_file);
        }

#if 0
        cpp_output_file << "#ifndef stream_function_" << target.name << "_included" << endl;
        cpp_output_file << "#define stream_function_" << target.name << "_included" << endl;
        cpp_output_file << endl;
        cpp_output_file << "#include <cstdint>" << endl;

        auto program = cpp_interface::create(target.name, target.args,
                                           statements,
                                           dataflow_model);
        cpp_gen::options opt;
        cpp_gen::state state(opt);
        try
        {
            program->generate(state, cpp_output_file);
        }
        catch (std::exception & e)
        {
            cerr << "Error generating C++ interface output file: "
                 << e.what()
                 << endl;
            return result::io_error;
        }
        catch (...)
        {
            cerr << "Error generating C++ interface output file." << endl;
            return result::io_error;
        }

        cpp_output_file << "#endif // stream_function_" << target.name << "_included" << endl;
#endif
    }

    // Output meta-data

    if (!args.meta_output_filename.empty())
    {
        cerr << "WARNING: meta-info output currently disabled." << endl;

        // FIXME: Broken input info
#if 0
        JSON::Object description;

        JSON::Array inputs;

        for(int in_idx = 0; in_idx < target.args.size(); ++in_idx)
        {
            JSON::Object input;
            JSON::Array size;

            polyhedral::statement *stmt = statements[in_idx];
            const dataflow::actor *actor = dataflow_model.find_actor_for(stmt);

            switch (stmt->expr->type )
            {
            case polyhedral::integer:
                input["type"] = "integer";
                break;
            case polyhedral::real:
                input["type"] = "real";
                break;
            case polyhedral::boolean:
                input["type"] = "boolean";
                break;
            default:
                input["type"] = "unknown";
            }

            if (actor)
            {
                input["init"] = actor->init_count;
                input["period"] = actor->steady_count;
                for (int dim = 0; dim < stmt->domain.size(); ++dim)
                {
                    if(dim == actor->flow_dimension)
                        continue;
                    size.push_back(stmt->domain[dim]);
                }

            }
            else
            {
                input["init"] = stmt->domain[0];
                input["period"] = 0;
                for (int dim = 1; dim < stmt->domain.size(); ++dim)
                    size.push_back(stmt->domain[dim]);
            }

            input["size"] = size;

            inputs.push_back(input);
        }

        JSON::Array buffers;

        for(polyhedral::statement *stmt : statements)
        {
            JSON::Object buffer;

            switch (stmt->expr->type )
            {
            case polyhedral::integer:
                buffer["type"] = "integer";
                break;
            case polyhedral::real:
                buffer["type"] = "real";
                break;
            case polyhedral::boolean:
                buffer["type"] = "boolean";
                break;
            default:
                buffer["type"] = "unknown";
            }

            if (stmt->buffer.empty())
                buffer["size"] = 0;
            else
            {
                int b = stmt->buffer[0];
                for (int i = 1; i < stmt->buffer.size(); ++i)
                    b *= stmt->buffer[i];
                buffer["size"] = b;
            }

            buffers.push_back(buffer);
        }

        assert(statements.size() > 0);

        JSON::Object output;

        {
            polyhedral::statement *stmt = statements.back();
            const dataflow::actor *actor = dataflow_model.find_actor_for(stmt);

            JSON::Array size;

            switch (stmt->expr->type )
            {
            case polyhedral::integer:
                output["type"] = "integer";
                break;
            case polyhedral::real:
                output["type"] = "real";
                break;
            case polyhedral::boolean:
                output["type"] = "boolean";
                break;
            default:
                output["type"] = "unknown";
            }

            if (actor)
            {
                output["init"] = actor->init_count;
                output["period"] = actor->steady_count;
                for (int dim = 0; dim < stmt->domain.size(); ++dim)
                {
                    if(dim == actor->flow_dimension)
                        continue;
                    size.push_back(stmt->domain[dim]);
                }
            }
            else
            {
                output["init"] = stmt->domain[0];
                output["period"] = 0;
                for (int dim = 1; dim < stmt->domain.size(); ++dim)
                    size.push_back(stmt->domain[dim]);
            }

            output["size"] = size;
        }

        description["inputs"] = inputs;
        description["output"] = output;
        description["buffers"] = buffers;

        ofstream output_file(args.meta_output_filename);
        if (!output_file.is_open())
        {
            cerr << "ERROR: Could not open description output file: "
                 << args.meta_output_filename << endl;
            return result::io_error;
        }

        output_file << description;
#endif
    }

#if 0
    // Output LLVM IR

    ofstream output_file(args.output_filename);
    if (!output_file.is_open())
    {
        cerr << "ERROR: Could not open output file for writing!" << endl;
        return result::io_error;
    }

    llvm_cloog.output(output_file);

    if (!llvm_cloog.verify())
        return result::generator_error;
#endif
    return result::ok;
}

#endif // STARTED_WORKING_ON_PH_MODEL


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
