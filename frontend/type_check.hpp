#ifndef STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
#define STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"

#include <unordered_set>

namespace stream {
namespace functional {

using std::unordered_set;

class type_checker : public visitor<type_ptr>
{
public:
    void process(const expr_ptr &);

    static type_ptr make_array_type(array_size_vec & size, const type_ptr & elem_type);

private:
    type_ptr visit(const expr_ptr & expr);
    type_ptr visit_int(const shared_ptr<constant<int>> &);
    type_ptr visit_double(const shared_ptr<constant<double>> &);
    type_ptr visit_bool(const shared_ptr<constant<bool>> &);
    type_ptr visit_ref(const shared_ptr<reference> &);
    type_ptr visit_primitive(const shared_ptr<primitive> & prim);
    type_ptr visit_affine(const shared_ptr<affine_expr> &);
    type_ptr visit_cases(const shared_ptr<case_expr> & cexpr);
    type_ptr visit_array(const shared_ptr<array> & arr);
    type_ptr process_array(const shared_ptr<array> & arr);
    type_ptr visit_array_self_ref(const shared_ptr<array_self_ref> &);
    type_ptr visit_array_app(const shared_ptr<array_app> & app);
    type_ptr visit_array_size(const shared_ptr<array_size> & as);
    type_ptr visit_func_app(const shared_ptr<func_app> & app);
    type_ptr visit_func(const shared_ptr<function> & func);

    bool m_force_revisit = false;
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
