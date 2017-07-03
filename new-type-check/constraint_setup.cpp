#include "constraint_setup.hpp"
#include "../utility/debug.hpp"
#include "../common/func_model_printer.hpp"

using namespace std;

namespace arrp {

using stream::verbose;
using namespace stream;
using functional::function;
using functional::array;

type_constraint_setup::type_constraint_setup(type_graph & graph):
    m_graph(graph)
{}

void type_constraint_setup::process(const unordered_set<id_ptr> & ids)
{
    for (auto id : ids)
        process(id);
}

void type_constraint_setup::process(const id_ptr & id)
{
    type e = visit(id->expr);
    m_graph.make_equal(type_for(id), e);
}

type type_constraint_setup::type_for(const id_ptr & id)
{
    if (!id->type2)
        id->type2 = new type_variable;
    return id->type2;
}

type type_constraint_setup::visit(const expr_ptr & e)
{
    e->type2 = visitor::visit(e);
    return e->type2;
}

type type_constraint_setup::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        return type_for(id);
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        return new scalar_type(primitive_type::integer);
    }
    else
    {
        throw error("Unexpected reference type.");
    }
}

type type_constraint_setup::visit_int(const shared_ptr<int_const> & e)
{
    return new scalar_type(primitive_type::integer);
}

type type_constraint_setup::visit_real(const shared_ptr<real_const> & e)
{
    return new scalar_type(primitive_type::real64);
}

type type_constraint_setup::visit_complex(const shared_ptr<complex_const> &)
{
    return new scalar_type(primitive_type::complex64);
}

type type_constraint_setup::visit_bool(const shared_ptr<bool_const> &)
{
    return new scalar_type(primitive_type::boolean);
}

type type_constraint_setup::visit_infinity(const shared_ptr<infinity> &)
{
    return new infinity_type;
}

type type_constraint_setup::visit_array(const shared_ptr<array> & arr)
{
    type t = visit(arr->expr);
    for(auto & var : arr->vars)
        t = new array_type(t);
    return t;
}

type type_constraint_setup::visit_array_app(const shared_ptr<array_app> & app)
{
    auto object_type = visit(app->object);

    type element_type = new type_variable;
    type required_object_type = element_type;
    for (auto & arg : app->args)
    {
        required_object_type = new array_like_type(required_object_type);
    }

    m_graph.make_equal(object_type, required_object_type);

    return element_type;
}

type type_constraint_setup::visit_array_size(const shared_ptr<array_size> & as)
{
    // NOTE: Should be checked later
    visit(as->object);
    visit(as->dimension);

    return new scalar_type(primitive_type::integer);
}

type type_constraint_setup::visit_func(const shared_ptr<function> & func)
{
    throw error("Unexpected function.");
}

type type_constraint_setup::visit_func_app(const shared_ptr<func_app> & app)
{
    auto ext = dynamic_pointer_cast<external>(app->object.expr);
    if (!ext)
    {
        throw error("Unexpected type of object of function application.");
    }

    auto o = visit(ext);

    auto f = new function_type;

    for (auto & arg : app->args)
    {
        auto a = visit(arg);
        f->parameters.push_back(a);
    }

    f->value = new type_variable;

    m_graph.make_equal(o, f);

    return f->value;
}

}
