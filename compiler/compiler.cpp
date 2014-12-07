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
    parser.setPrintsTokens(args.print_tokens);

    if (args.print_tokens)
        cout << "== Tokens ==" << endl;

    int success = parser.parse();
    if (success != 0)
        return result::syntactic_error;

    const ast::node_ptr & ast_root = parser.ast();

    if (args.print_ast)
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

    if (args.print_symbols)
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

    if (args.print_polyhedral_model)
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
    auto ast = poly_ast_gen.generate();
    if (!ast.first && !ast.second)
    {
        cout << "No AST generated. Aborting." << endl;
        return result::generator_error;
    }

    // Generate LLVM IR for finite part

    if (ast.first)
    {
        polyhedral::llvm_from_model llvm_from_model
                (module,
                 target.args.size(),
                 poly.statements(),
                 &dataflow_model,
                 polyhedral::initial_schedule);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_model.generate_statement(name, index, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.first,
                             llvm_from_model.start_block(),
                             llvm_from_model.end_block() );
    }

    // Generate LLVM IR for periodic part

    if (ast.second)
    {
        polyhedral::llvm_from_model llvm_from_model
                (module,
                 target.args.size(),
                 poly.statements(),
                 &dataflow_model,
                 polyhedral::periodic_schedule);

        auto stmt_func = [&]
                ( const string & name,
                const vector<llvm::Value*> & index,
                llvm::BasicBlock * block ) -> llvm::BasicBlock*
        {
            return llvm_from_model.generate_statement(name, index, block);
        };

        llvm_cloog.set_stmt_func(stmt_func);
        llvm_cloog.generate( ast.second,
                             llvm_from_model.start_block(),
                             llvm_from_model.end_block() );
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
            if (stmt->buffer.empty())
                buffers.push_back(0);
            else
            {
                int b = stmt->buffer[0];
                for (int i = 1; i < stmt->buffer.size(); ++i)
                    b *= stmt->buffer[i];
                buffers.push_back(b);
            }
        }

        assert(poly.statements().size() > target.args.size());

        JSON::Object output;

        {
            polyhedral::statement *stmt = poly.statements().back();
            const dataflow::actor *actor = dataflow_model.find_actor_for(stmt);

            JSON::Array size;

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
