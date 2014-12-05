#ifndef STREAM_LANG_COMPILER_ARG_PARSER
#define STREAM_LANG_COMPILER_ARG_PARSER

#include "../frontend/types.hpp"

#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

namespace stream {
namespace compiler {

using std::string;
using std::vector;
using std::ostringstream;
using std::istringstream;

struct target_info
{
    string name;
    vector<semantic::type_ptr> args;
};

void print_help()
{
    using namespace std;

    cout << "Usage:" << endl
         << "  streamc <input file> [<parameter>...]" << endl;

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

class arguments
{
public:
    class error
    {
    public:
        error() {}
        error(const string & msg)
        {
            set_msg(msg);
        }
        void set_msg(const string & msg)
        {
            std::ostringstream text;
            text << "streamc: error: " << msg;
            m_msg = text.str();
        }
        const string & msg() const { return m_msg; }
    private:
        string m_msg;
    };

    struct missing_argument : public error
    {
        missing_argument(const string & what, const string & opt = string())
        {
            std::ostringstream text;
            text << "Missing argument";
            if (!opt.empty())
                text << " for option " << opt;
            text << ": " << what << ".";
            set_msg(text.str());
        }
    };

    struct missing_option : public error
    {
        missing_option(const string & arg = string())
        {
            std::ostringstream text;
            text << "Expected an option";
            if (!arg.empty())
                text << ", got \"" << arg << "\" instead";
            text << '.';
            set_msg(text.str());
        }
    };

    struct abortion {};


    bool print_tokens = false;
    bool print_ast = false;
    bool print_symbols = false;
    bool print_polyhedral_model = false;
    string input_filename;
    string output_filename;
    string meta_output_filename;
    target_info target;

private:
    int m_arg_count;
    char **m_args;
    string m_current_opt;

public:
    arguments(int argc, char *argv[]):
        m_arg_count(argc),
        m_args(argv),
        output_filename("out.ll"),
        meta_output_filename("out.meta")
    {}

    void parse()
    {
        try_parse_argument(input_filename);

        while(arg_count())
        {
            parse_next_option();
        }
    }

private:
    void parse_next_option()
    {
        string opt;

        parse_option(opt);

        if (opt == "--help" || opt == "-h")
        {
            print_help();
            throw abortion();
        }
        else if (opt == "--print-tokens" || opt == "-t")
        {
            print_tokens = true;
        }
        else if (opt == "--print-ast" || opt == "-s")
        {
            print_ast = true;
        }
        else if (opt == "--print-poly")
        {
            print_polyhedral_model = true;
        }
        else if (opt == "--list-symbols" || opt == "-l")
        {
            print_symbols = true;
        }
        else if (opt == "--generate" || opt == "--gen" || opt == "-g")
        {
            target.name.clear();
            target.args.clear();
            parse_target(target, opt);
        }
        else if (opt == "--output" || opt == "-o")
        {
            parse_argument(output_filename, "output LLVM IR file");
        }
        else if (opt == "--meta" || opt == "-m")
        {
            parse_argument(meta_output_filename, "output meta-data file (JSON)");
        }
        else
        {
            ostringstream msg;
            msg << "Invalid option: " << opt;
            throw error(msg.str());
        }
    }

    void parse_target(target_info &tgt, const string & parameter)
    {
        parse_argument(tgt.name, "target name");

        string arg;
        while(try_parse_argument(arg))
        {
            tgt.args.push_back( parse_target_arg(arg) );
        }
    }

    semantic::type_ptr parse_target_arg(const string & arg)
    {
        assert(!arg.empty());

        switch(arg[0])
        {
        case '[':
            return parse_target_stream_arg(arg);
        //case '[':
            //return parse_range_arg(arg);
        default:
            return parse_target_scalar_arg(arg);
        }
    }

    semantic::type_ptr parse_target_stream_arg(const string & arg)
    {
        if (arg.size() < 3)
            throw error("Invalid stream argument: " + arg);

        if (arg.front() != '[' || arg.back() != ']')
            throw error("Invalid stream argument: " + arg);

        string size_list = arg.substr(1, arg.size()-2);

        vector<int> sizes;
        istringstream size_stream(size_list);
        string elem;
        while(std::getline(size_stream, elem, ','))
        {
            if (elem == "inf")
            {
                sizes.push_back(semantic::stream::infinite);
                continue;
            }
            else
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
            }

            throw error("Invalid stream argument.");
        }

        return semantic::type_ptr( new semantic::stream(sizes) );
    }

    semantic::type_ptr parse_target_scalar_arg(const string & arg)
    {
        if (arg == "int")
            return std::make_shared<semantic::integer_num>();
        else if (arg == "real")
            return std::make_shared<semantic::real_num>();
        else
            throw error("Invalid scalar argument: " + arg);
    }

    semantic::type_ptr parse_eval_range_arg(const string & arg)
    {
        throw error("Unsupported range argument: " + arg);
    }


    // General

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

    void parse_argument(string & arg, const string & what)
    {
        if (arg_count() && current_arg()[0] != '-')
        {
            arg = current_arg();
            advance();
        }
        else
        {
            throw missing_argument(what, m_current_opt);
        }
    }

    bool try_parse_argument(string & arg)
    {
        if (arg_count() && current_arg()[0] != '-')
        {
            arg = current_arg();
            advance();
            return true;
        }
        return false;
    }

    void parse_option(string & opt)
    {
        if (arg_count() && current_arg()[0] == '-')
        {
            opt = m_current_opt = current_arg();
            advance();
        }
        else
        {
            if (arg_count())
                throw missing_option(current_arg());
            else
                throw missing_option();
        }
    }

    void try_parse_option(string & opt)
    {
        if (arg_count() && current_arg()[0] == '-')
        {
            opt = current_arg();
            advance();
        }
    }
};

}
}

#endif // STREAM_LANG_COMPILER_ARG_PARSER
