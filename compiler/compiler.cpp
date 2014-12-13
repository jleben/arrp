#include "arg_parser.hpp"
#include "../frontend/parser.h"
#include "../frontend/ast_printer.hpp"
#include "../frontend/type_checker.hpp"
#include "../frontend/ir-generator.hpp"
#include "../polyhedral/translator.hpp"
#include "../polyhedral/printer.hpp"
#include "../polyhedral/ast_generator.hpp"
//#include "../polyhedral/llvm_ir_generator.hpp"
#include "../polyhedral/llvm_ir_from_cloog.hpp"
#include "../polyhedral/llvm_from_model.hpp"
#include "../interface/cpp-intf-gen.hpp"

#include <json++/json.hh>

#include <fstream>
#include <iostream>
#include <functional>

using namespace std;

using namespace stream;
using namespace stream::compiler;

namespace result {
enum code
{
    ok = 0,
    command_line_error,
    io_error,
    syntactic_error,
    symbolic_error,
    semantic_error,
    generator_error
};
}

int main(int argc, char *argv[])
{
    arguments args(argc-1, argv+1);

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

    for(const string & topic : args.debug_topics)
        debug::set_status_for_id(topic, debug::enabled);
    for(const string & topic : args.no_debug_topics)
        debug::set_status_for_id(topic, debug::disabled);

    if (args.input_filename.empty())
    {
        cerr << "streamc: error: Missing argument: input filename." << endl;
        return result::command_line_error;
    }

    ifstream input_file(args.input_filename);
    if (!input_file.is_open())
    {
        cerr << "streamc: error: Failed to open input file: '"
             << args.input_filename << "'." << endl;
        return result::io_error;
    }

    stream::Parser parser(input_file);
    parser.setPrintsTokens(args.print[arguments::tokens_output]);

    if (args.print[arguments::tokens_output])
        cout << "== Tokens ==" << endl;

    int success = parser.parse();
    if (success != 0)
        return result::syntactic_error;

    const ast::node_ptr & ast_root = parser.ast();

    if (args.print[arguments::ast_output])
    {
        cout << endl;
        cout << "== Abstract Syntax Tree ==" << endl;
        stream::ast::printer printer;
        printer.print(ast_root.get());
        cout << endl;
    }

    stream::semantic::environment env;
    stream::semantic::environment_builder env_builder(env);
    if (!env_builder.process(ast_root))
        return result::symbolic_error;

    if (args.print[arguments::symbols_output])
    {
        cout << endl;
        cout << "== Environment ==" << endl;
        cout << env;
    }

    if (args.target.name.empty())
        return result::ok;

    semantic::type_checker type_checker(env);

    //IR::generator gen(args.input_filename, env);

    polyhedral::translator poly(env);
    polyhedral::printer poly_printer;

    llvm::Module *module = new llvm::Module(args.input_filename,
                                            llvm::getGlobalContext());
    //polyhedral::llvm_ir_generator poly_llvm_gen(args.input_filename);
    polyhedral::llvm_from_cloog llvm_cloog(module);

    const target_info & target = args.target;

    cout << endl;
    cout << "== Generating: " << target.name;
    cout << "(";
    if (target.args.size())
        cout << *target.args.front();
    for ( int i = 1; i < target.args.size(); ++i )
    {
        cout << ", ";
        cout << *target.args[i];
    }
    cout << ")" << endl;

    auto sym_iter = env.find(target.name);
    if (sym_iter == env.end())
    {
        cerr << "ERROR: no symbol '" << target.name << "' available." << endl;
        return result::command_line_error;
    }

    semantic::type_ptr result_type =
            type_checker.check(sym_iter->second, target.args);

    if (type_checker.has_error())
        return result::semantic_error;

    cout << "Type: " << *result_type << endl;

    if (result_type->is(semantic::type::function))
    {
        sym_iter = env.find(result_type->as<semantic::function>().name);
        assert(sym_iter != env.end());
    }

    poly.translate( sym_iter->second, target.args );

    if (args.print[arguments::polyhedral_model_output])
    {
        cout << endl << "== Polyhedral Model ==" << endl;
        for( polyhedral::statement * stmt : poly.statements() )
        {
            cout << endl;
            poly_printer.print(stmt, cout);
        }
    }

    // Construct dataflow model

    dataflow::model dataflow_model(poly.statements());

    // Construct AST from polyhedral and dataflow models

    polyhedral::ast_generator poly_ast_gen( poly.statements(),
                                            &dataflow_model );
    poly_ast_gen.set_print_ast_enabled(args.print[arguments::target_ast_output]);

    auto ast = poly_ast_gen.generate();
    if (!ast.first && !ast.second)
    {
        cout << "No AST generated. Aborting." << endl;
        return result::generator_error;
    }

    // Generate LLVM IR

    polyhedral::llvm_from_model llvm_from_model
            (module, poly.statements(), &dataflow_model);

    // Generate LLVM IR for finite part

    if (ast.first)
    {
        auto ctx = llvm_from_model.create_process_function
                (polyhedral::initial_schedule, target.args);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_model.generate_statement(name, index, ctx, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.first,
                             ctx.start_block,
                             ctx.end_block );
    }

    // Generate LLVM IR for periodic part

    if (ast.second)
    {
        auto ctx = llvm_from_model.create_process_function
                (polyhedral::periodic_schedule, target.args);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_model.generate_statement(name, index, ctx, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.second,
                             ctx.start_block,
                             ctx.end_block );
    }

    // Output C++ interface

    if (!args.cpp_output_filename.empty())
    {
        ofstream cpp_output_file(args.cpp_output_filename);
        if (!cpp_output_file.is_open())
        {
            cerr << "Could not open C++ interface output file: "
                 << args.cpp_output_filename << endl;
            return result::io_error;
        }

        cpp_output_file << "#ifndef stream_function_" << target.name << "_included" << endl;
        cpp_output_file << "#define stream_function_" << target.name << "_included" << endl;
        cpp_output_file << endl;
        cpp_output_file << "#include <cstdint>" << endl;

        auto program = cpp_interface::create(target.name, target.args,
                                           poly.statements(),
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
    }

    // Output meta-data

    if (!args.meta_output_filename.empty())
    {
        JSON::Object description;

        JSON::Array inputs;

        for(int in_idx = 0; in_idx < target.args.size(); ++in_idx)
        {
            JSON::Object input;
            JSON::Array size;

            polyhedral::statement *stmt = poly.statements()[in_idx];
            const dataflow::actor *actor = dataflow_model.find_actor_for(stmt);

            switch (stmt->expr->type )
            {
            case polyhedral::integer:
                input["type"] = "integer";
                break;
            case polyhedral::real:
                input["type"] = "real";
                break;
            default:
                assert(false);
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

        for(polyhedral::statement *stmt : poly.statements())
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
            default:
                assert(false);
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

        assert(poly.statements().size() > 0);

        JSON::Object output;

        {
            polyhedral::statement *stmt = poly.statements().back();
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
            default:
                assert(false);
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
    }

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

    return result::ok;
}
