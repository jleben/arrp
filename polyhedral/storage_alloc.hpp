/*
Compiler for language for stream processing

Copyright (C) 2016  Jakob Leben <jakob.leben@gmail.com>

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

#ifndef STREAM_LANG_POLYHEDRAL_STORAGE_ALLOC_INCLUDED
#define STREAM_LANG_POLYHEDRAL_STORAGE_ALLOC_INCLUDED

#include "../common/ph_model.hpp"
#include "../utility/debug.hpp"

#include <isl-cpp/printer.hpp>

namespace stream {
namespace polyhedral {

class storage_allocator
{
public:
    storage_allocator( model & );

    void allocate(const schedule &);

private:

    void compute_buffer_size
    ( const schedule &,
      const array_ptr & array );

    void find_inter_period_dependency
    ( const schedule &,
      const array_ptr & );

    model & m_model;
    model_summary m_model_summary;
    isl::printer m_printer;
};

struct storage_output {};

}
}

#endif // STREAM_LANG_POLYHEDRAL_STORAGE_ALLOC_INCLUDED
