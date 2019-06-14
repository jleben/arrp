#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/context.hpp"
#include "error.hpp"
#include "name_provider.hpp"
#include "func_copy.hpp"
#include "array_bounding.hpp"
#include "linear_expr_gen.hpp"

#include <stack>
#include <unordered_set>

namespace stream {
namespace functional {

using std::stack;
using std::unordered_set;

class type_checker : public rewriter_base
{
    // FIXME: when array vars are passed as function arguments,
    // they can propagate into other top-level expressions,
    // outside the array to which the var belongs.

public:
    type_checker(name_provider &);

    unordered_set<id_ptr> & ids() { return m_ids; }

    // This may add ids into the scope:
    void process(scope &);

private:
    void process(id_ptr);
    void process_explicit_type(id_ptr);

    expr_ptr apply_external(const shared_ptr<func_app> &);

    expr_ptr visit(const expr_ptr & expr) override;
    expr_ptr visit_int(const shared_ptr<int_const> &) override;
    expr_ptr visit_real(const shared_ptr<real_const> &) override;
    expr_ptr visit_complex(const shared_ptr<complex_const> &) override;
    expr_ptr visit_bool(const shared_ptr<bool_const> &) override;
    expr_ptr visit_infinity(const shared_ptr<infinity> &) override;
    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_primitive(const shared_ptr<primitive> & e) override;
    expr_ptr visit_operation(const shared_ptr<operation> &) override;
    expr_ptr process_array_concat(const shared_ptr<operation> &);
    expr_ptr process_array_enum(const shared_ptr<operation> &);
    expr_ptr visit_cases(const shared_ptr<case_expr> & cexpr) override;
    expr_ptr visit_array(const shared_ptr<array> & arr) override;
    void process_array(const shared_ptr<array> & arr);
    expr_ptr visit_array_patterns(const shared_ptr<array_patterns> & ap) override;
    expr_ptr visit_array_self_ref(const shared_ptr<array_self_ref> &) override;
    expr_ptr visit_array_app(const shared_ptr<array_app> & app) override;
    expr_ptr visit_array_size(const shared_ptr<array_size> & as) override;
    expr_ptr visit_external(const shared_ptr<external> &) override;
    expr_ptr visit_type_name(const shared_ptr<type_name_expr> &) override;
    expr_ptr visit_array_type(const shared_ptr<array_type_expr> &) override;
    expr_ptr visit_func_type(const shared_ptr<func_type_expr> &) override;
    expr_ptr visit_func(const shared_ptr<function> & func) override;
    expr_ptr visit_func_app(const shared_ptr<func_app> & app) override;
    expr_ptr visit_scope(const shared_ptr<scope_expr> &scope) override;
    expr_ptr lambda_lift(expr_ptr, const string & name);

    source_error
    type_error(const string & msg, const location_type & loc)
    {
        return source_error(msg, loc, m_trace);
    }

    using processing_id_stack_type = stack_adapter<deque<id_ptr>>;

    tracing_stack<location_type> m_trace;
    processing_id_stack_type m_processing_ids;
    unordered_set<id_ptr> m_processed_ids;
    unordered_set<void*> m_processed_refs;

    bool m_force_revisit = false;

    unordered_set<id_ptr> m_ids;

    name_provider & m_name_provider;
    copier m_copier;
    array_bounding m_array_bounding;
    affine_integer_expression_check m_affine;
};

}
}
