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

#ifndef STREAM_LANG_CPP_GEN_FROM_POLYHEDRAL_INCLUDED
#define STREAM_LANG_CPP_GEN_FROM_POLYHEDRAL_INCLUDED

#include "cpp_target.hpp"
#include "../utility/cpp-gen.hpp"
#include "../common/polyhedral_model.hpp"

#include <vector>
#include <unordered_map>
#include <string>

namespace stream {
namespace cpp_gen {

using std::vector;
using std::unordered_map;
using std::string;

class cpp_from_polyhedral
{
public:
    typedef vector<expression_ptr> index_type;

    cpp_from_polyhedral(const vector<polyhedral::statement*> &,
                        const unordered_map<string,buffer> & buffers);

    void generate_statement(const string & name,
                            const index_type & index,
                            builder*);

    void generate_statement(polyhedral::statement *,
                            const index_type & index,
                            builder*);

    void set_in_period(bool flag) { m_in_period = flag; }

private:

    expression_ptr generate_expression
    (polyhedral::expression*, const index_type&, builder*);

    expression_ptr generate_primitive
    (polyhedral::primitive_expr*, const index_type&, builder*);

    void generate_input_access
    (polyhedral::statement*, const index_type&, builder*);

    void generate_output_access
    (polyhedral::statement*, const index_type&, builder*);

    expression_ptr generate_buffer_access
    (polyhedral::array_ptr, const index_type&, builder*);

    index_type mapped_index( const index_type & index,
                             const polyhedral::mapping &,
                             builder * );

    expression_ptr literal(bool v)
    {
        return std::make_shared<literal_expression<bool>>(v);
    }
    expression_ptr literal(int v)
    {
        return std::make_shared<literal_expression<int>>(v);
    }
    expression_ptr literal(long v)
    {
        return std::make_shared<literal_expression<long>>(v);
    }
    expression_ptr literal(double v)
    {
        return std::make_shared<literal_expression<double>>(v);
    }

    vector<polyhedral::statement*> m_statements;
    unordered_map<string,buffer> m_buffers;
    bool m_in_period = false;
};

}
}

#endif // STREAM_LANG_CPP_GEN_FROM_POLYHEDRAL_INCLUDED
