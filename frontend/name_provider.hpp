#ifndef STREAM_LANG_FUNCTIONAL_NAME_PROVIDER_INCLUDED
#define STREAM_LANG_FUNCTIONAL_NAME_PROVIDER_INCLUDED

#include <string>
#include <unordered_map>
#include <sstream>

namespace stream {
namespace functional {

using std::string;
using std::unordered_map;

class name_provider
{
public:
    name_provider(char separator): m_separator(separator) {}

    string new_name(const string & name)
    {
        string base;

        int sep_pos = name.rfind(m_separator);
        if (sep_pos == string::npos)
            base = name;
        else
            base = name.substr(0, sep_pos);

        int & count = m_name_counts[base];
        ++count;

        std::ostringstream text;
        text << base << m_separator << count;
        return text.str();
    }

private:
    char m_separator;
    unordered_map<string,int> m_name_counts;
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_NAME_PROVIDER_INCLUDED
