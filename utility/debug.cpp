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
