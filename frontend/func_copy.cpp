#include "func_copy.hpp"
#include "../utility/debug.hpp"

using namespace std;

namespace stream {
namespace functional {

copier::copier(unordered_set<id_ptr> & ids, name_provider & nmp):
    m_ids(ids),
    m_name_provider(nmp)
{

}

expr_ptr copier::visit_int(const shared_ptr<int_const> & i)
{
    return make_shared<int_const>(*i);
}

expr_ptr copier::visit_real(const shared_ptr<real_const> & d)
{
    return make_shared<real_const>(*d);
}

expr_ptr copier::visit_complex(const shared_ptr<complex_const> & d)
{
    return make_shared<complex_const>(*d);
}

expr_ptr copier::visit_bool(const shared_ptr<bool_const> & b)
{
    return make_shared<bool_const>(*b);
}

expr_ptr copier::visit_infinity(const shared_ptr<infinity> & inf)
{
    return make_shared<infinity>(*inf);
}

expr_ptr copier::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto a_var = dynamic_pointer_cast<array_var>(ref->var))
    {
        auto new_ref = make_shared<reference>(*ref);
        auto binding = m_copy_context.find(a_var);
        if (binding)
            new_ref->var = binding.value();
        return new_ref;
    }
    else if (auto f_var = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto new_ref = make_shared<reference>(*ref);
        auto binding = m_copy_context.find(f_var);
        if (binding)
            new_ref->var = binding.value();
        return new_ref;
    }
    else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        // FIXME: Go reduce the expression of id to replace
        // array vars with copied ones.

        auto new_ref = make_shared<reference>(*ref);

        auto binding = m_copy_context.find(id);
        if (binding)
        {
            auto bound_id = dynamic_pointer_cast<identifier>(binding.value());
            assert(bound_id);
            m_ids.insert(bound_id);
            new_ref->var = bound_id;
        }

        return new_ref;
    }
    else
    {
        throw error("Unexpected reference type.");
    }
}

expr_ptr copier::visit_array_self_ref(const shared_ptr<array_self_ref> & ref)
{
    // There may not be an array on stack if we are only copying
    // a nested expression.
    array_ptr arr;
    if (m_array_copy_stack.empty())
        arr = ref->arr;
    else
        arr = m_array_copy_stack.top();
    return make_shared<array_self_ref>(arr, ref->location, ref->type);
}

expr_ptr copier::visit_primitive(const shared_ptr<primitive> & op)
{
    auto new_op = make_shared<primitive>();
    new_op->location = op->location;
    new_op->type = op->type;
    new_op->kind = op->kind;
    for (auto & operand : op->operands)
    {
        new_op->operands.push_back(copy(operand));
    }
    return new_op;
}

expr_ptr copier::visit_operation(const shared_ptr<operation> & op)
{
    auto new_op = make_shared<operation>();
    new_op->location = op->location;
    new_op->type = op->type;
    new_op->kind = op->kind;
    for (auto & operand : op->operands)
    {
        new_op->operands.push_back(copy(operand));
    }
    return new_op;
}

expr_ptr copier::visit_affine(const shared_ptr<affine_expr> &)
{
    throw error("Affine expression not supported.");
}

expr_ptr copier::visit_cases(const shared_ptr<case_expr> & c)
{
    auto result = make_shared<case_expr>();
    result->location = c->location;
    result->type = c->type;
    for (auto & a_case : c->cases)
    {
        result->cases.emplace_back(copy(a_case.first), copy(a_case.second));
    }
    return result;
}

expr_ptr copier::visit_array(const shared_ptr<array> & arr)
{
    auto new_arr = make_shared<array>();
    new_arr->location = arr->location;
    new_arr->type = arr->type;
    new_arr->is_recursive = arr->is_recursive;

    stacker<array_ptr> array_stacker(new_arr, m_array_copy_stack);

    context_type::scope_holder scope(m_copy_context);

    for (auto & var : arr->vars)
    {
        auto new_var = make_shared<array_var>(copy(var->range), var->location);
        new_var->range.location = var->range.location;

        new_arr->vars.push_back(new_var);

        m_copy_context.bind(var, new_var);
    }

    new_arr->expr = copy(arr->expr);

    return new_arr;
}

expr_ptr copier::visit_array_patterns(const shared_ptr<array_patterns> & ap)
{
    auto new_ap = make_shared<array_patterns>();
    new_ap->location = ap->location;
    new_ap->type = ap->type;

    for (auto & pattern : ap->patterns)
    {
        array_patterns::pattern new_p;

        context_type::scope_holder scope(m_copy_context);

        new_p.indexes = pattern.indexes;

        if (pattern.domains)
            new_p.domains = copy(pattern.domains);

        new_p.expr = copy(pattern.expr);

        new_ap->patterns.push_back(new_p);
    }

    return new_ap;
}

expr_ptr copier::visit_array_app(const shared_ptr<array_app> & app)
{
    auto new_app = make_shared<array_app>();
    new_app->location = app->location;
    new_app->type = app->type;
    new_app->object = copy(app->object);
    for (auto & arg : app->args)
        new_app->args.push_back(copy(arg));
    return new_app;
}

expr_ptr copier::visit_array_size(const shared_ptr<array_size> & as)
{
    auto new_as = make_shared<array_size>();
    new_as->location = as->location;
    new_as->type = as->type;
    new_as->object = copy(as->object);
    auto &dim = as->dimension;
    if (dim)
        new_as->dimension = copy(dim);
    return new_as;
}

expr_ptr copier::visit_func(const shared_ptr<function> & func)
{
    auto new_func = make_shared<function>();
    new_func->location = func->location;
    new_func->type = func->type;

    context_type::scope_holder scope(m_copy_context);

    for (auto & var : func->vars)
    {
        auto new_var = make_shared<func_var>(*var);
        new_func->vars.push_back(new_var);
        m_copy_context.bind(var, new_var);
    }

    new_func->expr = copy(func->expr);

    return new_func;
}

expr_ptr copier::visit_func_app(const shared_ptr<func_app> & app)
{
    auto new_app = make_shared<func_app>();
    new_app->location = app->location;
    new_app->type = app->type;
    new_app->object = copy(app->object);
    for (auto & arg : app->args)
        new_app->args.push_back( copy(arg) );
    return new_app;
}

expr_ptr copier::visit_external(const shared_ptr<external> & e)
{
    auto r = make_shared<external>();
    r->location = e->location;
    r->type = e->type;
    r->is_input = e->is_input;
    r->name = e->name;
    r->type_expr = copy(e->type_expr);
    return r;
}

expr_ptr copier::visit_type_name(const shared_ptr<type_name_expr> & e)
{
    auto r = make_shared<type_name_expr>();
    r->location = e->location;
    r->type = e->type;
    r->name = e->name;
    return r;
}

expr_ptr copier::visit_array_type(const shared_ptr<array_type_expr> & e)
{
    auto r = make_shared<array_type_expr>();
    r->location = e->location;
    r->type = e->type;

    for (auto & size : e->size)
        r->size.push_back(copy(size));

    r->element = e->element;

    return r;
}

expr_ptr copier::visit_func_type(const shared_ptr<func_type_expr> & e)
{
    auto r = make_shared<func_type_expr>();
    r->location = e->location;
    r->type = e->type;

    for (auto & param : e->params)
        r->params.push_back(copy(param));

    r->result = copy(e->result);

    return r;
}

expr_ptr copier::visit_scope(const shared_ptr<scope_expr> & e)
{
    auto r = make_shared<scope_expr>(expr_slot());
    r->location = e->location;
    r->type = e->type;

    context_type::scope_holder scope(m_copy_context);

    for(auto & id : e->local.ids)
    {
        auto new_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_name, id->expr, id->location);
        new_id->type_expr = copy(id->type_expr);
        new_id->explicit_type = id->explicit_type;
        new_id->is_recursive = id->is_recursive;
        r->local.ids.push_back(new_id);

        m_copy_context.bind(id, new_id);
    }

    // Do the copying after binding all new ids,
    // to support mutual recursion
    for(auto & id : r->local.ids)
    {
        id->expr = copy(id->expr);
    }

    r->value = copy(e->value);

    return r;
}

}
}
