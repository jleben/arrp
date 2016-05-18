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

#ifndef STREAM_LANG_ERROR_INCLUDED
#define STREAM_LANG_ERROR_INCLUDED

#include <exception>
#include <sstream>
#include <string>

// FIXME: Enable on later versions of MS compiler:
#ifdef _MSC_VER
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

namespace stream {

using std::string;

struct abort_error {};

struct undefined {};

struct error : public std::exception
{
public:
    error() {}

    error(const string & what):
        m_msg(what)
    {}

    virtual const char *what() const NOEXCEPT
    {
        return m_msg.c_str();
    }

private:
    string m_msg;
};

inline void assert_or_throw(bool condition)
{
    if (!condition)
        throw error("Assertion failed.");
}

}

#endif // STREAM_LANG_ERROR_INCLUDED

