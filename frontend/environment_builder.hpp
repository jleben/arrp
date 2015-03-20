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

#ifndef STREAM_LANG_ENVIRONMENT_BUILDER_INCLUDED
#define STREAM_LANG_ENVIRONMENT_BUILDER_INCLUDED

#include "../common/environment.hpp"
#include "../utility/context.hpp"

namespace stream {
namespace semantic {

class environment_builder
{
public:
    environment_builder(environment &env);

    bool process( const ast::node_ptr & source );

private:
    struct dummy {};
    typedef context<string,dummy> context_type;

    void process_stmt_list( const ast::node_ptr & );
    void process_stmt( const sp<ast::node> & );
    void process_block( const sp<ast::node> & );
    void process_expr( const sp<ast::node> & );
    environment & m_env;
    context<string,dummy> m_ctx;

    bool can_bind( const string & name )
    {
        if (m_ctx.level() > 0)
            return !m_ctx.current_scope_has_bound(name);
        else
            return m_env.find(name) == m_env.end();
    }

    bool is_bound( const string & name )
    {
        bool bound = m_ctx.has_bound(name);
        if (!bound)
            bound = m_env.find(name) != m_env.end();
        return bound;
    }

    void report( const std::exception & e )
    {
        m_has_error = true;
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    bool m_has_error;
};

}
}

#endif // STREAM_LANG_ENVIRONMENT_BUILDER_INCLUDED
