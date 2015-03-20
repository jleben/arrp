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

#include "environment.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

environment::environment()
{}

std::ostream & operator<<(std::ostream & s, const environment & env)
{
    for (const auto & mapping : env)
    {
        const string & name = mapping.first;
        const semantic::symbol &sym = mapping.second;
        s << name;
        if (!sym.parameter_names.empty())
        {
            s << "(";
            int i = 0;
            for(const string & param : sym.parameter_names)
            {
                ++i;
                s << param;
                if (i < sym.parameter_names.size())
                    s << ", ";
            }
            s << ")";
        }
        s << std::endl;
    }
    return s;
}

}
}
