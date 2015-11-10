#include "functional_gen.hpp"
#include "error.hpp"

using namespace std;

namespace stream {
namespace functional {

vector<func_def_ptr>
generator::generate(ast::node_ptr ast)
{
    if (ast->type != ast::program)
    {
        throw source_error("Invalid AST root.", ast->location);
    }

    vector<func_def_ptr> defs;

    {
        context_type::scope_holder scope(m_context);

        auto stmt_list = ast->as_list();
        for (auto & stmt : stmt_list->elements)
        {
            defs.push_back( do_stmt(stmt) );
        }
    }

    return defs;
}

func_def_ptr generator::do_stmt(ast::node_ptr root)
{
    auto name = root->as_list()->elements[0]->as_leaf<string>()->value;
    auto params_node = root->as_list()->elements[1];
    auto block = root->as_list()->elements[2];

    cout << "func def: " << name << endl;

    vector<tuple<string,func_var_ptr,parsing::location>> params;
    if (params_node)
    {
        for(auto & param : params_node->as_list()->elements)
        {
            auto name = param->as_leaf<string>()->value;
            auto var = make_shared<func_var>();
            cout << "param: " << name << endl;
            params.emplace_back(name,var,param->location);
        }
    }

    vector<func_def_ptr> defs;
    expr_ptr expr;

    {
        context_type::scope_holder scope(m_context);

        for (auto & param : params)
        {
            try {
                m_context.bind(get<0>(param), get<1>(param));
            } catch (context_error & e) {
                throw source_error(e.what(), get<2>(param));
            }
        }

        expr = do_block(block, defs);
    }

    auto func = make_shared<func_def>();
    func->name = name;
    for (auto & param : params)
        func->vars.push_back(get<1>(param));
    func->defs = defs;
    func->expr = expr;

    return func;
}

expr_ptr generator::do_block(ast::node_ptr root, vector<func_def_ptr> & defs)
{
    auto stmts_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    if (stmts_node)
    {
        for (auto & stmt : stmts_node->as_list()->elements)
        {
            defs.push_back( do_stmt(stmt) );
        }
    }

    return do_expr(expr_node);
}

expr_ptr generator::do_expr(ast::node_ptr root)
{
    switch(root->type)
    {
    case ast::constant:
    {
        if (auto b = dynamic_pointer_cast<ast::leaf_node<bool>>(root))
            return make_shared<constant<bool>>(b->value);
        else if(auto i = dynamic_pointer_cast<ast::leaf_node<int>>(root))
            return make_shared<constant<int>>(i->value);
        else if(auto d = dynamic_pointer_cast<ast::leaf_node<double>>(root))
            return make_shared<constant<double>>(d->value);
        else
            throw source_error("Invalid constant type.", root->location);
    }
    case ast::identifier:
    {
        auto name = root->as_leaf<string>()->value;
        auto item = m_context.find(name);
        if (!item)
            throw source_error("Undefined name.", root->location);
        cout << "Found id: " << name << endl;
        return item.value();
    }
    case ast::primitive:
    case ast::array_def:
    case ast::array_apply:
    case ast::func_apply:
    default:
        throw source_error("Unsupported expression.", root->location);
    }
}

}
}
