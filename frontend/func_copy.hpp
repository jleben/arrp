#ifndef STREAM_LANG_FUNCTIONAL_MODEL_COPY_INCLUDED
#define STREAM_LANG_FUNCTIONAL_MODEL_COPY_INCLUDED

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/stacker.hpp"
#include "../utility/context.hpp"
#include "name_provider.hpp"

#include <unordered_set>
#include <stack>

namespace stream {
namespace functional {

using std::unordered_set;
using std::stack;

class copier : public visitor<expr_ptr>
{
public:
    copier(unordered_set<id_ptr> & ids, name_provider &);
    expr_slot copy(const expr_slot & slot)
    {
        return expr_slot(copy(slot.expr), slot.location);
    }
    expr_ptr copy(const expr_ptr & expr)
    {
        if (expr)
            return visit(expr);
        else
            return expr_ptr();
    }

protected:
    virtual expr_ptr visit_int(const shared_ptr<int_const> &) override;
    virtual expr_ptr visit_real(const shared_ptr<real_const> &) override;
    virtual expr_ptr visit_complex(const shared_ptr<complex_const> &) override;
    virtual expr_ptr visit_bool(const shared_ptr<bool_const> &) override;
    virtual expr_ptr visit_infinity(const shared_ptr<infinity> &) override;
    virtual expr_ptr visit_ref(const shared_ptr<reference> &) override;
    virtual expr_ptr visit_array_self_ref(const shared_ptr<array_self_ref> &) override;
    virtual expr_ptr visit_primitive(const shared_ptr<primitive> & prim) override;
    virtual expr_ptr visit_operation(const shared_ptr<operation> &) override;
    virtual expr_ptr visit_affine(const shared_ptr<affine_expr> &) override;
    virtual expr_ptr visit_cases(const shared_ptr<case_expr> & cexpr) override;
    virtual expr_ptr visit_array(const shared_ptr<array> & arr) override;
    virtual expr_ptr visit_array_patterns(const shared_ptr<array_patterns> & ap) override;
    virtual expr_ptr visit_array_app(const shared_ptr<array_app> & app) override;
    virtual expr_ptr visit_array_size(const shared_ptr<array_size> & as) override;
    virtual expr_ptr visit_func_app(const shared_ptr<func_app> & app) override;
    virtual expr_ptr visit_func(const shared_ptr<function> & func) override;
    virtual expr_ptr visit_external(const shared_ptr<external> &) override;
    virtual expr_ptr visit_type_name(const shared_ptr<type_name_expr> &) override;
    virtual expr_ptr visit_array_type(const shared_ptr<array_type_expr> &) override;
    virtual expr_ptr visit_func_type(const shared_ptr<func_type_expr> &) override;
    virtual expr_ptr visit_scope(const shared_ptr<scope_expr> &) override;

private:
    using context_type = context<var_ptr, var_ptr>;
    context_type m_copy_context;

    stack<array_ptr> m_array_copy_stack;

    unordered_set<id_ptr> & m_ids;

    name_provider & m_name_provider;
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_MODEL_COPY_INCLUDED
