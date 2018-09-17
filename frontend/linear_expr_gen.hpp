#ifndef STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED
#define STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED

#include "../common/functional_model.hpp"
#include "../common/linear_algebra.hpp"
#include "../common/func_model_visitor.hpp"
#include "func_copy.hpp"
#include "name_provider.hpp"

#include <tuple>

namespace stream {
namespace functional {

using std::tuple;

linexpr to_linear_expr(expr_ptr);

void ensure_affine_integer_expression(expr_ptr);
void ensure_affine_integer_constraint(expr_ptr);

linexpr maximum(const linexpr &);

class affine_integer_expression_check
{
public:
    affine_integer_expression_check(copier & c): m_copier(c) {}

    expr_ptr ensure_contraint(expr_ptr);
    expr_ptr ensure_expression(expr_ptr);

private:
    enum affine_expr_type
    {
        affine_constant,
        affine_variable
    };

    tuple<expr_ptr, affine_expr_type> ensure_expression_get_type(expr_ptr);

    copier & m_copier;
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED
