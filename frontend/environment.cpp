#include "environment.hpp"
#include "ast.hpp"
#include "types.hpp"
#include "semantic.hpp"

#include <iostream>

using namespace std;

namespace stream {
namespace semantic {

environment::environment()
{
    enter_scope();

    // Add built-in functions

    bind( new elemwise_func_symbol("log") );
    bind( new elemwise_func_symbol("sqrt") );
    bind( new elemwise_func_symbol("exp") );
    bind( new elemwise_func_symbol("sin") );
    bind( new elemwise_func_symbol("cos") );
    bind( new elemwise_func_symbol("tan") );
    bind( new elemwise_func_symbol("asin") );
    bind( new elemwise_func_symbol("acos") );
    bind( new elemwise_func_symbol("atan") );
    bind( new elemwise_func_symbol("ceil") );
    bind( new elemwise_func_symbol("floor") );
    bind( new elemwise_func_symbol("abs") );
}

environment top_environment( ast::node * root, bool print_symbols )
{
    using ast::list_node;
    using ast::leaf_node;
    using ast::node;

    if (root->type != ast::program)
        throw std::runtime_error("Root node is not a program.");

    environment env;

    list_node *statements = root->as_list();
    for ( const sp<node> & stmt : statements->elements )
    {
        try
        {
            symbol *sym = evaluate_statement(env, stmt);
            if (print_symbols)
                cout << "[line " << stmt->line << "] " << *sym << endl;
        }
        catch( semantic_error & e )
        {
            e.report();
        }
    }

    return std::move(env);
}

sp<type> constant_symbol::evaluate( environment & env, const vector<sp<type>> & args )
{
    if (!args.empty())
    {
        throw wrong_arg_count(name(), 0, args.size() );
    }

    if (!m_value)
        m_value = evaluate_expr_block(env, m_code);

    return m_value;
}

sp<type> function_symbol::evaluate( environment & env, const vector<sp<type>> & args )
{
    if (m_parameters.size() != args.size())
    {
        throw wrong_arg_count(name(), m_parameters.size(), args.size() );
    }

    sp<type> result;

    env.enter_scope();

    for (int i = 0; i < m_parameters.size(); ++i)
    {
        env.bind(m_parameters[i], args[i]);
    }

    result = evaluate_expr_block(env, m_code);

    env.exit_scope();

    return result;
}

sp<type> elemwise_func_symbol::evaluate( environment & env,
                                         const vector<sp<type>> & args )
{
    if (args.size() != 1)
        throw wrong_arg_count(name(), 1, args.size());

    const sp<type> & t = args[0];

    switch(t->get_tag())
    {
    case type::integer_num:
        return sp<type>(new integer_num);
    case type::real_num:
        return sp<type>(new real_num);
    case type::range:
        return sp<type>(new range);
    case type::stream:
        return t;
    default:
        throw call_error(name(), "Invalid argument type.");
    }
}

}
}
