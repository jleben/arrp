/*
Compiler for language for language Arrp

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

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

#include <sstream>
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>
#include <functional>
#include <utility>

namespace stream {
namespace compiler {

using std::pair;
using std::string;
using std::vector;
using std::unordered_map;
using std::ostringstream;
using std::istringstream;
using std::ostream;
using std::function;

class arguments;

struct target_info
{
    string name;
};

struct option_description
{
    string long_name;
    string short_name;
    string arg_names;
    string description;
};

struct option_parser
{
    virtual ~option_parser() {}
    virtual void process(arguments &) = 0;
    virtual string description() const { return string(); }
};

struct adhoc_option_parser : option_parser
{
    typedef function<void(arguments&)> handler_t;
    handler_t handler;
    adhoc_option_parser(handler_t h): handler(h) {}
    void process(arguments & args) { handler(args); }
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
            text << msg;
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

    int m_arg_count = 0;
    char **m_args = nullptr;

    string m_current_opt;
    option_description m_default_option_desc;
    option_parser * m_default_option = nullptr;
    vector<pair<option_description,option_parser*>> m_options;

public:
    arguments();

    void set_default_option(const option_description & desc, option_parser * parser)
    {
        m_default_option_desc = desc;
        m_default_option = parser;
    }

    void set_default_option(const option_description & desc, adhoc_option_parser::handler_t handler)
    {
        m_default_option_desc = desc;
        m_default_option = new adhoc_option_parser(handler);
    }

    void add_option(const option_description & desc, adhoc_option_parser::handler_t handler)
    {
        m_options.emplace_back(desc, new adhoc_option_parser(handler));
    }

    void add_option(const option_description & desc, option_parser * parser)
    {
        m_options.emplace_back(desc, parser);
    }

    void parse(int argc, char *argv[])
    {
        m_arg_count = argc;
        m_args = argv;

        if (m_default_option)
            m_default_option->process(*this);

        while(arg_count())
        {
            parse_next_option();
        }
    }

    void print_help() const;

private:

    void parse_next_option()
    {
        string opt_name;

        parse_option(opt_name);

        for(auto & opt : m_options)
        {
            using namespace std;
            auto & desc = opt.first;
            auto handler = opt.second;
            if ( opt_name == "--" + desc.long_name ||
                 opt_name == "-" + desc.short_name )
            {
                handler->process(*this);
                return;
            }
        }
        {
            ostringstream msg;
            msg << "Invalid option: " << opt_name;
            throw error(msg.str());
        }
    }

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

public:
    string parse_raw_argument(const string & what)
    {
        if (arg_count())
        {
            string arg = current_arg();
            advance();
            return arg;
        }
        else
        {
            throw missing_argument(what, m_current_opt);
        }
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


struct switch_option : public option_parser
{
    bool * value;
    bool enable;
    switch_option(bool * v, bool e = true): value(v), enable(e) {}
    void process(arguments &) { *value = enable; }
};

struct string_option : public option_parser
{
    string * value;
    string description;

    string_option(string * v, const string & d = string()):
        value(v), description(d) {}
    void process(arguments & args) { args.parse_argument(*value, description); }
};

struct string_list_option : public option_parser
{
    vector<string> * values;
    string description;

    string_list_option(vector<string>*v, const string & d = string()):
        values(v), description(d) {}

    void process(arguments & args)
    {
        string value;
        args.parse_argument(value, description);
        values->push_back(value);
    }
};

struct int_option : public option_parser
{
    int * value;

    int_option(int * v): value(v) {}

    void process(arguments & args) {
        string text;
        args.parse_argument(text, "");
        *value = std::stoi(text);
    }
};

}
}

#endif // STREAM_LANG_COMPILER_ARG_PARSER
