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
            auto def = do_stmt(stmt);
            defs.push_back(def);
        }
    }

    return defs;
}

func_def_ptr generator::do_stmt(ast::node_ptr root)
{
    auto name = root->as_list()->elements[0]->as_leaf<string>()->value;
    auto params_node = root->as_list()->elements[1];
    auto block = root->as_list()->elements[2];

    vector<tuple<string,func_var_ptr,parsing::location>> params;
    if (params_node)
    {
        for(auto & param : params_node->as_list()->elements)
        {
            auto name = param->as_leaf<string>()->value;
            auto var = make_shared<func_var>();
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
    func->location = root->location;

    m_context.bind(func->name, make_shared<func_id>(func));

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
            return make_shared<constant<bool>>(b->value, root->location);
        else if(auto i = dynamic_pointer_cast<ast::leaf_node<int>>(root))
            return make_shared<constant<int>>(i->value, root->location);
        else if(auto d = dynamic_pointer_cast<ast::leaf_node<double>>(root))
            return make_shared<constant<double>>(d->value, root->location);
        else
            throw source_error("Invalid constant type.", root->location);
    }
    case ast::identifier:
    {
        auto name = root->as_leaf<string>()->value;
        auto item = m_context.find(name);
        if (!item)
            throw source_error("Undefined name.", root->location);
        auto v = item.value();
        if (auto avar = dynamic_pointer_cast<array_var>(v))
            return make_shared<array_var_ref>(avar, root->location);
        else if(auto fvar = dynamic_pointer_cast<func_var>(v))
            return make_shared<func_var_ref>(fvar, root->location);
        else if (auto fid = dynamic_pointer_cast<func_id>(v))
            return make_shared<func_ref>(fid, root->location);
        else
            throw source_error("Invalid reference type.", root->location);
    }
    case ast::primitive:
    {
        return do_primitive(root);
    }
    case ast::array_def:
    {
        return do_array_def(root);
    }
    case ast::array_apply:
    {
        return do_array_apply(root);
    }
    case ast::func_apply:
    {
        return do_func_apply(root);
    }
    default:
        throw source_error("Unsupported expression.", root->location);
    }
}

expr_ptr generator::do_primitive(ast::node_ptr root)
{
    auto type_node = root->as_list()->elements[0];
    auto type = type_node->as_leaf<primitive_op>()->value;

    vector<expr_ptr> operands;
    for(int i = 1; i < root->as_list()->elements.size(); ++i)
    {
        auto expr_node = root->as_list()->elements[i];
        operands.push_back( do_expr(expr_node) );
    }

    auto op = make_shared<primitive>(type, operands);
    op->location = root->location;

    return op;
}

expr_ptr generator::do_array_def(ast::node_ptr root)
{
    auto params_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    vector<tuple<string,array_var_ptr,parsing::location>> params;

    for (auto & param : params_node->as_list()->elements)
    {
        auto name_node = param->as_list()->elements[0];
        auto size_node = param->as_list()->elements[1];

        auto name = name_node->as_leaf<string>()->value;

        auto var = make_shared<array_var>();
        if (size_node)
            var->range = do_expr(size_node);
        else
            var->range = nullptr;

        params.emplace_back(name, var, param->location);
    }

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

        expr = do_expr(expr_node);
    }

    auto array = make_shared<array_def>();
    for (auto & param : params)
        array->vars.push_back(get<1>(param));
    array->expr = expr;
    array->location = root->location;

    return array;
}

expr_ptr generator::do_array_apply(ast::node_ptr root)
{
    auto object_node = root->as_list()->elements[0];
    auto args_node = root->as_list()->elements[1];

    auto object = do_expr(object_node);

    vector<expr_ptr> args;
    for (auto & arg_node : args_node->as_list()->elements)
    {
        args.push_back(do_expr(arg_node));
    }

    auto result = make_shared<array_app>();
    result->object = object;
    result->args = args;
    result->location = root->location;

    return result;
}

expr_ptr generator::do_func_apply(ast::node_ptr root)
{
    auto object_node = root->as_list()->elements[0];
    auto args_node = root->as_list()->elements[1];

    auto object = do_expr(object_node);

    vector<expr_ptr> args;
    for (auto & arg_node : args_node->as_list()->elements)
    {
        args.push_back(do_expr(arg_node));
    }

    auto result = make_shared<func_app>();
    result->object = object;
    result->args = args;
    result->location = root->location;

    return result;
}

}
}
