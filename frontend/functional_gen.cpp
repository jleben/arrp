#include "functional_gen.hpp"
#include "error.hpp"
#include "../utility/stacker.hpp"

using namespace std;

namespace stream {
namespace functional {

vector<id_ptr>
generator::generate(ast::node_ptr ast)
{
    if (ast->type != ast::program)
    {
        throw source_error("Invalid AST root.", ast->location);
    }

    vector<id_ptr> ids;

    {
        context_type::scope_holder scope(m_context);

        auto stmt_list = ast->as_list();
        for (auto & stmt : stmt_list->elements)
        {
            auto id = do_stmt(stmt);
            ids.push_back(id);
        }
    }

    return ids;
}

id_ptr generator::do_stmt(ast::node_ptr root)
{
    auto name_node = root->as_list()->elements[0]->as_leaf<string>();
    auto name = name_node->value;
    auto params_node = root->as_list()->elements[1];
    auto block = root->as_list()->elements[2];

    vector<func_var_ptr> params;
    if (params_node)
    {
        for(auto & param : params_node->as_list()->elements)
        {
            auto name = param->as_leaf<string>()->value;
            auto var = make_shared<func_var>(name, param->location);
            params.push_back(var);
        }
    }

    vector<id_ptr> local_ids;
    expr_ptr expr;

    {
        context_type::scope_holder scope(m_context);

        for (auto & param : params)
        {
            try {
                m_context.bind(param->name, param);
            } catch (context_error & e) {
                throw source_error(e.what(), param->location);
            }
        }

        expr = do_block(block, local_ids);
    }

    if (!local_ids.empty())
    {
        expr = make_shared<expr_scope>(local_ids, expr, block->location);
    }

    if (!params.empty())
    {
        expr = make_shared<function>(params, expr, root->location);
    }

    auto id = make_shared<identifier>(name, expr, name_node->location);

    try  {
        m_context.bind(name, id);
    } catch (context_error & e) {
        throw source_error(e.what(), name_node->location);
    }

    return id;
}

expr_ptr generator::do_block(ast::node_ptr root, vector<id_ptr> & local_ids)
{
    auto stmts_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    if (stmts_node)
    {
        for (auto & stmt : stmts_node->as_list()->elements)
        {
            local_ids.push_back( do_stmt(stmt) );
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
        auto var = item.value();
        return make_shared<reference>(var, root->location);
    }
    case ast::array_self_ref:
    {
        if (m_array_stack.empty())
        {
            throw source_error("Array self reference without array.",
                               root->location);
        }
        auto arr = m_array_stack.top();
        arr->is_recursive = true;
        return make_shared<array_self_ref>(arr, root->location);
    }
    case ast::primitive:
    {
        return do_primitive(root);
    }
    case ast::case_expr:
    {
        return do_case_expr(root);
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

expr_ptr generator::do_case_expr(ast::node_ptr root)
{
    auto cases_node = root->as_list()->elements[0];
    auto else_node = root->as_list()->elements[1];

    vector<pair<expr_ptr,expr_ptr>> cases;

    expr_ptr else_domain;

    for (auto a_case : cases_node->as_list()->elements)
    {
        auto domain_node = a_case->as_list()->elements[0];
        auto expr_node = a_case->as_list()->elements[1];
        auto domain = do_expr(domain_node);
        auto expr = do_expr(expr_node);
        cases.emplace_back(domain, expr);

        auto domain_copy = do_expr(domain_node);
        if (else_domain)
            else_domain = make_shared<primitive>
                    (primitive_op::logic_or, else_domain, domain_copy);
        else
            else_domain = domain_copy;
    }

    else_domain = make_shared<primitive>(primitive_op::negate, else_domain);
    // FIXME: location of else_domain?
    cases.emplace_back(else_domain, do_expr(else_node));

    auto result = make_shared<case_expr>();
    result->location = root->location;
    result->cases = cases;
    return result;
}

expr_ptr generator::do_array_def(ast::node_ptr root)
{
    auto params_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    auto ar = make_shared<array>();
    ar->location = root->location;
    stacker<array_ptr> ar_stacker(ar, m_array_stack);

    vector<array_var_ptr> params;

    for (auto & param : params_node->as_list()->elements)
    {
        auto name_node = param->as_list()->elements[0];
        auto size_node = param->as_list()->elements[1];

        auto name = name_node->as_leaf<string>()->value;

        expr_ptr range;
        if (size_node)
            range = do_expr(size_node);

        auto var = make_shared<array_var>(name, range, param->location);

        params.push_back(var);
    }

    expr_ptr expr;

    {
        context_type::scope_holder scope(m_context);
        for (auto & param : params)
        {
            try {
                m_context.bind(param->name, param);
            } catch (context_error & e) {
                throw source_error(e.what(), param->location);
            }
        }

        expr = do_expr(expr_node);
    }

    for (auto & param : params)
        ar->vars.push_back(param);
    ar->expr = expr;

    return ar;
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
