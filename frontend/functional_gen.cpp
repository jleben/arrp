#include "functional_gen.hpp"
#include "error.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

using namespace std;

namespace stream {
namespace functional {

unordered_map<string, primitive_op> generator::m_prim_ops =
{
    { "exp", primitive_op::exp },
    { "exp2", primitive_op::exp2 },
    { "log", primitive_op::log },
    { "log2", primitive_op::log2 },
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
    { "real", primitive_op::real },
    { "imag", primitive_op::imag },
    { "int", primitive_op::to_integer },
    { "real32", primitive_op::to_real32 },
    { "real64", primitive_op::to_real64 },
    { "complex32", primitive_op::to_complex32 },
    { "complex64", primitive_op::to_complex64 },
};

source_error generator::module_error(const string & what, const parsing::location & loc)
{
    return source_error(what, location_in_module(loc));
}

vector<id_ptr> generator::generate(const vector<module*> modules)
{
    vector<id_ptr> ids;

    for (auto mod : modules)
    {
        revertable<module*> current_module(m_current_module, mod);
        stacker<string,name_stack_t> name_stacker(m_name_stack);
        if (mod != modules.back())
            name_stacker.push(mod->name);

        vector<id_ptr> mod_ids = generate(mod);

        for (auto id : mod_ids)
            m_final_ids.emplace(id->name, id);

        ids.insert(ids.end(), mod_ids.begin(), mod_ids.end());
    }

    return ids;
}

vector<id_ptr>
generator::generate(module * mod)
{
    if (verbose<generator>::enabled())
    {
        cout << "## Generating functional model for module "
             << mod->name << " ##" << endl;
    }

    auto ast = mod->ast;

    if (ast->type != ast::program)
    {
        throw module_error("Invalid AST root.", ast->location);
    }

    functional::scope func_scope;
    m_scope_stack.push(&func_scope);

    {
        context_type::scope_holder scope(m_context);

        auto declarations = ast->as_list()->elements[2];

        if (declarations)
        {
            do_mutually_recursive_declarations(declarations->as_list()->elements);
        }
    }

    m_scope_stack.pop();

    vector<id_ptr> ids(func_scope.ids.begin(), func_scope.ids.end());

    return ids;
}

void generator::do_mutually_recursive_declarations(const vector<ast::node_ptr> & nodes)
{
    vector<id_ptr> ids;

    for (auto & node : nodes)
    {
        auto id = make_id_for_declaration(node);
        ids.push_back(id);
    }

    auto id_it = ids.begin();
    auto node_it = nodes.begin();

    for(; node_it != nodes.end(); ++node_it, ++id_it)
    {
        make_expr_for_declaration(*id_it, *node_it);
    }
}

id_ptr generator::do_binding(ast::node_ptr root)
{
    auto id = make_id_for_declaration(root);

    make_expr_for_declaration(id, root);

    return id;
}

id_ptr generator::make_id_for_declaration(ast::node_ptr node)
{
    auto name_node = node->as_list()->elements[0]->as_leaf<string>();
    auto name = name_node->value;

    auto id = make_shared<identifier>(qualified_name(name),
                                      location_in_module(name_node->location));

    // Bind name for reference resolution

    try  {
        m_context.bind(name, id);
    } catch (context_error & e) {
        throw module_error(e.what(), name_node->location);
    }

    // Store name in current scope

    if (verbose<generator>::enabled())
        cout << "Storing id " << id->name << endl;

    assert(!m_scope_stack.empty());
    m_scope_stack.top()->ids.push_back(id);

    return id;
}

void generator::make_expr_for_declaration(id_ptr id, ast::node_ptr root)
{
    auto name = root->as_list()->elements[0]->as_leaf<string>()->value;

    expr_ptr expr;

    stacker<string, name_stack_t> name_stacker(name, m_name_stack);

    switch(root->type)
    {
    case ast::input:
    case ast::external:
    {
        auto type_node = root->as_list()->elements[1];

        auto ext = make_shared<external>();
        ext->name = name;
        ext->type_expr = expr_slot(do_type_expr(type_node));

        expr = ext;

        break;
    }
    case ast::binding:
    {
        auto type_node = root->as_list()->elements[1];
        auto params_node = root->as_list()->elements[2];
        auto expr_node = root->as_list()->elements[3];

        if (type_node)
        {
            id->type_expr = do_type_expr(type_node);
        }

        if (params_node)
        {
            expr = make_func(params_node, expr_node, root->location);
        }
        else
        {
            expr = do_expr(expr_node);
        }

        break;
    }
    default:
        throw error("Unexpected expression.");
    }

    id->expr = expr_slot(expr);
}

expr_ptr generator::do_expr(ast::node_ptr root)
{
    switch(root->type)
    {
    case ast::binding:
    {
        return do_binding_expr(root);
    }
    case ast::local_scope:
    {
        return do_local_scope(root);
    }
    case ast::lambda:
    {
        return do_lambda(root);
    }
    case ast::constant:
    {
        if (auto b = dynamic_pointer_cast<ast::leaf_node<bool>>(root))
            return make_shared<bool_const>(b->value, location_in_module(root->location));
        else if(auto i = dynamic_pointer_cast<ast::leaf_node<int>>(root))
            return make_shared<int_const>(i->value, location_in_module(root->location));
        else if(auto d = dynamic_pointer_cast<ast::leaf_node<double>>(root))
            return make_shared<real_const>(d->value, location_in_module(root->location));
        else if (auto c = dynamic_pointer_cast<ast::leaf_node<complex<double>>>(root))
            return make_shared<complex_const>(c->value, location_in_module(root->location));
        else
            throw module_error("Invalid constant type.", root->location);
    }
    case ast::infinity:
    {
        return make_shared<infinity>(location_in_module(root->location));
    }
    case ast::identifier:
    {
        auto name = root->as_leaf<string>()->value;
        if (verbose<generator>::enabled())
            cout << "Looking up name: " << name << endl;

        auto item = m_context.find(name);
        if (!item)
            throw module_error("Undefined name.", root->location);

        auto var = item.value();
        ++var->ref_count;

        auto ref = make_shared<reference>(var, location_in_module(root->location));

        return ref;
    }
    case ast::qualified_id:
    {
        auto & elems = root->as_list()->elements;

        auto module_name = elems[0]->as_leaf<string>()->value;
        auto module_alias_decl = m_current_module->imports.find(module_name);
        if(module_alias_decl != m_current_module->imports.end())
            module_name = module_alias_decl->second->name;

        auto name = elems[1]->as_leaf<string>()->value;
        auto qname = module_name + '.' + name;

        auto decl = m_final_ids.find(qname);
        if (verbose<generator>::enabled())
            cout << "Looking up qualified name: " << qname << endl;
        if (decl == m_final_ids.end())
            throw module_error("Undefined name.", root->location);

        auto id = decl->second;
        ++id->ref_count;
        return make_shared<reference>(id, location_in_module(root->location));
    }
    case ast::array_self_ref:
    {
        if (m_array_stack.empty())
        {
            throw module_error("Array self reference without array.",
                               root->location);
        }
        auto arr = m_array_stack.top();
        arr->is_recursive = true;
        return make_shared<array_self_ref>(arr, location_in_module(root->location));
    }
    case ast::primitive:
    {
        return do_primitive(root);
    }
#if 0
    case ast::case_expr:
    {
        return do_case_expr(root);
    }
#endif
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
        return make_shared<array_size>(object, dim,
                                       location_in_module(root->location));
    }
    case ast::func_apply:
    {
        return do_func_apply(root);
    }
    case ast::func_compose:
    {
        return do_func_comp(root);
    }
    default:
        throw module_error("Unsupported expression.", root->location);
    }
}

expr_ptr generator::do_binding_expr(ast::node_ptr root)
{
    context_type::scope_holder scope(m_context);

    auto id = do_binding(root);

    auto ref = make_shared<reference>(id, location_in_module(root->location));

    return ref;
}

expr_ptr generator::do_local_scope(ast::node_ptr root)
{
    auto decls_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    context_type::scope_holder scope(m_context);

    do_mutually_recursive_declarations(decls_node->as_list()->elements);

    return do_expr(expr_node);
}

expr_ptr generator::do_lambda(ast::node_ptr root)
{
    auto params_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    return make_func(params_node, expr_node, root->location);
}

expr_ptr generator::make_func(ast::node_ptr params_node, ast::node_ptr expr_node,
                               const parsing::location & loc)
{
    assert(params_node);

    vector<func_var_ptr> params;

    {
        for(auto & param : params_node->as_list()->elements)
        {
            auto name = param->as_leaf<string>()->value;
            auto var = make_shared<func_var>(name, location_in_module(param->location));
            var->qualified_name = qualified_name(name);
            params.push_back(var);
        }
    }

    assert(!params.empty());

    auto func = make_shared<function>(params, nullptr, location_in_module(loc));
    m_scope_stack.push(&func->scope);

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

        func->expr = expr_slot(do_expr(expr_node));
    }

    m_scope_stack.pop();

    return func;
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
    op->location = location_in_module(root->location);

    return op;
}

#if 0
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

    result->location = location_in_module(root->location);

    return result;
}
#endif
expr_ptr generator::do_array_def(ast::node_ptr root)
{
    auto ranges_node = root->as_list()->elements[0];
    auto patterns_node = root->as_list()->elements[1];

    auto ar = make_shared<array>();
    ar->location = location_in_module(root->location);
    stacker<array_ptr> ar_stacker(ar, m_array_stack);

    if (ranges_node)
    {
        for (auto & range_node : ranges_node->as_list()->elements)
        {
            string name = string("i") + to_string(ar->vars.size());

            expr_ptr range = do_expr(range_node);

            auto var = make_shared<array_var>
                    (name, range, location_in_module(range_node->location));

            ar->vars.push_back(var);
        }
    }

    stacker<scope*> scope_stacker(&ar->scope, m_scope_stack);

    ar->expr = do_array_patterns(patterns_node);

    return ar;
}

expr_ptr generator::do_array_patterns(ast::node_ptr root)
{
    auto patterns = make_shared<array_patterns>();
    patterns->location = location_in_module(root->location);

    for (auto & pattern_node : root->as_list()->elements)
    {
        patterns->patterns.push_back(do_array_pattern(pattern_node));
    }

    return patterns;
}

array_patterns::pattern
generator::do_array_pattern(ast::node_ptr root)
{
    auto ar = m_array_stack.top();

    auto index_nodes = root->as_list()->elements[0];
    auto domains_node = root->as_list()->elements[1];
    auto universal_node = root->as_list()->elements[2];

    array_patterns::pattern pattern;

    // If array range has not been declared, create variables on the fly
    // with no range, meaning that range must be inferred.
    if (ar->vars.empty())
    {
        for (int i = 0; i < (int)index_nodes->as_list()->elements.size(); ++i)
        {
            string name = string("i") + to_string(i);
            auto var = make_shared<array_var>
                    (name, nullptr, location_type());
            ar->vars.push_back(var);
        }
    }
    // Otherwise make sure the number of index dimensions match
    // the number of array dimensions.
    else if (index_nodes->as_list()->elements.size() != ar->vars.size())
    {
        throw source_error("Number of indexes does not match number "
                           "of array dimensions.",
                           location_in_module(index_nodes->location));
    }

    // Create index descriptions
    // and bind variable index names

    context_type::scope_holder scope(m_context);

    int dim_idx = 0;
    for (auto & index_node : index_nodes->as_list()->elements)
    {
        struct array_patterns::index i;
        if (index_node->type == ast::identifier)
        {
            i.is_fixed = false;

            assert(dim_idx < ar->vars.size());

            auto name = index_node->as_leaf<string>()->value;
            auto & var = ar->vars[dim_idx];

            try
            {
                m_context.bind(name, var);
            }
            catch (context_error & e)
            {
                throw source_error
                        (e.what(), location_in_module(index_node->location));
            }
        }
        else if(auto literal_int = dynamic_pointer_cast<ast::leaf_node<int>>(index_node))
        {
            i.is_fixed = true;
            i.value = literal_int->value;
        }
        else
        {
            throw source_error("Invalid array index pattern.",
                               location_in_module(index_node->location));
        }
        pattern.indexes.push_back(i);
        ++dim_idx;
    }

    // Process the pattern expression

    if (domains_node)
        pattern.domains = expr_slot(do_array_domains(domains_node));

    pattern.expr = expr_slot(do_expr(universal_node));

    return pattern;
}

expr_ptr generator::do_array_domains(ast::node_ptr root)
{
    auto domains = make_shared<case_expr>();
    domains->location = location_in_module(root->location);

    for (auto & domain_node : root->as_list()->elements)
    {
        auto & d = domain_node->as_list()->elements[0];
        auto & e = domain_node->as_list()->elements[1];
        domains->cases.emplace_back
                (expr_slot(do_expr(d)), expr_slot(do_expr(e)));
    }

    return domains;
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
    result->location = location_in_module(root->location);

    return result;
}

expr_ptr generator::do_array_enum(ast::node_ptr root)
{
    auto result = make_shared<operation>();
    result->location = location_in_module(root->location);
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
    result->location = location_in_module(root->location);
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
            op->location = location_in_module(root->location);
            return op;
        }
    }

    auto object = do_expr(object_node);

    auto result = make_shared<func_app>();
    result->object = expr_slot(object);
    result->args = args;
    result->location = location_in_module(root->location);

    return result;
}

expr_ptr generator::do_func_comp(ast::node_ptr root)
{
    auto left_node = root->as_list()->elements[0];
    auto right_node = root->as_list()->elements[1];

    string var_name("_x");
    auto var = make_shared<func_var>(var_name, location_type());
    var->qualified_name = qualified_name(var_name);

    vector<func_var_ptr> params = { var };

    expr_ptr left, right;

    auto func = make_shared<function>(params, nullptr, location_in_module(root->location));
    {
        auto stacker = stack_scoped(&func->scope, m_scope_stack);

        left = do_expr(left_node);
        right = do_expr(right_node);
    }

    auto app1 = make_shared<func_app>();
    app1->object = expr_slot(right, location_in_module(right_node->location));
    app1->args = { expr_slot(make_shared<reference>(var)) };

    auto app2 = make_shared<func_app>();
    app2->object = expr_slot(left, location_in_module(left_node->location));
    app2->args = { expr_slot(app1) };

    func->expr = app2;

    return func;
}

expr_ptr generator::do_type_expr(ast::node_ptr root)
{
    auto location = location_in_module(root->location);

    switch(root->type)
    {
    case ast::identifier:
    {
        auto name = root->as_leaf<string>()->value;
        auto expr = make_shared<type_name_expr>();
        expr->name = name;
        expr->location = location;
        return expr;
    }
    case ast::array_type:
    {
        auto array = make_shared<array_type_expr>();
        auto size_nodes = root->as_list()->elements[0];
        auto elem_node = root->as_list()->elements[1];
        for (auto & size_node : size_nodes->as_list()->elements)
        {
            array->size.emplace_back(do_expr(size_node));
        }
        array->element = expr_slot(do_type_expr(elem_node));
        return array;
    }
    case ast::function_type:
    {
        auto func = make_shared<func_type_expr>();
        auto params_node = root->as_list()->elements[0];
        auto result_node = root->as_list()->elements[1];
        for (auto & param_node : params_node->as_list()->elements)
        {
            func->params.emplace_back(do_type_expr(param_node));
        }
        func->result = expr_slot(do_type_expr(result_node));
        return func;
    }
    default:
        throw error("Unexpected expression.");
    }
}
#if 0
type_ptr generator::do_type(ast::node_ptr root)
{
    switch(root->type)
    {
    case ast::scalar_type:
    {
        auto name = root->as_leaf<string>()->value;
        auto ptype = primitive_type_for_name(name);
        if (ptype == primitive_type::undefined)
            throw source_error("Invalid type name.",
                               location_in_module(root->location));
        return make_shared<scalar_type>(ptype);
    }
    case ast::array_type:
    {
        return do_array_type(root);
    }
    case ast::function_type:
    {
        return do_function_type(root);
    }
    default:
        throw error("Unexpected type.");
    }
}

type_ptr generator::do_array_type(ast::node_ptr root)
{
    auto size_nodes = root->as_list()->elements[0];
    auto elem_node = root->as_list()->elements[1];

    array_size_vec size;

    for(const auto & size_node : size_nodes->as_list()->elements)
    {
        if (size_node->type == ast::constant)
        {
            if(auto i = dynamic_pointer_cast<ast::leaf_node<int>>(size_node))
            {
                auto v = i->value;
                if (v >= 0)
                    size.push_back(v);
                else
                    throw source_error("Array size less than 1.",
                                       location_in_module(size_node));
            }
            else
                throw source_error("Array size not integer.",
                                   location_in_module(size_node));
        }
        else if (size_node->type == ast::infinity)
        {
            size.push_back(-1);
        }
        else
        {
            throw source_error("Invalid array size.",
                               location_in_module(size_node));
        }
    }

    auto elem_name = elem_node->as_leaf<string>()->value;
    auto elem_type = primitive_type_for_name(elem_name);
    if (elem_type == primitive_type::undefined)
        throw source_error("Invalid type name.",
                           location_in_module(elem_node));

    return make_shared<array_type>(size, elem_type);
}

type_ptr generator::do_function_type(ast::node_ptr root)
{
    auto params_node = root->as_list()->elements[0]->as_list();
    auto value_node = root->as_list()->elements[1];

    auto func_type = make_shared<function_type>();

    auto is_infinite_array_type = [](type_ptr t) -> bool
    {
        if (!t->is_array())
            return false;

        for (auto & s : t->array()->size)
            if (s < 0)
                return true;

        return false;
    };

    for (auto & param_node : params_node->elements)
    {
        auto param_type = do_type(param_node);

        if (is_infinite_array_type(param_type))
        {
            throw source_error("Infinite array not allowed as function parameter.",
                               location_in_module(param_node->location));
        }

        func_type->params.push_back(param_type);
    }

    func_type->value = do_type(value_node);

    if (is_infinite_array_type(func_type->value))
    {
        throw source_error("Infinite array not allowed as function value.",
                           location_in_module(value_node->location));
    }

    return func_type;
}
#endif

primitive_type generator::primitive_type_for_name(const string & name)
{
    static unordered_map<string, primitive_type> map =
    {
        { "bool", primitive_type::boolean },
        { "int", primitive_type::integer },
        { "real32", primitive_type::real32 },
        { "real64", primitive_type::real64 },
        { "complex32", primitive_type::complex32 },
        { "complex64", primitive_type::complex64 },
    };

    auto m = map.find(name);
    if (m == map.end())
        return primitive_type::undefined;
    else
        return m->second;
}

string generator::qualified_name(const string & name)
{
    ostringstream qname;
    for (auto & name : m_name_stack)
        qname << name << '.';
    qname << name;
    return qname.str();
}

}
}
