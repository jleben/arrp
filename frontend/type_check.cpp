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
}

void type_checker::process(const expr_ptr & expr)
{
    if (verbose<type_checker>::enabled())
    {
        cout << "Processing:" << endl;
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

type_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    type_ptr type;

    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "Visiting id:" << endl;
            m_printer.print(id, cout);
            cout << endl;
        }
        // FIXME: if this is a top-level id, add separator to trace.
        type = visit(id->expr);
        if (verbose<type_checker>::enabled())
            cout << "Done visiting id " << id->name << endl;
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        auto s = make_shared<scalar_type>(primitive_type::integer);
        s->constant_flag = false;
        s->affine_flag = true;
        s->data_flag = avar->range.expr != nullptr;
        type = s;
    }
    else
    {
        throw error("Unexpected.");
    }

    return type;
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

        switch(prim->kind)
        {
        case primitive_op::add:
        case primitive_op::subtract:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->data_flag = lhs->is_data() && rhs->is_data();
            s->constant_flag = lhs->is_constant() && rhs->is_constant();
            s->affine_flag = lhs->is_affine() && rhs->is_affine();
            break;
        }
        case primitive_op::multiply:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->data_flag = lhs->is_data() && rhs->is_data();
            s->constant_flag = lhs->is_constant() && rhs->is_constant();
            s->affine_flag =
                    lhs->is_affine() && rhs->is_affine() &&
                    (lhs->is_constant() || rhs->is_constant());
            break;
        }
        case primitive_op::divide:
        {
            auto lhs = operand_types[0];
            auto rhs = operand_types[1];
            s->data_flag = lhs->is_data() && rhs->is_data();
            s->constant_flag = lhs->is_constant() && rhs->is_constant();
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
    }
    return type;
}

type_ptr type_checker::process_array(const shared_ptr<array> & arr)
{
    array_size_vec size;

    for (auto & var : arr->vars)
    {
        if (var->range)
        {
            if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
            {
                if (c->value < 1)
                {
                    ostringstream msg;
                    msg << "Array bound not positive (" <<  c->value << ")";
                    throw type_error(msg.str(), var->range.location);
                }
                size.push_back(c->value);
            }
            else
            {
                throw type_error("Array bound not a constant integer.",
                                   var->range.location);
            }
        }
        else
        {
            size.push_back(-1);
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

    if (result_elem_type == primitive_type::undefined)
    {
        throw type_error("Array element type can not be inferred.",
                           arr->expr.location);
    }

    patterns->type = make_shared<array_type>(common_subdom_size, result_elem_type);

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
        if (var->range)
        {
            auto c = dynamic_pointer_cast<constant<int>>(var->range.expr);
            if (!c)
                throw error("Unexpected.");
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
        if (!arg_type->is_affine())
        {
            throw type_error("Array argument is not an affine expression.",
                               arg.location);
        }

        if (arg_idx >= object_size.size())
            continue;

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
    auto arr_type = dynamic_pointer_cast<array_type>(as->object->type);
    if (!arr_type)
        throw type_error("Not an array.", as->object.location);

    assert(!arr_type->size.empty());

    int dim = 1;

    if (as->dimension)
    {
        auto dim_const = dynamic_pointer_cast<constant<int>>(as->dimension.expr);
        if (!dim_const)
            throw type_error("Not an integer constant.", as->dimension.location);

        dim = dim_const->value;
        if (dim < 1 || dim > arr_type->size.size())
        {
            ostringstream msg;
            msg << "Dimension index out of bounds: " << dim;
            throw type_error(msg.str(), as->dimension.location);
        }
    }

    int size = arr_type->size[dim-1];
    assert(size >= -1);
    if (size == -1)
    {
        ostringstream msg;
        msg << "Dimension " << dim << " is unbounded.";
        throw type_error(msg.str(), as->location);
    }

    auto result = make_shared<scalar_type>(primitive_type::integer);
    result->constant_flag = true;
    result->affine_flag = true;
    result->data_flag = true;
    return result;
}

type_ptr type_checker::visit_func_app(const shared_ptr<func_app> & app)
{
    throw error("Unexpected.");
#if 0
    auto func_type = dynamic_pointer_cast<function_type>(visit(app->object));
    if (!func_type)
        throw type_error("Not a function.", app->object.location);

    if (app->args.size() > func_type->arg_count)
    {
        ostringstream msg;
        msg << "Too many arguments in function application: "
            << func_type->arg_count << " expected, "
            << app->args.size() << " given.";
        throw type_error(msg.str(), app->location);
    }

    int remaining_arg_count = func_type->arg_count - app->args.size();

    if (remaining_arg_count)
        return make_shared<function_type>(remaining_arg_count);
    else
        throw error("Unexpected.");
#endif
}

type_ptr type_checker::visit_func(const shared_ptr<function> & func)
{
    return make_shared<function_type>(func->vars.size());
}

type_ptr type_checker::visit_affine(const shared_ptr<affine_expr> &)
{
    throw error("Not supported.");
}

}
}
