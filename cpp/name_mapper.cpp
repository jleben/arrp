#include "name_mapper.hpp"

#include <locale>

using namespace std;

namespace stream {
namespace cpp_gen {

string name_mapper::operator()(const string & original_name)
{
    locale loc;

    string name = original_name;

    auto mapping = m_name_map.find(name);
    if (mapping != m_name_map.end())
        return mapping->second;

    for (auto & c : name)
    {
        if (!(isdigit(c, loc) || isalpha(c, loc) || c == '_'))
            c = '_';
    }

    int & count = ++m_name_counter[name];
    if (count > 1)
    {
        name += '_';
        name += to_string(count);
    }

    m_name_map[original_name] = name;

    return name;
}

}
}
