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

#ifndef STREAM_CPP_GEN_FROM_CLOOG_INCLUDED
#define STREAM_CPP_GEN_FROM_CLOOG_INCLUDED

#include "../utility/context.hpp"
#include "../utility/cpp-gen.hpp"

#include <cloog/cloog.h>

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>

namespace stream {
namespace cpp_gen {

using std::unordered_map;
using std::string;
using std::vector;

class cpp_from_cloog
{
    int id_count;

public:
    cpp_from_cloog(builder *ctx);

    void generate( clast_stmt *ast );

    template<typename F>
    void set_stmt_func(F f)
    {
        m_stmt_func = f;
    }

private:
    void process_list( clast_stmt *, vector<statement_ptr> & );
    statement_ptr process( clast_stmt * );
    statement_ptr process( clast_root* );
    statement_ptr process( clast_block* );
    statement_ptr process( clast_assignment* );
    statement_ptr process( clast_guard* );
    statement_ptr process( clast_for* );
    statement_ptr process( clast_user_stmt* );
    expression_ptr process( clast_expr* );
    expression_ptr process( clast_equation* );

    vector<expression_ptr> vec( std::initializer_list<expression_ptr> l )
    {
        return vector<expression_ptr>(l);
    }

    expression_ptr literal(bool v)
    {
        return std::make_shared<literal_expression<bool>>(v);
    }

    expression_ptr literal(long v)
    {
        return std::make_shared<literal_expression<long>>(v);
    }

#ifdef CLOOG_INT_GMP
    expression_ptr literal( mpz_t v )
    {
        return literal(mpz_get_si(v));
    }
#endif

    std::function<void(const string &,
                       const vector<expression_ptr> &,
                       builder *)> m_stmt_func;

private:
    builder *m_ctx;
};

}
}

#endif // STREAM_CPP_GEN_FROM_CLOOG_INCLUDED
