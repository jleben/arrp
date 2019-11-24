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

    vector<id_ptr> ids;

    {
        context_type::scope_holder scope(m_context);

        auto declarations = ast->as_list()->elements[2];

        if (declarations)
        {
            ids = do_mutually_recursive_declarations(declarations->as_list()->elements);
        }
    }

    return ids;
}

vector<id_ptr> generator::do_mutually_recursive_declarations(const vector<ast::node_ptr> & raw_nodes)
{
    auto nodes = process_array_element_declarations(raw_nodes);

    vector<id_ptr> unique_ids;
    vector<id_ptr> ids;

    for (auto & node : nodes)
    {
        //id_ptr id;
        //bool is_new;
        auto [id, id_is_new] = make_id_for_declaration(node);
        ids.push_back(id);
        if (id_is_new)
        {
            unique_ids.push_back(id);
            if (verbose<generator>::enabled())
                cerr << "-- Stored new id: " << id->name << endl;
        }
    }

    auto id_it = ids.begin();
    auto node_it = nodes.begin();

    for(; node_it != nodes.end(); ++node_it, ++id_it)
    {
        if (verbose<generator>::enabled())
            cerr << "-- Processing declaration for id: " << (*id_it)->name << endl;
        apply_declaration(*id_it, *node_it);
    }

    return unique_ids;
}

// Collect all array_element_def { name, pattern } with the same name,
// into a single node array_element_def { name, { pattern, pattern, ... } }.
vector<ast::node_ptr>
generator::process_array_element_declarations
(const vector<ast::node_ptr> & input)
{
    vector<ast::node_ptr> output;

    unordered_map<string, ast::node_ptr> array_pattern_map;

    for (auto & decl : input)
    {
        if (decl->type == ast::array_element_def)
        {
            auto & name_node = decl->as_list()->elements[0];
            auto & pattern_node = decl->as_list()->elements[1];

            auto & patterns = array_pattern_map[name_node->as_leaf<string>()->value];
            if (!patterns)
                patterns = ast::make_list(name_node->location, {});

            patterns->as_list()->elements.push_back(pattern_node);
        }
        else
        {
            output.push_back(decl);
        }
    }

    for (auto & pattern_def : array_pattern_map)
    {
        auto & name = pattern_def.first;
        auto & patterns_node = pattern_def.second;
        auto & loc = patterns_node->location;

        auto name_node = make_shared<ast::leaf_node<string>>(ast::identifier, loc, name);
        auto decl_node = ast::make_list(ast::array_element_def, loc, { name_node, patterns_node });
        output.push_back(decl_node);
    }

    return output;
}

tuple<id_ptr, bool> generator::make_id_for_declaration(ast::node_ptr node)
{
    // Note: Assuming that all declarations are a list with name as first element

    auto name_node = node->as_list()->elements[0]->as_leaf<string>();
    auto name = name_node->value;

    id_ptr id;

    // If ID already exists in context, abort.
    // This is to support type declarations.
    // In case of multiple type or value declarations for same ID,
    // we fail later in 'apply_declaration'.
    {
        auto item = m_context.find(name);
        if (item &&
            item.scope() == m_context.current_scope() &&
            (id = dynamic_pointer_cast<identifier>(item.value())))
        {
            if (verbose<generator>::enabled())
                cerr << "-- Not making identifier - it already exists: " << id->name << endl;

            return {id, false};
        }
    }

    id = make_shared<identifier>(qualified_name(name),
                                 location_in_module(name_node->location));

    if (verbose<generator>::enabled())
    {
        cerr << "-- Made an identifier: " << id->name << endl;
    }

    // Bind name for reference resolution

    try  {
        m_context.bind(name, id);
    } catch (context_error & e) {
        throw module_error(e.what(), name_node->location);
    }

    return {id, true};
}

void generator::apply_declaration(id_ptr id, ast::node_ptr root)
{
    auto name = root->as_list()->elements[0]->as_leaf<string>()->value;

    expr_ptr expr;

    stacker<string, name_stack_t> name_stacker(name, m_name_stack);

    // For output declarations, mark id as output.
    switch(root->type)
    {
    case ast::output:
    case ast::output_type:
    case ast::output_value:
    {
        if (id->is_external)
        {
            throw source_error("Name has already been defined as input or external function.",
                               location_in_module(root->location));
        }
        id->is_output = true;
        break;
    }
    default: break;
    }

    // Convert output declarations into other declarations
    // and then continue as usual.
    switch(root->type)
    {
    case ast::output:
        return;
    case ast::output_type:
        root->type = ast::id_type_decl;
        break;
    case ast::output_value:
        root->type = ast::binding;
        break;
    default: break;
    }

    // Generate id expression.
    switch(root->type)
    {
    case ast::id_type_decl:
    {
        if (id->type_expr)
        {
            throw source_error("Name already has a type declaration.",
                               location_in_module(root->location));
        }

        auto type_node = root->as_list()->elements[1];
        id->type_expr = expr_slot(do_type_expr(type_node));
        break;
    }
    case ast::input:
    case ast::external:
    {
        if (id->expr)
        {
            throw source_error("Name already defined.",
                               location_in_module(root->location));
        }
        if (id->type_expr)
        {
            throw source_error("Name already has a type declaration.",
                               location_in_module(root->location));
        }

        auto type_node = root->as_list()->elements[1];

        auto ext = make_shared<external>();
        ext->is_input = root->type == ast::input;
        ext->name = name;

        ext->type_expr = expr_slot(do_type_expr(type_node));
        id->expr = expr_slot(ext);
        id->type_expr = ext->type_expr;
        id->is_external = true;

        break;
    }
    case ast::binding:
    {
        if (id->expr)
        {
            throw source_error("Name already defined.",
                               location_in_module(root->location));
        }

        auto params_node = root->as_list()->elements[1];
        auto expr_node = root->as_list()->elements[2];

        expr_ptr expr;
        if (params_node)
        {
            expr = make_func(params_node, expr_node, root->location);
        }
        else
        {
            expr = do_expr(expr_node);
        }
        id->expr = expr_slot(expr);

        break;
    }
    case ast::array_element_def:
    {
        if (id->expr)
        {
            throw source_error("Name already defined.",
                               location_in_module(root->location));
        }

        auto & patterns_node = root->as_list()->elements[1];

        shared_ptr<array> array_def = make_shared<array>();

        {
            stacker<array_ptr> ar_stacker(array_def, m_array_stack);
            array_def->expr = do_array_patterns(patterns_node);
        }

        id->expr = array_def;

        break;
    }
    default:
        throw error("Unexpected expression.");
    }
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

    auto [id, is_new] = make_id_for_declaration(root);
    assert_or_throw(is_new);
    apply_declaration(id, root);

    auto e = make_shared<scope_expr>();
    e->location = location_in_module(root->location);
    e->local.ids.push_back(id);
    e->value = make_shared<reference>(id, e->location);

    return e;
}

expr_ptr generator::do_local_scope(ast::node_ptr root)
{
    auto decls_node = root->as_list()->elements[0];
    auto expr_node = root->as_list()->elements[1];

    auto e = make_shared<scope_expr>();
    e->location = location_in_module(root->location);

    context_type::scope_holder scope(m_context);

    e->local.ids = do_mutually_recursive_declarations(decls_node->as_list()->elements);
    e->value = do_expr(expr_node);

    return e;
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
            if (param->type != ast::identifier)
                throw source_error("Function parameter is not a name.", location_in_module(param->location));

            auto name = param->as_leaf<string>()->value;
            auto var = make_shared<func_var>(name, location_in_module(param->location));
            var->qualified_name = qualified_name(name);
            params.push_back(var);
        }
    }

    assert(!params.empty());

    auto func = make_shared<function>(params, nullptr, location_in_module(loc));

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
            expr_ptr range = do_expr(range_node);

            auto var = make_shared<array_var>
                    (range, location_in_module(range_node->location));

            ar->vars.push_back(var);
        }
    }

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

    array_patterns::pattern pattern;

    // If array range has not been declared, create variables on the fly
    // with no range, meaning that range must be inferred.
    if (ar->vars.empty())
    {
        for (int i = 0; i < (int)index_nodes->as_list()->elements.size(); ++i)
        {
            auto var = make_shared<array_var>
                    (nullptr, location_type());
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

        expr_ptr dx;
        if (d)
            dx = do_expr(d);
        else
            dx = make_shared<bool_const>(true, location_in_module(e->location));

        domains->cases.emplace_back(expr_slot(dx), expr_slot(do_expr(e)));
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
    expr_ptr result;

    switch(root->type)
    {
    case ast::identifier:
    {
        auto name = root->as_leaf<string>()->value;
        auto type_name = make_shared<type_name_expr>();
        type_name->name = name;
        result = type_name;
        break;
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
        array->element = elem_node->as_leaf<string>()->value;
        result = array;
        break;
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
        result = func;
        break;
    }
    default:
        throw error("Unexpected expression.");
    }

    result->location = location_in_module(root->location);

    return result;
}

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
