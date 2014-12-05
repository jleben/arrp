#include "debug.hpp"

namespace stream {
namespace debug {

typedef std::unordered_map<string,status_value> status_map;

status_map & the_map()
{
    static status_map map;
    return map;
}

status_value status_for_id( const string & id,
                            status_value default_value )
{
    status_map::iterator it = the_map().find(id);
    if (it != the_map().end())
        return it->second;
    else
        return default_value;
}

void set_status_for_id( const string & id,
                        status_value value )
{
    the_map()[id] = value;
}

} // namespace debug
} // namespace stream
