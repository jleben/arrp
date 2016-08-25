#include "arg_parser.hpp"

using namespace std;

namespace stream {
namespace compiler {

arguments::arguments(int argc, char *argv[]):
    m_arg_count(argc),
    m_args(argv)
{}

void arguments::print_help() const
{
    cout << "Usage: arrp "
         << m_default_option_desc.arg_names
         << " [<option> [<arg> ...] ...]" << endl;
    cout << "Options:" << endl;
    for(auto & opt : m_options)
    {
        auto & desc = opt.first;
        auto parser = opt.second;
        cout << " ";
        if (!desc.long_name.empty())
        {
            cout << " --" << desc.long_name;
            if (!desc.arg_names.empty())
                cout << " " << desc.arg_names;
        }
        if (!desc.short_name.empty())
        {
            if (!desc.long_name.empty())
                cout << ",";
            cout << " -" << desc.short_name;
            if (!desc.arg_names.empty())
                cout << " " << desc.arg_names;
        }
        cout << " : " << desc.description << endl;
        cout << parser->description();
    }
}

}
}
