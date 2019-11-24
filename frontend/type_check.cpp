#include "type_check.hpp"
#include "prim_reduction.hpp"
#include "linear_expr_gen.hpp"
#include "error.hpp"
#include "array_to_isl.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/stacker.hpp"
#include "../utility/debug.hpp"

#include <isl-cpp/constraint.hpp>

#include <iostream>
#include <sstream>

using namespace std;

namespace stream {
namespace functional {

static bool is_constant(expr_ptr expr)
{
    if (dynamic_cast<int_const*>(expr.get()))
        return true;
    if (dynamic_cast<real_const*>(expr.get()))
        return true;
    if (dynamic_cast<bool_const*>(expr.get()))
        return true;
    return false;
}

string wrong_arg_count_msg(int required, int actual)
{
    ostringstream text;
    text << " Wrong number of arguments ("
         << "expected: " << required << ", "
         << "actual: " << actual
         << ")."
            ;
    return text.str();
}

class primitive_expr_check : private visitor<bool>
{
public:
    bool operator()(const expr_ptr & e)
    {
        printer p;
        //cout << "Checking if expr is primitive: ";
        //p.print(e, cout);
        //cout << endl;
        bool is_primitive = visit(e);
        //cout << "  -> " << is_primitive << endl;
        return is_primitive;
    }

protected:
    virtual bool visit_int(const shared_ptr<int_const> &) override { return true; }
    virtual bool visit_real(const shared_ptr<real_const> &) override { return true; }
    virtual bool visit_complex(const shared_ptr<complex_const> &) override { return true; }
    virtual bool visit_bool(const shared_ptr<bool_const> &) override { return true; }
    virtual bool visit_infinity(const shared_ptr<infinity> &) override { return true; }
    virtual bool visit_ref(const shared_ptr<reference> &) override { return true; }
    virtual bool visit_array_self_ref(const shared_ptr<array_self_ref> &) override { return true; }
    virtual bool visit_primitive(const shared_ptr<primitive> & prim) override
    {
        for (auto & arg : prim->operands)
        {
            if (!arg->type->is_scalar())
                return false;
        }
        return true;
    }
    virtual bool visit_affine(const shared_ptr<affine_expr> &) override { return true; }
    virtual bool visit_array_app(const shared_ptr<array_app> & app) override {
        return visit(app->object);
    }
    virtual bool visit_array_size(const shared_ptr<array_size> & as) override { return true; }
};

class atomic_expr_check : public primitive_expr_check
{
protected:
    virtual bool visit_primitive(const shared_ptr<primitive> & prim) override
    { return false; }
    virtual bool visit_affine(const shared_ptr<affine_expr> & prim) override
    { return false; }
};


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

type_checker::type_checker(name_provider & nmp):
    m_trace("trace"),
    m_name_provider(nmp),
    m_copier(m_ids, nmp),
    m_affine(m_copier)
{
    m_trace.set_enabled(false);
}

void type_checker::process(scope & sc)
{
    int pass_count = 3;
    for (m_pass = 1; m_pass <= pass_count; ++m_pass)
    {
        if (verbose<type_checker>::enabled())
            cout << "--- Type check pass " << m_pass << " ---" << endl;

        m_processed_ids.clear();

        for (auto & id : sc.ids)
            process(id);
    }
}

void type_checker::assign(expr_ptr e, type_ptr t)
{
    if (m_pass < 3)
    {
        e->type = t;
    }
    else
    {
        if (!e->type || e->type->is_undefined())
        {
            throw type_error("Type could not be completely inferred.", e->location);
        }

        if (*e->type != *t)
        {
            throw type_error("No type satisfying the constraints.", e->location);
        }
    }
}

void type_checker::process(id_ptr id)
{
    if (m_processed_ids.count(id))
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "Id already processed: " << id->name << endl;
        }
        return;
    }

    bool is_visiting =
            std::find(m_processing_ids.begin(), m_processing_ids.end(), id)
            != m_processing_ids.end();

    if (is_visiting)
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "Recursion at id: " << id->name << endl;
        }
        return;
    }

    process_explicit_type(id);

    if (!id->expr)
    {
        if (verbose<type_checker>::enabled())
            cout << "Id has no expression: " << id->name << endl;
        return;
    }

    if (verbose<type_checker>::enabled())
    {
        cout << "Processing id " << id->name << endl;
    }

    auto processing_id_token = stack_scoped(id, m_processing_ids);

    id->expr = visit(id->expr);

    m_processed_ids.insert(id);

    if (m_pass >= 3 && id->explicit_type && *id->explicit_type != *id->expr->type)
    {
        ostringstream msg;
        msg << "Explicit type "
            << *id->explicit_type
            << " does not match expression type "
            << *id->expr->type
            << ".";
        throw type_error(msg.str(), id->location);
    }

    auto t = id->expr->type;

    if (!t->is_function() && !t->is_data())
    {
        throw type_error("Expression can not be bound to name.",
                              id->expr.location);
    }

    if (verbose<type_checker>::enabled())
    {
        cout << "Inferred type: " << id->name
             << " :: " << *t
             << endl;
    }

    m_ids.insert(id);
}

void type_checker::process_explicit_type(id_ptr id)
{
    if (id->explicit_type || !id->type_expr)
        return;

    if (verbose<type_checker>::enabled())
    {
        cout << "Processing explicit type for id " << id->name << endl;
    }
#if 0
    bool is_visiting =
            std::find(m_processing_ids.begin(), m_processing_ids.end(), id)
            != m_processing_ids.end();

    if (is_visiting)
    {
        if (verbose<type_checker>::enabled())
        {
            cout << "Recursion at id: " << id->name << endl;
        }
        return;
    }

    auto processing_id_token = stack_scoped(id, m_processing_ids);
#endif
    id->type_expr = visit(id->type_expr);

    auto meta_type = id->type_expr->type;
    if (meta_type->is_meta())
    {
        id->explicit_type = meta_type->meta()->concrete;
    }
    else
    {
        throw type_error("Not a type expression.", id->type_expr.location);
    }

    if (verbose<type_checker>::enabled())
    {
        cout << "Explicit type: " << id->name
             << " :: " << *id->explicit_type
             << endl;
    }
}

expr_ptr type_checker::visit(const expr_ptr & expr)
{
    // FIXME: Is this OK?
    if (!expr)
        return expr;

    if (false)
    {
        auto p = expr.get();
        if (m_processed_refs.count(p))
        {
            throw error("Double processing.");
        }
        m_processed_refs.insert(p);
    }

    return visitor<expr_ptr>::visit(expr);
#if 0
    if (expr && (m_force_revisit || !expr->type))
        return visitor<expr_ptr>::visit(expr);

    return expr;
#endif
}

expr_ptr type_checker::visit_int(const shared_ptr<int_const> & expr)
{
    assign(expr, make_shared<scalar_type>(primitive_type::integer));
    return expr;
}

expr_ptr type_checker::visit_real(const shared_ptr<real_const> & expr)
{
    assign(expr, make_shared<scalar_type>(primitive_type::real64));
    return expr;
}

expr_ptr type_checker::visit_complex(const shared_ptr<complex_const> & expr)
{
    assign(expr, make_shared<scalar_type>(primitive_type::complex64));
    return expr;
}

expr_ptr type_checker::visit_bool(const shared_ptr<bool_const> & expr)
{
    assign(expr, make_shared<scalar_type>(primitive_type::boolean));
    return expr;
}

expr_ptr type_checker::visit_infinity(const shared_ptr<infinity> & expr)
{
    assign(expr, type::infinity());
    return expr;
}

expr_ptr type_checker::visit_ref(const shared_ptr<reference> & ref)
{
    printer p;

    if (auto id = dynamic_pointer_cast<identifier>(ref->var))
    {
        if (!id->expr)
        {
            throw source_error("Name '" + id->name + "' is used, but has no defined value.",
                               ref->location);
        }

        if (verbose<type_checker>::enabled())
        {
            cout << "Referenced id: " << id->name << endl;
        }

        process(id);

        if (id->explicit_type)
        {
            assign(ref, id->explicit_type);
        }
        else
        {
            if (id->expr->type)
            {
                assign(ref, id->expr->type);
            }
            else if (id->is_recursive)
            {
                assign(ref, make_shared<scalar_type>(primitive_type::undefined));
            }
            else
            {
                throw error("Id has no inferred or explicit type.");
            }
        }

        if (verbose<type_checker>::enabled())
        {
            cout <<  "Using this type for referenced id: "
                  << id->name << " :: " << *ref->type
                  << endl;
        }

        if (is_constant(id->expr))
        {
            if (verbose<type_checker>::enabled())
            {
                cout << "Reference to id " << id->name
                     << " is constant - using the value instead."
                     << endl;
            }
            return id->expr;
        }

        m_ids.insert(id);

        return ref;
    }
    else if (auto avar = dynamic_pointer_cast<array_var>(ref->var))
    {
        assign(ref, make_shared<scalar_type>(primitive_type::integer));
        return ref;
    }
    else
    {
        throw error("Function variable was not substituted.");
    }
}

expr_ptr type_checker::visit_primitive(const shared_ptr<primitive> & prim)
{
    // FIXME: This seems redundant. Maybe it was useful when type checker did beta reduction...
#if 0
    auto prim = dynamic_pointer_cast<primitive>(rewriter_base::visit_primitive(source_prim));
    assert(prim);
#endif

    for (auto & operand : prim->operands)
    {
        operand = visit(operand);
    }

    {
        auto reduced_prim = reduce_primitive(prim);

        if (reduced_prim != prim)
        {
            return visit(reduced_prim);
        }
    }

    array_size_vec common_size;
    vector<primitive_type> elem_types;

    for (auto & operand : prim->operands)
    {
        if (operand->type->is_function())
        {
            throw type_error("Function not allowed as operand.", operand.location);
        }
        else if (auto arr = dynamic_pointer_cast<array_type>(operand->type))
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
        else if (auto scalar = dynamic_pointer_cast<scalar_type>(operand->type))
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
        assign(prim, make_shared<scalar_type>(result_elem_type));
    }
    else
    {
        assign(prim, make_shared<array_type>(common_size, result_elem_type));
    }

    return prim;
}

expr_ptr type_checker::visit_operation(const shared_ptr<operation> & op)
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

expr_ptr type_checker::process_array_concat(const shared_ptr<operation> & op)
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

        operand = visit(operand);

        auto type = operand->type;

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

    assign(op, make_shared<array_type>(result_size, result_elem_type));

    return op;
}

expr_ptr type_checker::process_array_enum(const shared_ptr<operation> & op)
{
    array_size_vec common_elem_size;
    vector<primitive_type> elem_types;

    for (auto & operand : op->operands)
    {
        operand = visit(operand);

        auto type = operand->type;

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

    assign(op, make_shared<array_type>(result_size, result_elem_type));

    return op;
}

expr_ptr type_checker::visit_cases(const shared_ptr<case_expr> & cexpr)
{
    array_size_vec common_size;
    vector<primitive_type> elem_types;

    for (auto & c : cexpr->cases)
    {
        auto & domain = c.first;
        auto & expr = c.second;

        domain = visit(domain);
        domain = m_affine.ensure_contraint(domain);

        expr = visit(expr);

        auto type = expr->type;

        if (!type->is_data())
        {
            throw type_error("Expression can not be used as data.", expr.location);
        }

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
        assign(cexpr, make_shared<scalar_type>(result_elem_type));
    }
    else
    {
        assign(cexpr, make_shared<array_type>(common_size, result_elem_type));
    }

    return cexpr;
}

expr_ptr type_checker::visit_array(const shared_ptr<array> & arr)
{
    // FIXME: var range is also visited in process_array. Remove this?
    for (auto & var : arr->vars)
    {
        var->range = visit(var->range);
    }

    {
        process_array(arr);
#if 0
        if (arr->is_recursive)
        {
            if (verbose<type_checker>::enabled())
                cout << mention(arr->location)
                     << "Revisiting recursive array." << endl;

            // We need another pass with proper type for self-references.
            revertable<bool> revisit(m_force_revisit, true);
            process_array(arr);

            assert(arr->type->is_array());
            if (arr->type->array()->element == primitive_type::undefined)
            {
                throw type_error("Array type can not be inferred.",
                                      arr->location);
            }
        }
#endif
    }

    return arr;
}

void type_checker::process_array(const shared_ptr<array> & arr)
{
    auto var_stacker = stacker<array_var_ptr, array_var_stack_type>(m_array_var_stack);

    for (auto & var : arr->vars)
    {
        var_stacker.push(var);
    }

    check_array_variables(arr);

    // Create ISL model of variable bounds (including variables in context)

    //isl::printer isl_printer(m_isl_ctx);

    auto isl_context_domain = array_context_domain(arr);
    auto isl_domain = isl_context_domain & array_var_bounds(arr);
    auto isl_space = isl_domain.get_space();
    int context_var_count = m_array_var_stack.size() - arr->vars.size();

    // Process definitions of array values

    vector<array_size_vec> elem_sizes;
    vector<primitive_type> elem_types;

    space_map isl_space_map(isl_space, vector<array_var_ptr>(m_array_var_stack.begin(), m_array_var_stack.end()));
    auto isl_array_def_domain = isl::set(isl_space);

    auto patterns = dynamic_pointer_cast<array_patterns>(arr->expr.expr);
    for (auto & pattern : patterns->patterns)
    {
        auto isl_pattern_domain = isl_domain;

        for (int i = 0; i < pattern.indexes.size(); ++i)
        {
            auto & index = pattern.indexes[i];
            if (index.is_fixed)
            {
                isl_pattern_domain.add_constraint(isl_space.var(i + context_var_count) == index.value);
            }
        }

#if 0
        cout << "Pattern domain: ";
        isl_printer.print(isl_pattern_domain);
        cout << endl;
#endif
        if (pattern.domains)
        {
            auto cexpr = dynamic_pointer_cast<case_expr>(pattern.domains.expr);

            for (auto & c : cexpr->cases)
            {
                auto & domain = c.first;
                auto & expr = c.second;

                domain = visit(domain);
                domain = m_affine.ensure_contraint(domain);

                auto isl_guard_domain = to_affine_set(domain, isl_space_map);
                auto isl_expr_domain = (isl_pattern_domain & isl_guard_domain) - isl_array_def_domain;

#if 0
                cout << "Expression domain: ";
                isl_printer.print(isl_expr_domain);
                cout << endl;
#endif
                {
                    auto stacked_domain = stack_scoped(isl_expr_domain, m_isl_array_domain_stack);
                    process_array_value(expr, elem_sizes, elem_types);
                }

                isl_array_def_domain |= isl_expr_domain;
            }
        }
#if 0
        if (pattern.expr)
        {
            auto isl_expr_domain = isl_pattern_domain - isl_array_def_domain;

#if 0
            cout << "Expression domain: ";
            isl_printer.print(isl_expr_domain);
            cout << endl;
#endif
            {
                auto stacked_domain = stack_scoped(isl_expr_domain, m_isl_array_domain_stack);
                process_array_value(pattern.expr, elem_sizes, elem_types);
            }

            isl_array_def_domain |= isl_pattern_domain;
        }
#endif
    }

#if 0
    cout << "Union of array expression domains: ";
    isl_printer.print(isl_array_def_domain);
    cout << endl;
#endif

    // Compute result type

    array_size_vec common_elem_size;

    for (auto & size : elem_sizes)
    {
        try {
            common_elem_size = common_array_size(common_elem_size, size);
        } catch (no_type &) {
            throw type_error("Incompatible element sizes.", arr->location);
        }
    }

    primitive_type common_elem_type;

    try {
        common_elem_type = common_type(elem_types);
    } catch (no_type &) {
        throw type_error("Incompatible element types.", arr->location);
    }

    assign(patterns, type_for(common_elem_size, common_elem_type));

    // Make sure defined array domain is rectangular and independent of the context,
    // and store its bounds as range of array variables...

    if (isl_array_def_domain.is_empty())
    {
        throw type_error("Inferred array domain is empty.", arr->location);
    }

    // For each array variable, if it has no explicit bound,
    // assign one as the maximum value within known constraints.
    for (int i = 0; i < arr->vars.size(); ++i)
    {
        auto & var = arr->vars[i];

        if (!var->range)
        {
            isl::value max_index(nullptr);
            try { max_index = isl_array_def_domain.maximum(isl_space.var(i + context_var_count)); }
            catch (isl::error &)
            {
                throw error("Failed to infer array variable bound.");
            }

            if (max_index.is_integer())
            {
                var->range = make_shared<int_const>(max_index.integer() + 1);
            }
            else if (max_index.is_pos_infinity())
            {
                var->range = make_shared<infinity>();
            }
            else
            {
                throw error("Failed to infer array variable bound.");
            }
        }
    }

    // Extract array size from range of array variables

    array_size_vec size;

    for (auto & var : arr->vars)
    {
        if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
            size.push_back(c->value);
        else
            size.push_back(-1);
    }

    // Make sure domain is rectangular and independent of the context.
    // This is somewhat complicated because index constraints may include
    // variables from enclosing arrays.
    // Solution:
    // Let context constraints be C_ctx,
    // Let C be local constraints
    // Let C_max be rectangular hull of local constraints (max range for each variable)
    // Check if C_Ctx & C = C_ctx & C_max

    {
        auto rect_domain = isl::set::universe(isl_space);
        for (int i = 0; i < size.size(); ++i)
        {
            int j = i + context_var_count;
            rect_domain.add_constraint(isl_space.var(j) >= 0);
            if (size[i] >= 0)
                rect_domain.add_constraint(isl_space.var(j) < int(size[i]));
        }

        if (not ((isl_context_domain & rect_domain) == isl_array_def_domain))
        {
            throw type_error("Part of array domain is undefined.", arr->location);
        }
    }

    // Expand array space with common space of subdomains

    size.insert(size.end(), common_elem_size.begin(), common_elem_size.end());

    assign(arr, make_shared<array_type>(size, common_elem_type));
}

void type_checker::check_array_variables(const shared_ptr<array> & arr)
{
    for (auto & var : arr->vars)
    {
        if (!var->range)
        {
            continue;
        }

        var->range = visit(var->range);

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
}

void type_checker::process_array_value
(expr_slot & e, vector<array_size_vec> & elem_sizes, vector<primitive_type> & elem_types)
{
    e = visit(e);

    const auto & type = e->type;

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
        elem_sizes.push_back(at->size);
        elem_types.push_back(at->element);
    }
    else if (auto st = dynamic_pointer_cast<scalar_type>(type))
    {
        elem_sizes.push_back(array_size_vec());
        elem_types.push_back(st->primitive);
    }
    else
    {
        throw error("Unexpected type.");
    }
}

isl::set type_checker::array_context_domain(const shared_ptr<array> & arr)
{
    if (m_isl_array_domain_stack.empty())
    {
        isl::space space(m_isl_ctx, isl::set_tuple(arr->vars.size()));
        return isl::set::universe(space);
    }
    else
    {
        auto context = m_isl_array_domain_stack.top();
        context.add_dimensions(isl::space::variable, arr->vars.size());
        return context;
    }
}

isl::set type_checker::array_var_bounds(const shared_ptr<array> & arr)
{
    isl::space space(m_isl_ctx, isl::set_tuple(m_array_var_stack.size()));
    auto domain = isl::set::universe(space);

    int context_size = m_array_var_stack.size() - arr->vars.size();

    // Add explicit bounds:
    for (int i = 0; i < arr->vars.size(); ++i)
    {
        auto & var = arr->vars[i];

        int j = context_size + i;

        domain.add_constraint(space.var(j) >= 0);

        if (auto c = dynamic_pointer_cast<constant<int>>(var->range.expr))
        {
            domain.add_constraint(space.var(j) < int(c->value));
        }
    }

    return domain;
}


expr_ptr type_checker::visit_array_patterns(const shared_ptr<array_patterns> &)
{
    throw error("Unexpected.");
}

expr_ptr type_checker::visit_array_self_ref(const shared_ptr<array_self_ref> & self)
{
    vector<int> size;

    auto & arr = self->arr;

    if (arr->type)
    {
        if (verbose<type_checker>::enabled())
            cout << mention(self->location)
                 << "Self reference using known array type: "
                 << *arr->type << endl;
        assign(self, arr->type);
        return self;
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

    assign(self, result);
    return self;
}

expr_ptr type_checker::visit_array_app(const shared_ptr<array_app> & app)
{
    app->object = visit(app->object);

    auto object_type = app->object->type;
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

        arg = visit(arg);

        auto arg_type = dynamic_pointer_cast<scalar_type>(arg->type);

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

        try
        {
            // If it is an affine expression, make sure it's flattened
            // (identifiers substituted with their values)
            arg = m_affine.ensure_expression(arg);
        }
        catch(source_error &)
        {
            // No problem if it's not an affine expression
            // (we will overestimate the range later).
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
                throw reduction_error(msg.str(),
                                   arg.location);
            }
        }
        else
        {
            if (bound != array_var::unconstrained)
            {
                throw reduction_error("Unbounded argument to"
                                   " bounded array dimension.",
                                   arg.location);
            }
        }
#endif
    }

    if (object_size.size() <= app->args.size())
    {
        assign(app, make_shared<scalar_type>(elem_type));
    }
    else
    {
        vector<int> remaining_size(object_size.begin() + app->args.size(),
                                   object_size.end());
        assign(app, make_shared<array_type>(remaining_size, elem_type));
    }

    return app;
}



expr_ptr type_checker::visit_array_size(const shared_ptr<array_size> & as)
{
    as->object = visit(as->object);
    if (as->dimension)
        as->dimension = visit(as->dimension);

    if (as->object->type->is_scalar())
    {
        return make_shared<int_const>(1, location_type());
    }

    if (!as->object->type->is_array())
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
        return make_shared<int_const>(size, as->location);
    else
        return make_shared<infinity>(as->location);
}

expr_ptr type_checker::visit_external(const shared_ptr<external> & ext)
{
    ext->type_expr = visit(ext->type_expr);

    auto meta_type = ext->type_expr->type;

    if (!meta_type->is_meta())
    {
        throw type_error("Expression is not a type.", ext->type_expr.location);
    }

    auto type = meta_type->meta()->concrete;

    if (ext->is_input)
    {
        if (type->is_function())
        {
            throw type_error("Type of input must not be a function.",
                               ext->type_expr.location);
        }
    }
    else
    {
        if (!type->is_function())
        {
            throw type_error("Type of external must be a function.",
                               ext->type_expr.location);
        }

        vector<type_ptr> types = type->func()->params;
        types.push_back(type->func()->value);

        for (auto & t : types)
        {
            if (t->is_array() && t->array()->is_infinite())
            {
                throw type_error("Type of external must not involve infinite arrays.",
                                   ext->type_expr.location);
            }
            else if (t->is_function())
            {
                throw type_error("Parameters or result of external must not contain functions.",
                                   ext->type_expr.location);
            }
        }
    }

    assign(ext, type);

    return ext;
}


expr_ptr type_checker::visit_type_name(const shared_ptr<type_name_expr> & e)
{
    auto pt = primitive_type_for_name(e->name);
    if (pt == primitive_type::undefined)
        throw type_error("Invalid type name.", e->location);

    auto ct = make_shared<scalar_type>(pt);

    e->type = make_shared<meta_type>(ct);

    return e;
}

expr_ptr type_checker::visit_array_type(const shared_ptr<array_type_expr> & e)
{
    vector<int> size;

    for (auto & size_expr : e->size)
    {
        size_expr = visit(size_expr);

        if (auto ci = dynamic_pointer_cast<int_const>(size_expr.expr))
        {
            auto v = ci->value;
            if (v < 0)
                throw type_error("Array size must be positive.", size_expr.location);
            size.push_back(v);
        }
        else if (auto i = dynamic_pointer_cast<infinity>(size_expr.expr))
        {
            size.push_back(-1);
        }
        else
        {
            throw type_error("Invalid array size.", size_expr.location);
        }
    }

    auto element_type = primitive_type_for_name(e->element);
    if (element_type == primitive_type::undefined)
    {
        throw type_error("Invalid array element type.", e->location);
    }

    auto at = make_shared<array_type>(size, element_type);

    e->type = make_shared<meta_type>(at);

    return e;
}

expr_ptr type_checker::visit_func_type(const shared_ptr<func_type_expr> & e)
{
    auto ft = make_shared<function_type>();

    for (auto & param : e->params)
    {
        param = visit(param);

        if (param->type->is_meta())
        {
            ft->params.push_back(param->type->meta()->concrete);
        }
        else
        {
            throw type_error("Expression is not a type.", param.location);
        }
    }

    e->result = visit(e->result);

    auto rt = e->result->type;
    if (rt->is_meta())
    {
        ft->value = rt->meta()->concrete;
    }
    else
    {
        throw type_error("Expression is not a type.", e->result.location);
    }

    e->type = make_shared<meta_type>(ft);

    return e;
}

expr_ptr type_checker::visit_func(const shared_ptr<function> & func)
{
    throw source_error("Unexpected function.", func->location);
}

expr_ptr type_checker::visit_func_app(const shared_ptr<func_app> & app)
{
    app->object = visit(app->object);

    auto func_type = dynamic_pointer_cast<function_type>(app->object->type);
    if (!func_type)
        throw type_error("Not a function.", app->object.location);

    auto & args = app->args;

    for (auto & arg : args)
    {
        arg = visit(arg);
    }

    if (args.size() != func_type->param_count())
    {
        ostringstream msg;
        msg << "Wrong number of arguments in function application: "
            << func_type->param_count() << " expected, "
            << args.size() << " given.";
        throw type_error(msg.str(), app->location);
    }

    for (int p = 0; p < func_type->param_count(); ++p)
    {
        assert(func_type->params[p]);
        const auto & pt = *func_type->params[p];
        const auto & at = *args[p]->type;
        // FIXME: I'm not sure the following is good for external.
        // An external may accept a scalar subtype, but not an array subtype.
        bool ok = at <= pt;
        if (!ok)
        {
            ostringstream msg;
            msg << "Argument type mismatch:"
                << " expected " << pt
                << " but given " << at;
            throw type_error(msg.str(), args[p].location);
        }
    }

    assert(func_type->value);
    assign(app, func_type->value);

    return app;
}

expr_ptr type_checker::visit_scope(const shared_ptr<scope_expr> &scope)
{
    for (auto & id : scope->local.ids)
        process(id);

    scope->value = visit(scope->value);

    assign(scope, scope->value->type);
    return scope;
}

}
}
