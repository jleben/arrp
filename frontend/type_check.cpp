#include "type_check.hpp"
#include "linear_expr_gen.hpp"
#include "error.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

template <typename T>
struct mention_ {
    const T & d;
    mention_(const T & d): d(d) {}
};

ostream & operator<< (ostream & s, const mention_<location_type> & m)
{
    s << "@" << m.d << ": ";
    return s;
}

template <typename T>
mention_<T> mention(const T & d) { return mention_<T>(d); }

string text(primitive_op op, const vector<primitive_type> & args)
{
    ostringstream text;
    text << op
        << " ( ";
    for (auto & t : args)
        text << t << " ";
    text << ")";
    return text.str();
}

type_checker::type_checker(stack<location_type> & trace):
    m_trace(trace)
{
    m_printer.set_print_var_address(true);
    m_printer.set_print_scopes(false);
}

type_ptr type_checker::process(const id_ptr & id)
{
    // FIXME: if this is a top-level id, add separator to trace.

    if (verbose<type_checker>::enabled())
    {
        cout << "Type checker: processing id:" << endl << "  ";
        m_printer.print(id, cout);
        cout << endl;
    }

    auto t = visit(id->expr);

    if (id->is_recursive)
    {
        if (verbose<type_checker>::enabled())
            cout << "Type checker: revisiting recursive id:" << id << endl;

        revertable<bool> revisit(m_force_revisit, true);

        t = visit(id->expr);
        if (verbose<type_checker>::enabled())
            cout << "1 -> " << *t << endl;

        auto t2 = visit(id->expr);
        if (verbose<type_checker>::enabled())
            cout << "2 -> " << *t2 << endl;

        bool is_undefined =
                (!(*t2 == *t))
                || (t->is_scalar() && t->scalar()->primitive == primitive_type::undefined)
                || (t->is_array() && t->array()->element == primitive_type::undefined);
        if (is_undefined)
            throw type_error("Type of recursive expression undefined.",
                             id->expr.location);
    }

    if (t->is_scalar())
    {
        if (!t->scalar()->is_data())
            throw type_error("Expression can not be used as data.",
                             id->expr.location);
    }

    if (verbose<type_checker>::enabled())
        cout << "Type checker: processed id: " << id->name << endl;

    return t;
}

void type_checker::process(const expr_ptr & expr)
{
    if (verbose<type_checker>::enabled())
    {
        cout << "Type checker: processing expression:" << endl;
        m_printer.print(expr, cout);
        cout << endl;
    }

    visit(expr);
}

type_ptr type_checker::visit(const expr_ptr & expr)
{
    if (m_force_revisit || !expr->type)
        expr->type = visitor<type_ptr>::visit(expr);

    return expr->type;
}

type_ptr type_checker::visit_int(const shared_ptr<int_const> &)
{
    auto s = make_shared<scalar_type>(primitive_type::integer);
    s->constant_flag = true;
    s->affine_flag = true;
    s->data_flag = true;
    return s;
}

type_ptr type_checker::visit_real(const shared_ptr<real_const> &)
{
    auto s = make_shared<scalar_type>(primitive_type::real64);
    s->constant_flag = true;
    s->affine_flag = true;
    s->data_flag = true;
    return s;
}

type_ptr type_checker::visit_complex(const shared_ptr<complex_const> &)
{
    auto s = make_shared<scalar_type>(primitive_type::complex64);
    s->constant_flag = true;
    s->affine_flag = false;
    s->data_flag = true;
    return s;
}

type_ptr type_checker::visit_bool(const shared_ptr<bool_const> &)
{
    auto s = make_shared<scalar_type>(primitive_type::boolean);
    s->constant_flag = true;
    s->affine_flag = false;
    s->data_flag = true;
    return s;
}

type_ptr type_checker::visit_infinity(const shared_ptr<infinity> &)
{
    return type::infinity();
}

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (ref->is_recursion)
        {
            if (id->expr->type)
                return id->expr->type;
            else
                return type::undefined();
        }
        else
        {
            return process(id);
        }
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        auto s = make_shared<scalar_type>(primitive_type::integer);
        s->constant_flag = false;
        s->affine_flag = true;
        s->data_flag = true;
        return s;
    }
    else
    {
        throw error("Unexpected.");
    }
}

type_ptr type_checker::visit_primitive(const shared_ptr<primitive> & prim)
{
    array_size_vec common_size;
    vector<primitive_type> elem_types;
    vector<type_ptr> operand_types;

    for (auto & operand : prim->operands)
    {
        auto type = visit(operand);
        operand_types.push_back(type);

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw type_error("Function not allowed as operand.", operand.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            try {
                common_size = common_array_size(common_size, arr->size);
            } catch (no_type &) {
                ostringstream msg;
                msg << "Operand size mismatch: "
                    << arr->size << " != " << common_size;
                throw type_error(msg.str(), operand.location);
            }

            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
        }
        else
        {
            throw error("Unexpected operand type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = result_type(prim->kind, elem_types);
    }
    catch (no_type &)
    {
        string msg("Invalid operand types: ");
        msg += text(prim->kind, elem_types);
        throw type_error(msg, prim->location);
    }
    catch (ambiguous_type &)
    {
        string msg("Ambiguous type resolution:");
        msg += text(prim->kind, elem_types);
        throw type_error(msg, prim->location);
    }

    if (common_size.empty())
    {
        auto s = make_shared<scalar_type>(result_elem_type);

        s->data_flag = true;
        for (auto & t : operand_types)
            s->data_flag &= t->is_data();

        // This denotes whether the result depends on a variable,
        // rather than whether it is a compile-time constant.
        s->constant_flag = true;
        for (auto & t : operand_types)
            s->constant_flag &= t->is_constant();

        switch(prim->kind)
        {
        case primitive_op::add:
        case primitive_op::subtract:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->affine_flag = lhs->is_affine() && rhs->is_affine();
            break;
        }
        case primitive_op::multiply:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->affine_flag =
                    lhs->is_affine() && rhs->is_affine() &&
                    (lhs->is_constant() || rhs->is_constant());
            break;
        }
        case primitive_op::divide_integer:
        case primitive_op::modulo:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->affine_flag =
                    lhs->is_affine() && rhs->is_affine() &&
                    rhs->is_constant();
            break;
        }
        default:
            // FIXME: Reject non-data operands for all other operations?
            ;
        }

        return s;
    }
    else
    {
        return make_shared<array_type>(common_size, result_elem_type);
    }
}

type_ptr type_checker::visit_operation(const shared_ptr<operation> & op)
{
    switch(op->kind)
    {
    case operation::array_concatenate:
        return process_array_concat(op);
    case operation::array_enumerate:
        return process_array_enum(op);
    default:
        throw error("Unexpected operation type.");
    }
}

type_ptr type_checker::visit_cases(const shared_ptr<case_expr> & cexpr)
{
    array_size_vec common_size;
    vector<primitive_type> elem_types;

    for (auto & c : cexpr->cases)
    {
        auto & domain = c.first;
        auto & expr = c.second;

        auto type = visit(expr);

        if (!type->is_data())
        {
            throw type_error("Expression can not be used as data.", expr.location);
        }

        visit(domain);

        to_linear_set(domain);

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw type_error("Function not allowed in case.", expr.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            try {
                common_size = common_array_size(common_size, arr->size);
            } catch (no_type &) {
                ostringstream msg;
                msg << "Case size mismatch: "
                    << arr->size << " != " << common_size;
                throw type_error(msg.str(), expr.location);
            }

            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
        }
        else
        {
            throw error("Unexpected case type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw type_error("Incompatible case types.", cexpr->location);
    }

    if (common_size.empty())
    {
        auto st = make_shared<scalar_type>(result_elem_type);
        st->data_flag = true;
        return st;
    }
    else
    {
        return make_shared<array_type>(common_size, result_elem_type);
    }
}

type_ptr type_checker::visit_array(const shared_ptr<array> & arr)
{
    if (verbose<type_checker>::enabled())
        cout << mention(arr->location)
             << "Visiting array." << endl;

    auto type = process_array(arr);

    if (arr->is_recursive)
    {
        if (verbose<type_checker>::enabled())
            cout << mention(arr->location)
                 << "Revisiting recursive array." << endl;

        // We need another pass with proper type for self-references.
        arr->type = type;
        revertable<bool> revisit(m_force_revisit, true);
        type = process_array(arr);

        assert(type->is_array());
        if (type->array()->element == primitive_type::undefined)
        {
            throw type_error("Array type can not be inferred.",
                             arr->location);
        }
    }
    return type;
}

type_ptr type_checker::process_array(const shared_ptr<array> & arr)
{
    for (auto & var : arr->vars)
    {
        if (!var->range)
            continue;

        auto type = visit(var->range);

        if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
        {
            if (c->value < 1)
            {
                ostringstream msg;
                msg << "Array size not positive (" <<  c->value << ")";
                throw type_error(msg.str(), var->range.location);
            }
        }
        else
        {
            bool ok = (bool) dynamic_pointer_cast<infinity>(var->range.expr);
            if (!ok)
            {
                ostringstream msg;
                msg << "Array size is not a constant integer or infinity: "
                    << var->range->type;
                throw type_error(msg.str(), var->range.location);
            }
        }
    }

    array_size_vec common_subdom_size;
    vector<primitive_type> elem_types;

    auto process_type = [&](expr_slot & e)
    {
        auto & type = e->type;

        if (!type->is_data())
        {
            throw type_error("Expression can not be used as data.", e.location);
        }

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw type_error("Function not allowed here.", e.location);
        }
        else if (auto at = dynamic_pointer_cast<array_type>(type))
        {
            try {
                common_subdom_size = common_array_size(common_subdom_size, at->size);
            } catch (no_type &) {
                ostringstream msg;
                msg << "Size mismatch: "
                    << at->size << " != " << common_subdom_size;
                throw type_error(msg.str(), e.location);
            }

            elem_types.push_back(at->element);
        }
        else if (auto st = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(st->primitive);
        }
        else
        {
            throw error("Unexpected type.");
        }
    };

    auto patterns = dynamic_pointer_cast<array_patterns>(arr->expr.expr);
    for (auto & pattern : patterns->patterns)
    {
        if (pattern.domains)
        {
            auto cexpr = dynamic_pointer_cast<case_expr>(pattern.domains.expr);

            for (auto & c : cexpr->cases)
            {
                auto & domain = c.first;
                auto & expr = c.second;

                visit(domain);
                to_linear_set(domain);

                visit(expr);
                process_type(expr);
            }
        }

        visit(pattern.expr);
        process_type(pattern.expr);
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw type_error("Incompatible types.", arr->location);
    }

    patterns->type = type_for(common_subdom_size, result_elem_type);

    // Limit array size

    m_array_bounding.bound(arr);

    // Extract array size from range of array variables

    array_size_vec size;

    for (auto & var : arr->vars)
    {
        if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
            size.push_back(c->value);
        else
            size.push_back(-1);
    }

    // Expand array space with common space of subdomains

    size.insert(size.end(), common_subdom_size.begin(), common_subdom_size.end());

    auto type = make_shared<array_type>(size, result_elem_type);
    return type;
}

type_ptr type_checker::visit_array_patterns(const shared_ptr<array_patterns> & ap)
{
    throw error("Unexpected.");
}

type_ptr type_checker::process_array_concat(const shared_ptr<operation> & op)
{
    array_size_vec common_elem_size;
    int total_elem_count = 0;
    vector<primitive_type> elem_types;

    for (auto & operand : op->operands)
    {
        if (total_elem_count == -1)
        {
            throw type_error
                    ("Can not concatenate to end of infinite array.",
                     operand.location);
        }

        auto type = visit(operand);

        if (!type->is_data())
        {
            throw type_error("Expression can not be used as data.",
                               operand.location);
        }

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw type_error("Function not allowed here.", operand.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            assert(!arr->size.empty());

            int elem_count = arr->size[0];
            array_size_vec elem_size(++arr->size.begin(), arr->size.end());

            if (elem_count >= 0)
                total_elem_count += elem_count;
            else
                total_elem_count = -1;

            try {
                common_elem_size = common_array_size(common_elem_size, elem_size);
            } catch (no_type &) {
                ostringstream msg;
                msg << "Element size mismatch: "
                    << elem_size << " != " << common_elem_size;
                throw type_error(msg.str(), operand.location);
            }

            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
            ++total_elem_count;
        }
        else
        {
            throw error("Unexpected operand type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw type_error("Incompatible element types.", op->location);
    }

    assert(total_elem_count != 0);

    array_size_vec result_size;
    result_size.push_back(total_elem_count);
    result_size.insert(result_size.end(),
                       common_elem_size.begin(), common_elem_size.end());

    return make_shared<array_type>(result_size, result_elem_type);
}

type_ptr type_checker::process_array_enum(const shared_ptr<operation> & op)
{
    array_size_vec common_elem_size;
    vector<primitive_type> elem_types;

    for (auto & operand : op->operands)
    {
        auto type = visit(operand);

        if (!type->is_data())
        {
            throw type_error("Expression can not be used as data.",
                               operand.location);
        }

        if (dynamic_pointer_cast<function_type>(type))
        {
            throw type_error("Function not allowed here.", operand.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(type))
        {
            try {
                common_elem_size = common_array_size(common_elem_size, arr->size);
            } catch (no_type &) {
                ostringstream msg;
                msg << "Element size mismatch: "
                    << arr->size << " != " << common_elem_size;
                throw type_error(msg.str(), operand.location);
            }

            elem_types.push_back(arr->element);
        }
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(type))
        {
            elem_types.push_back(scalar->primitive);
        }
        else
        {
            throw error("Unexpected operand type.");
        }
    }

    primitive_type result_elem_type;

    try {
        result_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw type_error("Incompatible element types.", op->location);
    }

    array_size_vec result_size;
    result_size.push_back(op->operands.size());
    result_size.insert(result_size.end(),
                       common_elem_size.begin(), common_elem_size.end());

    return make_shared<array_type>(result_size, result_elem_type);
}

type_ptr type_checker::visit_array_self_ref(const shared_ptr<array_self_ref> & self)
{
    vector<int> size;

    auto & arr = self->arr;

    if (arr->type)
    {
        if (verbose<type_checker>::enabled())
            cout << mention(self->location)
                 << "Self reference using known array type: "
                 << *arr->type << endl;
        return arr->type;
    }

    for (auto & var : arr->vars)
    {
        if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
        {
            size.push_back(c->value);
        }
        else
        {
            size.push_back(-1);
        }
    }

    auto result = make_shared<array_type>(size, primitive_type::undefined);
    if (verbose<type_checker>::enabled())
        cout << mention(self->location)
             << "Self reference synthesized type: " << *result << endl;

    return result;
}

type_ptr type_checker::visit_array_app(const shared_ptr<array_app> & app)
{
    auto object_type = visit(app->object);
    array_size_vec object_size;
    primitive_type elem_type;

    if (object_type->is_array())
    {
        auto at = object_type->array();
        object_size = at->size;
        elem_type = at->element;
    }
    else if (object_type->is_scalar())
    {
        auto st = object_type->scalar();
        elem_type = st->primitive;
    }
    else
    {
        ostringstream msg;
        msg << "Object of type " << *object_type << " can not be applied as array.";
        throw type_error(msg.str(), app->object.location);
    }

    if(auto self = dynamic_pointer_cast<array_self_ref>(app->object.expr))
    {
        int num_vars = self->arr->vars.size();
        if (num_vars != app->args.size())
        {
            ostringstream msg;
            msg << "Array self reference must be applied exactly."
                << " Expected " << num_vars << " arguments, but "
                << app->args.size() << " given.";
            throw type_error(msg.str(), app->location);
        }
    }

    for (int arg_idx = 0; arg_idx < app->args.size(); ++arg_idx)
    {
        auto & arg = app->args[arg_idx];

        auto arg_type = dynamic_pointer_cast<scalar_type>(visit(arg));

        if (!arg_type)
        {
            throw type_error("Array argument is not a scalar.",
                              arg.location);
        }
        if (arg_type->primitive != primitive_type::integer)
        {
            throw type_error("Array argument is not an integer.",
                               arg.location);
        }

        if (arg_idx >= object_size.size())
            continue;

        // FIXME: test lower & upper bound
#if 0
        auto lin_arg = to_linear_expr(arg);
        auto max_arg = maximum(lin_arg);
        int bound = object_size[arg_idx];
        if (max_arg.is_constant())
        {
            int max_value = max_arg.constant();
            if (bound != array_var::unconstrained && max_value >= bound)
            {
                ostringstream msg;
                msg << "Argument range (" << max_value << ")"
                    << " out of array bound (" << bound << ")";
                throw type_error(msg.str(),
                                   arg.location);
            }
        }
        else
        {
            if (bound != array_var::unconstrained)
            {
                throw type_error("Unbounded argument to"
                                   " bounded array dimension.",
                                   arg.location);
            }
        }
#endif
    }

    if (object_size.size() <= app->args.size())
    {
        auto st = make_shared<scalar_type>(elem_type);
        st->data_flag = true;
        return st;
    }
    else
    {
        vector<int> remaining_size(object_size.begin() + app->args.size(),
                                   object_size.end());
        return make_shared<array_type>(remaining_size, elem_type);
    }
}

type_ptr type_checker::visit_array_size(const shared_ptr<array_size> & as)
{
    visit(as->object);

    if (as->object->type->is_scalar())
    {
        // Result is always 1.
        auto result = make_shared<scalar_type>(primitive_type::integer);
        result->constant_flag = true;
        result->affine_flag = true;
        result->data_flag = true;

        return result;
    }
    else if (!as->object->type->is_array())
    {
        ostringstream msg;
        msg << "Invalid type of object for array size operation: "
            << as->object->type;
        throw type_error(msg.str(), as->object.location);
    }

    auto arr_type = as->object->type->array();

    assert(!arr_type->size.empty());

    int dim = 1;

    if (as->dimension)
    {
        auto dim_const = dynamic_pointer_cast<constant<int>>(as->dimension.expr);
        if (!dim_const)
            throw type_error("Not an integer constant.", as->dimension.location);

        dim = dim_const->value;
        if (dim < 1)
        {
            ostringstream msg;
            msg << "Dimension index out of bounds: " << dim;
            throw type_error(msg.str(), as->dimension.location);
        }
    }

    dim -= 1;

    int size;
    if (dim < arr_type->size.size())
        size = arr_type->size[dim];
    else
        size = 1;

    if (size >= 0)
    {
        auto result = make_shared<scalar_type>(primitive_type::integer);
        result->constant_flag = true;
        result->affine_flag = true;
        result->data_flag = true;
        return result;
    }
    else
    {
        return type::infinity();
    }
}

type_ptr type_checker::visit_func_app(const shared_ptr<func_app> & app)
{
    // Application of external functions

    auto func_type = dynamic_pointer_cast<function_type>(visit(app->object));
    if (!func_type)
        throw type_error("Not a function.", app->object.location);

    if (app->args.size() != func_type->param_count())
    {
        ostringstream msg;
        msg << "Wrong number of arguments in function application: "
            << func_type->param_count() << " expected, "
            << app->args.size() << " given.";
        throw type_error(msg.str(), app->location);
    }

    for (auto & arg : app->args)
        visit(arg);

    for (int p = 0; p < func_type->param_count(); ++p)
    {
        assert(func_type->params[p]);
        const auto & pt = *func_type->params[p];
        const auto & at = *app->args[p]->type;
        bool ok = at <= pt;
        if (!ok)
        {
            ostringstream msg;
            msg << "Argument type mismatch:"
                << " expected " << pt
                << " but given " << at;
            throw type_error(msg.str(), app->args[p].location);
        }
    }

    assert(func_type->value);
    return func_type->value;
}

type_ptr type_checker::visit_func(const shared_ptr<function> & func)
{
    return make_shared<function_type>(func->vars.size());
}

type_ptr type_checker::visit_affine(const shared_ptr<affine_expr> &)
{
    throw error("Not supported.");
}

type_ptr type_checker::visit_external(const shared_ptr<external> & ext)
{
    return ext->type;
}

}
}
