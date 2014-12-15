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

#ifndef STREAM_LANG_DEBUG_INCLUDED
#define STREAM_LANG_DEBUG_INCLUDED

#include <unordered_map>
#include <string>

namespace stream {
namespace debug {

using std::string;

enum status_value
{
    neutral = 0,
    enabled,
    disabled
};

status_value status_for_id( const string & id,
                            status_value default_value );

void set_status_for_id( const string & id,
                        status_value value );

template<typename T>
struct status
{
    static status_value value()
    {
        return status_for_id(T::full_id(), T::default_status());
    }

    static void set_value( status_value value )
    {
        set_status_for_id(T::full_id(), value);
    }
};

struct all
{
    static status_value default_status() { return neutral; }

    static string full_id()
    {
        return "all";
    }

    static string combined_id_with( const string & id )
    {
        return id;
    }

    static status_value full_status()
    {
        return status<all>::value();
    }

    static bool is_enabled()
    {
#ifndef NDEBUG
        return full_status() == enabled;
#else
        return false;
#endif
    }

    static void enable()
    {
        status<all>::set_value(enabled);
    }

    static void disable()
    {
        status<all>::set_value(disabled);
    }

    static void neutralize()
    {
        status<all>::set_value(neutral);
    }
};

template<typename topic_type,
         typename supertopic_type,
         status_value default_status_value = neutral>
struct topic
{
    static status_value default_status() { return default_status_value; }

    static string combined_id_with( const string & id )
    {
        return full_id() + '.' + id;
    }

    static string full_id()
    {
        using namespace std;
        string id = supertopic_type::combined_id_with( topic_type::id() );
        return id;
    }

    static status_value full_status()
    {
        status_value subtopic_status = status<topic_type>::value();
        status_value supertopic_status =
                supertopic_type::full_status();

        switch(supertopic_status)
        {
        case neutral:
            return subtopic_status;
        case enabled:
            return (subtopic_status == disabled) ? disabled : enabled;
        case disabled:
            return disabled;
        }
    }

    static bool is_enabled()
    {
#ifndef NDEBUG
        return full_status() == enabled;
#else
        return false;
#endif
    }

    static void enable()
    {
        status<topic_type>::set_value(enabled);
    }

    static void disable()
    {
        status<topic_type>::set_value(disabled);
    }

    static void neutralize()
    {
        status<topic_type>::set_value(neutral);
    }
};

} // namespace debug
} // namespace stream

#endif // STREAM_LANG_DEBUG_INCLUDED
