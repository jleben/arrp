/*
Compiler for language for language Arrp

Copyright (C) 2014-2016  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef ARRP_COMPILER_OPTIONS
#define ARRP_COMPILER_OPTIONS

#include "../polyhedral/scheduling.hpp"

namespace stream {
namespace compiler {

struct options
{
    string input_filename;
    //string output_filename;
    //string meta_output_filename;

    struct {
        bool enabled;
        string nmspace;
        string filename;
    } cpp;

    vector<string> import_dirs;
    vector<polyhedral::scheduler::reversal> sched_reverse;
    bool optimize_schedule = true;
    bool schedule_whole = false;
    bool split_statements = false;
    bool separate_loops = false;
};

}
}

#endif // ARRP_COMPILER_OPTIONS
