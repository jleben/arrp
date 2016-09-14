#ifndef STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
#define STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED

#include "error.hpp"
#include "array_bounding.hpp"
#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../common/func_model_printer.hpp"

#include <unordered_set>

namespace stream {
namespace functional {

using std::unordered_set;

class type_checker : public visitor<type_ptr>
{
public:
    type_checker(stack<location_type> & trace);

    type_ptr process(const id_ptr &);
    void process(const expr_ptr &);

    static type_ptr make_array_type(array_size_vec & size, const type_ptr & elem_type);

private:
    type_ptr visit(const expr_ptr & expr) override;
    type_ptr visit_int(const shared_ptr<int_const> &) override;
    type_ptr visit_real(const shared_ptr<real_const> &) override;
    type_ptr visit_complex(const shared_ptr<complex_const> &) override;
    type_ptr visit_bool(const shared_ptr<bool_const> &) override;
    type_ptr visit_infinity(const shared_ptr<infinity> &) override;
    type_ptr visit_ref(const shared_ptr<reference> &) override;
    type_ptr visit_primitive(const shared_ptr<primitive> & prim) override;
    type_ptr visit_operation(const shared_ptr<operation> &) override;
    type_ptr process_array_concat(const shared_ptr<operation> &);
    type_ptr process_array_enum(const shared_ptr<operation> &);
    type_ptr visit_affine(const shared_ptr<affine_expr> &) override;
    type_ptr visit_cases(const shared_ptr<case_expr> & cexpr) override;
    type_ptr visit_array(const shared_ptr<array> & arr) override;
    type_ptr process_array(const shared_ptr<array> & arr);
    type_ptr visit_array_patterns(const shared_ptr<array_patterns> & ap) override;
    type_ptr visit_array_self_ref(const shared_ptr<array_self_ref> &) override;
    type_ptr visit_array_app(const shared_ptr<array_app> & app) override;
    type_ptr visit_array_size(const shared_ptr<array_size> & as) override;
    type_ptr visit_func_app(const shared_ptr<func_app> & app) override;
    type_ptr visit_func(const shared_ptr<function> & func) override;
    type_ptr visit_input(const shared_ptr<input> & in) override;

    source_error type_error(const string & msg, const location_type & loc)
    {
        return source_error(msg, loc, m_trace);
    }

    bool m_force_revisit = false;

    printer m_printer;
    array_bounding m_array_bounding;

    stack<location_type> & m_trace;
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
