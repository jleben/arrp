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

#ifndef STREAM_LANG_FRONTEND_ERROR_INCLUDED
#define STREAM_LANG_FRONTEND_ERROR_INCLUDED

#include "../common/error.hpp"
#include "../common/module.hpp"
#include "location.hh"

namespace stream {

struct source_error : public error
{
    using location_type = code_location;

    source_error(const string & what, const location_type & location):
        error(what),
        location(location)
    {}

    location_type location;
};

}

#endif // STREAM_LANG_FRONTEND_ERROR_INCLUDED
