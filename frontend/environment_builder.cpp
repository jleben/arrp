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

#include "environment_builder.hpp"
#include "error.hpp"

#include <iostream>

using namespace std;

namespace stream {
namespace semantic {

class name_already_in_scope_error : public source_error
{
public:
    name_already_in_scope_error(const string & name,
                                const location_type & loc):
        source_error(msg(name),loc)
    {}
private:
    string msg(const string & name)
    {
        ostringstream text;
        text << "Duplicate name in scope: '" << name << "'";
        return text.str();
    }
};

class name_not_in_scope_error : public source_error
{
public:
    name_not_in_scope_error(const string & name,
                            const location_type & loc):
        source_error(msg(name),loc)
    {}
private:
    string msg(const string & name)
    {
        ostringstream text;
        text << "Name not in scope: '" << name << "'";
        return text.str();
    }
};

environment_builder::environment_builder(parsing::driver & driver, environment &env):
    m_driver(driver),
    m_env(env),
    m_has_error(false)
{
    m_ctx.enter_scope();

    vector<string> builtin_func_names = {
        "log",
        "log2",
        "log10",
        "exp",
        "exp2",
        "pow",
        "sqrt",
        "sin",
        "cos",
        "tan",
        "asin",
        "acos",
        "atan",
        "ceil",
        "floor",
        "abs",
        "min",
        "max"
    };

    for ( const auto & name : builtin_func_names )
    {
        m_ctx.bind(name, dummy());
    }
}

bool environment_builder::process( const ast::node_ptr & source )
{
    //m_ctx.enter_scope();

    process_stmt_list(source);

    //m_ctx.exit_scope();

    return !m_has_error;
}

void environment_builder::process_stmt_list( const ast::node_ptr & root )
{
    assert(root->type == ast::statement_list || root->type == ast::program);

    ast::list_node *stmts = root->as_list();

    for ( const sp<ast::node> & stmt : stmts->elements )
    {
        try {
            process_stmt(stmt);
        } catch( source_error & e ) {
            report(e);
        }
    }
}

void environment_builder::process_stmt( const sp<ast::node> & root )
{
    assert(root->type == ast::statement);
    ast::list_node *stmt = root->as_list();

    assert(stmt->elements.size() == 3);

    assert(stmt->elements[0]->type == ast::identifier);
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & body_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    m_ctx.enter_scope();

    bool param_name_error = false;
    vector<string> parameters;

    if (params_node)
    {
        assert(params_node->type == ast::id_list);
        ast::list_node *param_list = params_node->as_list();
        for ( const sp<ast::node> & param : param_list->elements )
        {
            assert(param->type == ast::identifier);
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);

            try
            {
                m_ctx.bind(param_name, dummy());
            }
            catch (context_error &)
            {
                report( name_already_in_scope_error(param_name, param->location) );
                param_name_error = true;
            }
        }
    }

    if (!param_name_error)
    {
        try {
            process_block(body_node);
        } catch ( source_error & e ) {
            report(e);
        }
    }

    m_ctx.exit_scope();

    if (m_ctx.level() > 1)
    {
        try
        {
            m_ctx.bind(id, dummy());
        }
        catch (context_error&)
        {
            throw name_already_in_scope_error(id, root->location);
        }
    }
    else
    {
        symbol::symbol_type sym_type = parameters.empty() ? symbol::expression : symbol::function;
        symbol sym(sym_type, id, root);
        sym.parameter_names = parameters;
        bool success = m_env.emplace(id, sym).second;
        if (!success)
            throw name_already_in_scope_error(id, root->location);
    }
}

void environment_builder::process_block( const sp<ast::node> & root )
{
    assert(root->type == ast::expression_block);
    ast::list_node *expr_block = root->as_list();

    assert(expr_block->elements.size() == 2);
    const auto stmt_list = expr_block->elements[0];
    const auto expr = expr_block->elements[1];

    if (stmt_list)
        process_stmt_list(stmt_list);

    process_expr(expr);
}

void environment_builder::process_expr( const sp<ast::node> & root )
{
    switch(root->type)
    {
    case ast::integer_num:
    case ast::real_num:
    case ast::boolean:
        return;
    case ast::identifier:
    {
        string id = root->as_leaf<string>()->value;
        if (!is_bound(id))
        {
            ostringstream msg;
            msg << "Name not in scope: '" << id << "'";
            throw source_error(msg.str(), root->location);
        }
        return;
    }
    case ast::oppose:
    case ast::negate:
    {
        ast::list_node * list = root->as_list();
        process_expr(list->elements[0]);
        return;
    }
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::divide_integer:
    case ast::modulo:
    case ast::raise:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
    case ast::logic_and:
    case ast::logic_or:
    {
        ast::list_node * list = root->as_list();
        process_expr(list->elements[0]);
        process_expr(list->elements[1]);
        return;
    }
    case ast::range:
    {
        ast::list_node * list = root->as_list();
        const auto &start = list->elements[0];
        const auto &end = list->elements[1];
        if (start)
            process_expr(start);
        if (end)
            process_expr(end);
        return;
    }
    case ast::hash_expression:
    {
        ast::list_node * list = root->as_list();
        const auto & object = list->elements[0];
        const auto & dim = list->elements[1];
        process_expr(object);
        if (dim)
            process_expr(dim);
        return;
    }
    case ast::call_expression:
    {
        ast::list_node * list = root->as_list();
        const auto & object = list->elements[0];
        const auto & args = list->elements[1];
        process_expr(object);
        ast::list_node *arg_list = args->as_list();
        for (const auto & arg : arg_list->elements)
            process_expr(arg);
        return;
    }
    case ast::if_expression:
    {
        process_expr(root->as_list()->elements[0]);
        process_block(root->as_list()->elements[1]);
        process_block(root->as_list()->elements[2]);
        return;
    }
    case ast::array_function:
    {
        auto var_list = root->as_list()->elements[0]->as_list();
        auto expr = root->as_list()->elements[1];
        for (auto var : var_list->elements)
        {
            auto size_expr = var->as_list()->elements[1];
            if (size_expr)
                process_expr(size_expr);
        }
        context_type::scope_holder scope(m_ctx);
        for (auto var : var_list->elements)
        {
            auto id_node = var->as_list()->elements[0]->as_leaf<string>();
            auto id = id_node->value;
            try { m_ctx.bind(id, dummy()); }
            catch (context_error &) {
                // FIXME: line number;
                throw name_already_in_scope_error(id, id_node->location);
            }
        }
        process_expr(expr);
        return;
    }
    case ast::array_application:
    {
        auto array_expr = root->as_list()->elements[0];
        auto arg_list = root->as_list()->elements[1]->as_list();
        process_expr(array_expr);
        for(auto & arg : arg_list->elements)
        {
            process_expr(arg);
        }
        return;
    }
    default:
        assert(false);
        throw source_error("Unsupported expression.", root->location);
    }
}

}
}
