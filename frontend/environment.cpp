#include "environment.hpp"
#include "error.hpp"

#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

class name_already_in_scope_error : public source_error
{
public:
    name_already_in_scope_error(const string & name, int line):
        source_error(msg(name),line)
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
    name_not_in_scope_error(const string & name, int line):
        source_error(msg(name),line)
    {}
private:
    string msg(const string & name)
    {
        ostringstream text;
        text << "Name not in scope: '" << name << "'";
        return text.str();
    }
};

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

environment_builder::environment_builder(environment &env):
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
                report( name_already_in_scope_error(param_name, param->line) );
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
            throw name_already_in_scope_error(id, root->line);
        }
    }
    else
    {
        symbol::symbol_type sym_type = parameters.empty() ? symbol::expression : symbol::function;
        symbol sym(sym_type, id, root);
        sym.parameter_names = parameters;
        bool success = m_env.emplace(id, sym).second;
        if (!success)
            throw name_already_in_scope_error(id, root->line);
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
        return;
    case ast::identifier:
    {
        string id = root->as_leaf<string>()->value;
        if (!is_bound(id))
        {
            ostringstream msg;
            msg << "Name not in scope: '" << id << "'";
            throw source_error(msg.str(), root->line);
        }
        return;
    }
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
    case ast::raise:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
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
    case ast::transpose_expression:
    {
        ast::list_node * list = root->as_list();
        const auto & object = list->elements[0];
        process_expr(object);
        return;
    }
    case ast::slice_expression:
    {
        ast::list_node * list = root->as_list();
        const auto & object = list->elements[0];
        const auto & ranges = list->elements[1];
        process_expr(object);
        ast::list_node *range_list = ranges->as_list();
        for (const auto & range : range_list->elements)
            process_expr(range);
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
    case ast::for_expression:
    {
        ast::list_node * list = root->as_list();
        ast::list_node * domain_list = list->elements[0]->as_list();
        vector<string> ids;
        for (const auto & domain : domain_list->elements )
        {
            ast::list_node *domain_elems = domain->as_list();
            const auto & id = domain_elems->elements[0];
            const auto & size = domain_elems->elements[1];
            const auto & hop = domain_elems->elements[2];
            const auto & source = domain_elems->elements[3];
            if (id)
                ids.push_back(id->as_leaf<string>()->value);
            if (size)
                process_expr(size);
            if (hop)
                process_expr(hop);
            process_expr(source);
        }
        context_type::scope_holder iteration_scope(m_ctx);
        for (const auto & id : ids )
        {
            try { m_ctx.bind(id, dummy()); }
            catch (context_error &) {
                // FIXME: line number;
                throw name_already_in_scope_error(id, 0);
            }
        }
        process_block(list->elements[1]);
        return;
    }
    case ast::reduce_expression:
    {
        ast::list_node * list = root->as_list();
        const string & id1 = list->elements[0]->as_leaf<string>()->value;
        const string & id2 = list->elements[1]->as_leaf<string>()->value;
        const auto & domain = list->elements[2];
        process_expr(domain);
        context_type::scope_holder iteration_scope(m_ctx);
        try { m_ctx.bind(id1, dummy()); }
        catch (context_error &) {
            // FIXME: line number;
            throw name_already_in_scope_error(id1, 0);
        }
        try { m_ctx.bind(id2, dummy()); }
        catch (context_error &) {
            // FIXME: line number;
            throw name_already_in_scope_error(id2, 0);
        }
        process_block(list->elements[3]);
        return;
    }
    default:
        assert(false);
        throw source_error("Unsupported expression.", root->line);
    }
}

}
}
