#include "parser.h"
#include "ast_printer.hpp"
#include "type_checker.hpp"
#include "ir-generator.hpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace stream;

void print_help()
{
    cout << "Usage:" << endl
         << "  frontend <input file> [<parameter>...]" << endl;

    cout << "Parameters:" << endl;

    cout << "  --output or -o : LLVM IR output file name (default: 'out.ll')." << endl;
    cout << "  --print-tokens or -t : Print all tokens produced by lexical scanner." << endl;
    cout << "  --print-ast or -s : Print abstract syntax tree (AST) produced by parser." << endl;
    cout << "  --list-symbols or -l : List all top-level declarations." << endl;
    cout << "  --generate or --gen or -g <symbol> [<type>...] :" << endl
         << "  \tGenerate output for top-level function or expression <symbol> with given argument types." << endl
         << "  \tEach following argument <type> is used as the type of a function parameter in "
         "generic function instantiation." << endl
         << "  \tA <type> can be one of the following:" << endl
         << "  \t- \"int\" - integer number," << endl
         << "  \t- \"real\" - real number," << endl
         << "  \t- a stream, e.g. \"[10,4,5]\""
         << " - each number represents size in one dimension." << endl
         ;
}

struct command_line_error
{
public:
    command_line_error(const string & what):
        m_what(msg(what))
    {}
    command_line_error(const char * what):
        m_what(msg(what))
    {}
    const char * what() const
    {
        return m_what.c_str();
    }
private:
    static string msg(const string & what)
    {
        ostringstream text;
        text << "ERROR: Command line: ";
        text << what;
        return text.str();
    }
    string m_what;
};

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
    string input_filename;
    string output_filename;
    vector<evaluation> evaluations;

    argument_parser(int argc, char *argv[]):
        m_arg_count(argc),
        m_args(argv),
        output_filename("out.ll")
    {}

    void parse()
    {
        if (m_arg_count && current_arg()[0] != '-')
        {
            input_filename = current_arg();
            advance();
        }

        while(m_arg_count)
            parse_next_option();
    }

private:

    void advance()
    {
        --m_arg_count;
        ++m_args;
    }

    int arg_count() { return m_arg_count; }

    char *current_arg()
    {
        return m_args[0];
    }

    void parse_next_option()
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
        else if (arg == "--generate" || arg == "--gen" || arg == "-g")
        {
            advance();
            parse_evaluation(arg);
        }
        else if (arg == "--output" || arg == "-o")
        {
            advance();
            if (m_arg_count && current_arg()[0] != '-')
                output_filename = current_arg();
            else
            {
                ostringstream msg;
                msg << "Missing argument for option " << arg << ".";
                throw command_line_error(msg.str());
            }
            advance();
        }
        else
        {
            ostringstream msg;
            msg << "Invalid argument: " << arg;
            throw command_line_error(msg.str());
        }
    }

    void parse_evaluation(const string & parameter)
    {
        string name;

        if (arg_count() && current_arg()[0] != '-')
        {
            name = current_arg();
            advance();
        }
        else
        {
            ostringstream msg;
            msg << "Missing symbol name after " << parameter << " option.";
            throw command_line_error(msg.str());
        }

        evaluations.emplace_back();
        evaluation &eval = evaluations.back();
        eval.name = name;

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
            throw command_line_error("Invalid stream argument.");

        if (arg.front() != '[' || arg.back() != ']')
            throw command_line_error("Invalid stream argument.");

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

            throw command_line_error("Invalid stream argument.");
        }

        return sp<semantic::type>( new semantic::stream(sizes) );
    }

    sp<semantic::type> parse_eval_scalar_arg(const string & arg)
    {
        if (arg == "int")
            return make_shared<semantic::integer_num>();
        else if (arg == "real")
            return make_shared<semantic::real_num>();
        else
            throw command_line_error("Invalid scalar argument.");
    }

    sp<semantic::type> parse_eval_range_arg(const string & arg)
    {
        throw command_line_error("Unsupported range argument.");
    }

    int m_arg_count;
    char **m_args;
};

int main(int argc, char *argv[])
{
    argument_parser args(argc-1, argv+1);

    try {
        args.parse();
    }
    catch (argument_parser::abortion &)
    {
        return 0;
    }
    catch (command_line_error & e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    if (args.input_filename.empty())
    {
        cerr << "ERROR: Command line: Missing argument: input filename." << endl;
        return 1;
    }

    ifstream input_file(args.input_filename);
    if (!input_file.is_open())
    {
        cerr << "ERROR: Failed to open input file for reading: '"
             << args.input_filename << "'." << endl;
        return 1;
    }

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

    IR::generator gen(args.input_filename, env);

    for (const evaluation & eval : args.evaluations)
    {
        cout << endl;
        cout << "== Generating: " << eval.name;
        cout << "(";
        if (eval.args.size())
            cout << *eval.args.front();
        for ( int i = 1; i < eval.args.size(); ++i )
        {
            cout << ", ";
            cout << *eval.args[i];
        }
        cout << ")" << endl;

        auto sym_iter = env.find(eval.name);
        if (sym_iter == env.end())
        {
            cerr << "WARNING: no symbol '" << eval.name << "' available." << endl;
            continue;
        }

        semantic::type_ptr result_type =
                type_checker.check(sym_iter->second, eval.args);

        if (type_checker.has_error())
            continue;

        cout << "Type: " << *result_type << endl;

        {
            if(result_type->get_tag() == semantic::type::function)
            {
                string func_name = result_type->as<semantic::function>().name;
                sym_iter = env.find(func_name);
                if (sym_iter == env.end())
                {
                    throw error("Function just generated not found.");
                }
            }
        }


        gen.generate(sym_iter->second, eval.args);
    }

    if (type_checker.has_error())
        return 1;

#if 0
    cout << endl;
    cout << env;
    cout << endl;
    ast::printer printer;
    for (const auto & sym : env)
    {
        cout << "-- " << sym.first << ":" << endl;
        printer.print(sym.second.source.get());
        cout << endl;
    }
#endif

    if (args.evaluations.empty())
        return 0;

    ofstream output_file(args.output_filename);
    if (!output_file.is_open())
    {
        cerr << "ERROR: Could not open output file for writing!" << endl;
        return 1;
    }

    gen.output(output_file);

    bool generator_has_errors = gen.verify();
    if (generator_has_errors)
    {
        cerr << "ERROR: Bad IR code output." << endl;
        return 1;
    }

    return 0;
}
