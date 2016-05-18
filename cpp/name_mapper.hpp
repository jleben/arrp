#ifndef STREAM_LANG_CPP_NAME_MAPPER_INCLUDED
#define STREAM_LANG_CPP_NAME_MAPPER_INCLUDED

#include <string>
#include <unordered_map>

namespace stream {
namespace cpp_gen {

using std::string;
using std::unordered_map;

class name_mapper
{
public:
    string operator()(const string &);
private:
    unordered_map<string,string> m_name_map;
    unordered_map<string,int> m_name_counter;
};

}
}

#endif // STREAM_LANG_CPP_NAME_MAPPER_INCLUDED
