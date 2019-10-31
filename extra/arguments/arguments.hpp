#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>
#include <stack>

namespace Arguments {

using std::string;
using std::istringstream;
using std::ostream;
using std::unordered_map;
using std::vector;
using std::function;

class Parser
{
    using Option_Parser = function<void(const vector<string> &)>;
    using Option_Printer = function<void(ostream&)>;

    struct Option
    {
        Option_Parser parser;
        Option_Printer printer;
        int param_count = 0;
    };

public:
    class Error : public std::exception
    {
        string msg;
    public:
        Error() {}
        Error(const string & msg): msg(msg) {}
        const char * what() const noexcept override
        {
            return msg.c_str();
        }
    };

    struct Invalid_Option_Param : public Error
    {
        Invalid_Option_Param(const string & o, const string & v):
            Error("Invalid value '" + v + "' for option " + o) {}
    };

    struct Option_Param_Error {};

    template <typename T>
    static
    void parse_value(const string & text, T & value)
    {
        istringstream text_stream(text);
        text_stream >> value;
        if (!text_stream || text_stream.tellg() < text.size())
            throw Option_Param_Error();
    }

    template <typename T>
    void add_option(const string & name, T & destination, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 1;

        option.parser = [name, &destination](const vector<string> & values)
        {
            auto & value = values[0];
            try {
                parse_value(value, destination);
            }
            catch (Option_Param_Error &) {
                throw Invalid_Option_Param(name, value);
            }
        };

        option.printer = [=](ostream & s)
        {
            s << name << " <" << typeid(destination).name() << ">";
            if (!description.empty())
            {
                s << " : " << description;
            }
        };
    }

    void add_option(const string & name, string & destination, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 1;

        option.parser = [name, &destination](const vector<string> & params)
        {
            destination = params[0];
        };

        option.printer = [=](ostream & s)
        {
            s << name << " <string>";
            if (!description.empty())
            {
                s << " : " << description;
            }
        };
    }

    void add_switch(const string & name, bool & destination, bool enable = true, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 0;

        option.parser = [name, enable, &destination](const vector<string> & params)
        {
            destination = enable;
        };

        option.printer = [=](ostream & s)
        {
            s << name;
            if (!description.empty())
            {
                s << " : " << description;
            }
        };
    }

    void remaining_argument(string & destination)
    {
        remaining_arg = &destination;
    }

    void remaining_arguments(vector<string> & destination)
    {
        remaining_args = &destination;
    }

    void parse(int argc, char ** argv)
    {
        std::stack<string> args;

        for(int i = argc-1; i > 0; --i)
            args.push(argv[i]);

        while(!args.empty())
        {
            string arg = args.top();
            args.pop();

            if(try_parse_option(arg, args))
                continue;

            ++remaining_arg_count;
            if (remaining_args)
            {
                remaining_args->push_back(arg);
            }
            else if (remaining_arg && remaining_arg_count == 1)
            {
                *remaining_arg = arg;
            }
            else
            {
                throw Error("Unexpected argument: " + arg);
            }
        }
    }

    void print(ostream & s)
    {
        for (auto & option : options)
        {
            if (option.second.printer)
            {
                option.second.printer(s);
                s << std::endl;
            }
        }
    }

private:
    bool try_parse_option(const string & arg, std::stack<string> & args)
    {
        string name;
        string value;

        bool has_equals = false;
        {
            auto equals_pos = arg.find('=');
            if (equals_pos == string::npos)
            {
                name = arg;
            }
            else
            {
                has_equals = true;
                name = arg.substr(0, equals_pos);
                value = arg.substr(equals_pos+1);
            }
        }

        if (!options.count(name))
            return false;

        auto & option = options.at(name);

        vector<string> params;

        if (has_equals)
        {
            if (!value.empty())
                params.push_back(value);
        }
        else
        {
            while(params.size() < option.param_count and !args.empty())
            {
                params.push_back(args.top());
                args.pop();
            }
        }

        if (params.size() < option.param_count)
        {
            std::ostringstream msg;
            msg << "Option " << name << " requires " << option.param_count
                << " parameters, but " << params.size() << " given.";
            throw Error(msg.str());
        }

        option.parser(params);

        return true;
    }

    unordered_map<string, Option> options;
    int remaining_arg_count = 0;
    string * remaining_arg = nullptr;
    vector<string> * remaining_args = nullptr;
};

}
