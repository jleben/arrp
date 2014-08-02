#include "parser.h"
#include "ast_printer.hpp"
#include "type_checker.hpp"
#include "ir-generator.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <fstream>
#include <iostream>

using namespace std;
using namespace stream;

void print_help()
{
    cout << "Usage:" << endl
         << "  test <input file> [<parameter>...]" << endl;

    cout << "Parameters:" << endl;

    cout << "  --print-tokens or -t : Print all tokens produced by lexical scanner." << endl;
    cout << "  --print-ast or -s : Print abstract syntax tree (AST) produced by parser." << endl;
    cout << "  --list-symbols or -l : List all top-level declarations." << endl;
    cout << "  --eval or -e <symbol> [<arg>...] :" << endl
         << "  \tEvaluate a top-level declaration of <symbol> and print the result type." << endl
         << "  \tEach following argument <arg> is passed as argument type in function evaluation." << endl
         << "  \tAn argument can be:" << endl
         << "  \t- a constant integer number (e.g. \"123\")," << endl
         << "  \t- a constant real number (e.g. \"123.45\"), " << endl
         << "  \t- a stream (e.g. \"[10,4,5]\" with each number representing size in one dimension)." << endl
         ;
}

struct evaluation
{
    string name;
    vector<sp<semantic::type>> args;
};

struct argument_parser
{
    struct abortion {};

    bool print_tokens = false;
    bool print_ast = false;
    bool print_symbols = false;
    vector<evaluation> evaluations;

    argument_parser(int argc, char *argv[]):
        m_arg_count(argc),
        m_args(argv)
    {}

    void parse()
    {
        while(m_arg_count)
            parse_next_arg();
    }

private:

    void advance()
    {
        --m_arg_count;
        ++m_args;
    }

    void parse_next_arg()
    {
        string arg( m_args[0] );

        if (arg == "--help" || arg == "-h")
        {
            print_help();
            throw abortion();
        }
        else if (arg == "--print-tokens" || arg == "-t")
        {
            print_tokens = true;
            advance();
        }
        else if (arg == "--print-ast" || arg == "-s")
        {
            print_ast = true;
            advance();
        }
        else if (arg == "--list-symbols" || arg == "-l")
        {
            print_symbols = true;
            advance();
        }
        else if (arg == "--evaluate" || arg == "--eval" || arg == "-e")
        {
            advance();
            parse_evaluation(arg);
        }
        else
        {
            ostringstream msg;
            msg << "Invalid argument: " << arg;
            throw std::runtime_error(msg.str());
        }
    }

    void parse_evaluation(const string & parameter)
    {
        evaluations.emplace_back();
        evaluation &eval = evaluations.back();

        string arg( m_args[0] );
        if (arg[0] == '-')
        {
            ostringstream msg;
            msg << "Missing function name after " << parameter << " parameter.";
            throw std::runtime_error(msg.str());
        }

        eval.name = arg;

        advance();

        while(m_arg_count && m_args[0][0] != '-')
        {
            string arg(m_args[0]);
            eval.args.emplace_back( parse_eval_arg(arg) );
            advance();
        }
    }

    sp<semantic::type> parse_eval_arg(const string & arg)
    {
        assert(!arg.empty());

        switch(arg[0])
        {
        case '[':
            return parse_eval_stream_arg(arg);
        //case '[':
            //return parse_range_arg(arg);
        default:
            return parse_eval_scalar_arg(arg);
        }
    }

    sp<semantic::type> parse_eval_stream_arg(const string & arg)
    {
        if (arg.size() < 3)
            throw std::runtime_error("Misformatted stream argument.");

        if (arg.front() != '[' || arg.back() != ']')
            throw std::runtime_error("Misformatted stream argument.");

        string size_list = arg.substr(1, arg.size()-2);

        vector<int> sizes;
        istringstream size_stream(size_list);
        string elem;
        while(std::getline(size_stream, elem, ','))
        {
            try {
                size_t pos = 0;
                int size = stoi(elem, &pos);
                if (pos == elem.size())
                {
                    sizes.push_back(size);
                    continue;
                }
            } catch (...) {}

            throw std::runtime_error("Misformatted stream argument.");
        }

        return sp<semantic::type>( new semantic::stream(sizes) );
    }

    sp<semantic::type> parse_eval_scalar_arg(const string & arg)
    {
        try {
            size_t pos = 0;
            int i = stoi(arg, &pos);
            if (pos == arg.size())
                return sp<semantic::type>( new semantic::integer_num(i) );
        }
        catch (...) {}

        try {
            size_t pos = 0;
            double d = stod(arg, &pos);
            if (pos == arg.size())
                return sp<semantic::type>( new semantic::real_num(d) );
        }
        catch (...) {}

        throw std::runtime_error("Misformatted scalar argument.");
    }

    sp<semantic::type> parse_eval_range_arg(const string & arg)
    {
        throw std::runtime_error("Unsupported range argument.");
    }

    int m_arg_count;
    char **m_args;
};

int main(int argc, char *argv[])
{
    string input_filename;
    string function_name;

    if (argc < 2)
    {
        cerr << "Missing argument: input filename." << endl;
        return 1;
    }
    input_filename = argv[1];

    argument_parser args(argc-2, argv+2);

    try {
        args.parse();
    }
    catch (argument_parser::abortion &)
    {
        return 0;
    }
    catch (exception & e)
    {
        cerr << "ERROR: Command line: " << e.what() << endl;
        return 1;
    }

    ifstream input_file(input_filename);

    stream::Parser parser(input_file);
    parser.setPrintsTokens(args.print_tokens);

    if (args.print_tokens)
        cout << "== Tokens ==" << endl;

    int success = parser.parse();
    if (success != 0)
        return success;

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
        return 1;

    if (args.print_symbols)
    {
        cout << endl;
        cout << "== Environment ==" << endl;
        cout << env;
    }

    semantic::type_checker type_checker(env);

    llvm::Module *module = new llvm::Module(input_filename, llvm::getGlobalContext());
    IR::generator gen(module, env);

    for (const evaluation & eval : args.evaluations)
    {
        cout << endl;
        cout << "== Evaluating: " << eval.name;
        cout << "(";
        if (eval.args.size())
            cout << *eval.args.front();
        for ( int i = 1; i < eval.args.size(); ++i )
        {
            cout << ", ";
            cout << *eval.args[i];
        }
        cout << ")" << endl;

        const auto & sym_iter = env.find(eval.name);
        if (sym_iter == env.end())
        {
            cerr << "WARNING: no symbol '" << eval.name << "' available." << endl;
            continue;
        }

        semantic::type_ptr result_type =
                type_checker.check(sym_iter->second, eval.args);

        if (type_checker.has_error())
            continue;

        if (result_type)
            cout << "Result type = " << *result_type << endl;
        else
            cout << "Result type = void" << endl;

        gen.generate(sym_iter->second, result_type, eval.args);
    }

    if (type_checker.has_error())
        return 1;

#if 0
    llvm::Module *module = new llvm::Module(input_filename, llvm::getGlobalContext());
    IR::generator gen(module, env);

    for (const evaluation & eval : args.evaluations)
    {
        const auto & symbol_iter = env.find(eval.name);
        assert(symbol_iter != env.end());

        gen.generate(symbol_iter->second, eval.args);
    }
#endif
    cout << endl;
    cout << "== LLVM IR:" << endl;
    module->dump();
}
