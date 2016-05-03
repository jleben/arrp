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

#ifndef STREAM_LANG_COMPILER_ARG_PARSER
#define STREAM_LANG_COMPILER_ARG_PARSER

#include "../common/types.hpp"
#include "../polyhedral/scheduling.hpp"
#include "../utility/debug.hpp"

#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

namespace stream {
namespace compiler {

using std::string;
using std::vector;
using std::unordered_map;
using std::ostringstream;
using std::istringstream;

class arguments;

void print_help(const arguments &);

struct target_info
{
    string name;
    vector<semantic::type_ptr> args;
};

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

private:
    int m_arg_count;
    char **m_args;
    string m_current_opt;
    unordered_map<string,bool*> m_verbose_topics;

public:

    string input_filename;
    string output_filename;
    string meta_output_filename;
    string cpp_output_filename;
    target_info target;
    vector<polyhedral::scheduler::reversal> sched_reverse;
    bool optimize_schedule = true;
    bool split_statements = false;

public:
    arguments(int argc, char *argv[]):
        m_arg_count(argc),
        m_args(argv),
        output_filename("out.ll")
    {}

    template <typename T>
    void add_verbose_topic(const string & name)
    {
        m_verbose_topics.emplace(name, &verbose<T>::enabled());
    }

    void parse()
    {
        try_parse_argument(input_filename);

        while(arg_count())
        {
            parse_next_option();
        }
    }

    vector<string> verbose_topics() const
    {
        vector<string> names;
        for (auto & pair : m_verbose_topics)
            names.push_back(pair.first);
        return names;
    }

private:

    void parse_next_option()
    {
        string opt;

        parse_option(opt);

        if (opt == "--help" || opt == "-h")
        {
            print_help(*this);
            throw abortion();
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
        else if (opt == "--cpp")
        {
            parse_argument(cpp_output_filename, "C++ interface output file");
        }
        else if (opt == "--verbose" || opt == "-v")
        {
            string topic;
            while(try_parse_argument(topic))
            {
                try { *m_verbose_topics.at(topic) = true; }
                catch ( std::out_of_range & e )
                {
                    throw error("Unknown verbose topic: " + topic);
                }
            }
        }
        else if (opt == "--no-opt-schedule")
        {
            optimize_schedule = false;
        }
        else if (opt == "--reverse")
        {
            string stmt_name;
            string dim_str;
            parse_argument(stmt_name, "statement name");
            parse_argument(dim_str, "schedule dimension");
            int dim;
            try {
                dim = stoi(dim_str);
            } catch (std::invalid_argument &) {
                throw error("Invalid schedule dimension: " + dim_str);
            }
            sched_reverse.push_back({stmt_name, dim});
        }
        else if (opt == "--split-stmts")
        {
            split_statements = true;
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

        // FIXME: support all element types
        return semantic::type_ptr( new semantic::stream(sizes, primitive_type::real) );
    }

    semantic::type_ptr parse_target_scalar_arg(const string & arg)
    {
        if (arg == "bool")
            return std::make_shared<semantic::boolean>();
        else if (arg == "int")
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
