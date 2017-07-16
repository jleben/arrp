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

type type_constraint_setup::visit_cases(const shared_ptr<case_expr> & e)
{
    type r = new type_variable;

    for (auto & part : e->cases)
    {
        auto t = visit(part.second);
        m_graph.make_equal(t, new type_variable(data_type));
        m_graph.make_sub_type(t, r);
    }

    return r;
}

type type_constraint_setup::visit_array_patterns(const shared_ptr<array_patterns> & e)
{
    type r = new type_variable;

    for (auto & pattern : e->patterns)
    {
        auto t = visit(pattern.expr);
        m_graph.make_equal(t, new type_variable(data_type));
        m_graph.make_sub_type(t, r);
    }

    return r;
}

type type_constraint_setup::visit_array(const shared_ptr<array> & arr)
{
    type t = visit(arr->expr);
    for (int i = 0; i < arr->vars.size(); ++i)
        t = new array_type(t);
    return t;
}

type type_constraint_setup::visit_array_app(const shared_ptr<array_app> & app)
{
    auto object_type = visit(app->object);

    type element_type = new type_variable;
    type required_object_type = element_type;
    for (int i = 0; i < app->args.size(); ++i)
    {
        required_object_type = new type_variable(indexable_type, {required_object_type});
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

type type_constraint_setup::visit_primitive(const shared_ptr<primitive> & prim)
{
    vector<type> operand_elem_types;

    type result;

    for (auto & operand : prim->operands)
    {
        auto t = visit(operand);
        type e = new type_variable;
        m_graph.make_equal(t, new type_variable(array_like_type, {e}));
        operand_elem_types.push_back(e);
    }

    switch(prim->kind)
    {
    case primitive_op::negate:
        m_graph.make_equal(operand_elem_types[0], new type_variable(scalar_data_type));
        result = operand_elem_types[0];
        break;
    case primitive_op::add:
    case primitive_op::subtract:
    case primitive_op::multiply:
        m_graph.make_equal(operand_elem_types[0], new type_variable(numeric_type));
        m_graph.make_equal(operand_elem_types[1], new type_variable(numeric_type));
        result = new type_variable;
        break;
    case primitive_op::divide:
        m_graph.make_equal(operand_elem_types[0], new type_variable(numeric_type));
        m_graph.make_equal(operand_elem_types[1], new type_variable(numeric_type));
        result = new type_variable(array_like_type, { new type_variable(real_numeric_type) });
        break;
    case primitive_op::divide_integer:
        m_graph.make_equal(operand_elem_types[0], new type_variable(simple_numeric_type));
        m_graph.make_equal(operand_elem_types[1], new type_variable(simple_numeric_type));
        // FIXME: Doesn't work:
        // Result must be supertype, to handle array operands,
        // but its element type must be integer
        // - not necessarily a super-type of operand element types.
        result = new type_variable(array_like_type, { new scalar_type(primitive_type::integer) });
        break;
    case primitive_op::modulo:
        m_graph.make_equal(operand_elem_types[0], new scalar_type(primitive_type::integer));
        m_graph.make_equal(operand_elem_types[1], new scalar_type(primitive_type::integer));
        result = new type_variable(array_like_type, { new scalar_type(primitive_type::integer) });
        break;
    case primitive_op::raise:
    case primitive_op::min:
    case primitive_op::max:
        m_graph.make_equal(operand_elem_types[0], new type_variable(simple_numeric_type));
        m_graph.make_equal(operand_elem_types[1], new type_variable(simple_numeric_type));
        result = new type_variable;
        break;
    case primitive_op::exp:
    case primitive_op::log:
    case primitive_op::log10:
    case primitive_op::sin:
    case primitive_op::cos:
    case primitive_op::tan:
    case primitive_op::asin:
    case primitive_op::acos:
    case primitive_op::atan:
    case primitive_op::sqrt:
        m_graph.make_equal(operand_elem_types[0], new type_variable(numeric_type));
        result = new type_variable(real_numeric_type);
        m_graph.make_sub_type(operand_elem_types[0], result);
        break;
    case primitive_op::log2:
    {
        m_graph.make_equal(operand_elem_types[0], new type_variable(numeric_type));
        auto r = new type_variable;
        r->classes.emplace_back(real_numeric_type);
        r->classes.emplace_back(simple_numeric_type);
        result = r;
        m_graph.make_sub_type(operand_elem_types[0], result);
        break;
    }
    case primitive_op::exp2:
    case primitive_op::ceil:
    case primitive_op::floor:
    case primitive_op::abs:
        m_graph.make_equal(operand_elem_types[0], new type_variable(simple_numeric_type));
        result = new type_variable;
        m_graph.make_sub_type(operand_elem_types[0], result);
        break;
    case primitive_op::real:
    case primitive_op::imag:
        result = new type_variable;
        m_graph.make_sub_type(operand_elem_types[0], new type_variable(complex_numeric_type, {result}));
        break;
    default:
        throw error("Type constraints: This primitive kind not implemented.");
    };

    for (auto & operand : prim->operands)
    {
        m_graph.make_sub_type(operand, result);
    }

    return result;
}


}
