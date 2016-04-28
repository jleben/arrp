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

expr_ptr copier::visit_int(const shared_ptr<constant<int>> & i)
{
    return make_shared<constant<int>>(*i);
}

expr_ptr copier::visit_double(const shared_ptr<constant<double>> & d)
{
    return make_shared<constant<double>>(*d);
}

expr_ptr copier::visit_bool(const shared_ptr<constant<bool>> & b)
{
    return make_shared<constant<bool>>(*b);
}

expr_ptr copier::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto a_var = dynamic_pointer_cast<array_var>(ref->var))
    {
        auto binding = m_copy_context.find(a_var);
        if (binding)
            return make_shared<reference>(binding.value(), ref->location);
        else
        {
            if (verbose<functional::model>::enabled())
                cout << "WARNING: Copying: no substitution for array var: " << a_var << endl;
            return make_shared<reference>(*ref);
        }
    }
    else if (auto f_var = dynamic_pointer_cast<func_var>(ref->var))
    {
        auto binding = m_copy_context.find(f_var);
        if (binding)
            return make_shared<reference>(binding.value(), ref->location);
        else
            return make_shared<reference>(*ref);
    }
    else if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        // FIXME: Go reduce the expression of id to replace
        // array vars with copied ones.
        auto binding = m_copy_context.find(id);
        if (binding)
        {
            auto bound_id = dynamic_pointer_cast<identifier>(binding.value());
            assert(bound_id);
            m_ids.insert(bound_id);
            return make_shared<reference>(bound_id, ref->location);
        }
        else
            return make_shared<reference>(*ref);
    }
    else
    {
        throw error("Unexpected reference type.");
    }
}

expr_ptr copier::visit_array_self_ref(const shared_ptr<array_self_ref> & ref)
{
    assert(!m_array_copy_stack.empty());
    auto arr = m_array_copy_stack.top();
    return make_shared<array_self_ref>(arr, ref->location);
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

expr_ptr copier::visit_affine(const shared_ptr<affine_expr> &)
{
    throw error("Affine expression not supported.");
}

expr_ptr copier::visit_cases(const shared_ptr<case_expr> & c)
{
    auto result = make_shared<case_expr>();
    result->location = c->location;
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

    stacker<array_ptr> array_stacker(new_arr, m_array_copy_stack);

    context_type::scope_holder scope(m_copy_context);

    for (auto & var : arr->vars)
    {
        auto new_var = make_shared<array_var>(var->name, copy(var->range), var->location);
        new_var->range.location = var->range.location;

        new_arr->vars.push_back(new_var);

        m_copy_context.bind(var, new_var);
    }

    for(auto & id : arr->scope.ids)
    {
        auto new_expr = copy(id->expr);
        auto new_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_name, new_expr, id->location);
        new_arr->scope.ids.push_back(new_id);
        m_copy_context.bind(id, new_id);
    }

    new_arr->expr = copy(arr->expr);

    return new_arr;
}

expr_ptr copier::visit_array_app(const shared_ptr<array_app> & app)
{
    auto new_app = make_shared<array_app>();
    new_app->location = app->location;
    new_app->object = copy(app->object);
    for (auto & arg : app->args)
        new_app->args.push_back(copy(arg));
    return new_app;
}

expr_ptr copier::visit_array_size(const shared_ptr<array_size> & as)
{
    auto new_as = make_shared<array_size>();
    new_as->location = as->location;
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

    context_type::scope_holder scope(m_copy_context);

    for (auto & var : func->vars)
    {
        auto new_var = make_shared<func_var>(*var);
        new_func->vars.push_back(new_var);
        m_copy_context.bind(var, new_var);
    }

    for(auto & id : func->scope.ids)
    {
        auto new_expr = copy(id->expr);
        auto new_name = m_name_provider.new_name(id->name);
        auto new_id = make_shared<identifier>(new_name, new_expr, id->location);
        new_func->scope.ids.push_back(new_id);
        m_copy_context.bind(id, new_id);
    }

    new_func->expr = copy(func->expr);

    return new_func;
}

expr_ptr copier::visit_func_app(const shared_ptr<func_app> & app)
{
    auto new_app = make_shared<func_app>();
    new_app->location = app->location;
    new_app->object = copy(app->object);
    for (auto & arg : app->args)
        new_app->args.push_back( copy(arg) );
    return new_app;
}

}
}
