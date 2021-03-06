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

#include <string>
#include <vector>

namespace stream {
namespace compiler {

using std::string;
using std::vector;

struct options
{
    string input_filename;
    //string output_filename;
    //string meta_output_filename;

    string output_filename_base;
    string interface_type = "cpp";

    struct {
        string nmspace;
    } cpp;

    struct {
    } generic_io;

    struct {
        string name;
    } jack_io;

    struct {
        string name;
    } puredata_io;

    vector<string> import_dirs;
    vector<string> import_extensions { "arrp" };

    struct {
      bool cluster = true;
      vector<int> tile_size;
      bool tile_parallelism = false;
      vector<int> intra_tile_permutation;
      vector<int> periodic_tile_direction;
      int period_offset = 0;
      int period_scale = 1;
    } schedule;

    bool split_statements = false;
    bool separate_loops = false;

    bool atomic_io = false;
    bool ordered_io = true;
    // clocked_io: all IO channels transfer a sample
    // before any of them transfers the next sample.
    bool clocked_io = false;

    bool parallel = false;
    int parallel_dim = -1;
    bool vectorize = false;

    bool classic_storage_allocation = false;
    bool buffer_data_shifting = false;
    bool loop_invariant_code_motion = false;

    int data_alignment = 0;
    bool data_size_power_of_two = true;

    string report_file;
};

}
}

#endif // ARRP_COMPILER_OPTIONS
