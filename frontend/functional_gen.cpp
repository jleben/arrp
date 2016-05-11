#include "functional_gen.hpp"
#include "error.hpp"
#include "../utility/stacker.hpp"

using namespace std;

namespace stream {
namespace functional {

unordered_map<string, primitive_op> generator::m_prim_ops =
{
    { "exp", primitive_op::exp },
    { "exp2", primitive_op::exp2 },
    { "log", primitive_op::log },
    {  "log2", primitive_op::log2 },
     { "log10", primitive_op::log10 },
     { "sqrt", primitive_op::sqrt },
     { "sin", primitive_op::sin },
     { "cos", primitive_op::cos },
     { "tan", primitive_op::tan },
     { "asin", primitive_op::asin },
     { "acos", primitive_op::acos },
     { "atan", primitive_op::atan },
     { "ceil", primitive_op::ceil },
     { "floor", primitive_op::floor },
     { "abs", primitive_op::abs },
     { "min", primitive_op::min },
     { "max", primitive_op::max },
};

vector<id_ptr>
generator::generate(ast::node_ptr ast)
{
    if (ast->type != ast::program)
    {
        throw source_error("Invalid AST root.", ast->location);
    }

    functional::scope func_scope;
    m_func_scope_stack.push(&func_scope);

    {
        context_type::scope_holder scope(m_context);

        auto stmt_list = ast->as_list();
        for (auto & stmt : stmt_list->elements)
        {
            do_stmt(stmt);
        }
    }

    m_func_scope_stack.pop();

    vector<id_ptr> ids(func_scope.ids.begin(), func_scope.ids.end());
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

    shared_ptr<function> func;

    if (!params.empty())
    {
        func = make_shared<function>(params, nullptr, root->location);
        m_func_scope_stack.push(&func->scope);
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

        expr = do_block(block);
    }

    if (func)
    {
        m_func_scope_stack.pop();
        func->expr = expr_slot(expr);
        expr = func;
    }

    auto id = make_shared<identifier>(name, expr, name_node->location);

    try  {
        m_context.bind(name, id);
    } catch (context_error & e) {
        throw source_error(e.what(), name_node->location);
    }

    assert(!m_func_scope_stack.empty());
    m_func_scope_stack.top()->ids.push_back(id);

    return id;
}

expr_ptr generator::do_block(ast::node_ptr root)
{
    auto stmts_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    if (stmts_node)
    {
        for (auto & stmt : stmts_node->as_list()->elements)
        {
            do_stmt(stmt);
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
        ++var->ref_count;
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
    case ast::array_enum:
    {
        return do_array_enum(root);
    }
    case ast::array_concat:
    {
        return do_array_concat(root);
    }
    case ast::array_apply:
    {
        return do_array_apply(root);
    }
    case ast::array_size:
    {
        expr_ptr object = do_expr(root->as_list()->elements[0]);
        expr_ptr dim;
        auto dim_node = root->as_list()->elements[1];
        if (dim_node)
            dim = do_expr(dim_node);
        return make_shared<array_size>(object, dim, root->location);
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

    auto result = make_shared<case_expr>();

    vector<pair<expr_ptr,expr_ptr>> cases;

    expr_ptr else_domain;

    for (auto a_case : cases_node->as_list()->elements)
    {
        auto domain_node = a_case->as_list()->elements[0];
        auto expr_node = a_case->as_list()->elements[1];
        auto domain = do_expr(domain_node);
        auto expr = do_expr(expr_node);

        result->cases.emplace_back(expr_slot(domain), expr_slot(expr));

        auto domain_copy = do_expr(domain_node);
        if (else_domain)
            else_domain = make_shared<primitive>
                    (primitive_op::logic_or, else_domain, domain_copy);
        else
            else_domain = domain_copy;
    }

    else_domain = make_shared<primitive>(primitive_op::negate, else_domain);
    auto else_expr = do_expr(else_node);

    // FIXME: location of else_domain?
    result->cases.emplace_back(expr_slot(else_domain), expr_slot(else_expr));

    result->location = root->location;

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
    ar->expr = expr_slot(expr);

    return ar;
}

expr_ptr generator::do_array_apply(ast::node_ptr root)
{
    auto object_node = root->as_list()->elements[0];
    auto args_node = root->as_list()->elements[1];

    auto object = do_expr(object_node);

    vector<expr_slot> args;
    for (auto & arg_node : args_node->as_list()->elements)
    {
        args.emplace_back(do_expr(arg_node));
    }

    auto result = make_shared<array_app>();
    result->object = expr_slot(object);
    result->args = args;
    result->location = root->location;

    return result;
}

expr_ptr generator::do_array_enum(ast::node_ptr root)
{
    auto result = make_shared<operation>();
    result->location = root->location;
    result->kind = operation::array_enumerate;

    for (auto & child_node : root->as_list()->elements)
    {
        result->operands.emplace_back(do_expr(child_node));
    }

    return result;
}

expr_ptr generator::do_array_concat(ast::node_ptr root)
{
    auto result = make_shared<operation>();
    result->location = root->location;
    result->kind = operation::array_concatenate;

    for (auto & child_node : root->as_list()->elements)
    {
        result->operands.emplace_back(do_expr(child_node));
    }

    return result;
}

expr_ptr generator::do_func_apply(ast::node_ptr root)
{
    auto object_node = root->as_list()->elements[0];
    auto args_node = root->as_list()->elements[1];

    vector<expr_slot> args;
    for (auto & arg_node : args_node->as_list()->elements)
    {
        args.emplace_back(do_expr(arg_node));
    }

    // Handle primitive functions

    if (object_node->type == ast::identifier)
    {
        auto & name = object_node->as_leaf<string>()->value;
        auto op_it = m_prim_ops.find(name);
        if (op_it != m_prim_ops.end())
        {
            auto op_type = op_it->second;
            auto op = make_shared<primitive>(op_type, args);
            op->location = root->location;
            return op;
        }
    }

    auto object = do_expr(object_node);

    auto result = make_shared<func_app>();
    result->object = expr_slot(object);
    result->args = args;
    result->location = root->location;

    return result;
}

}
}
