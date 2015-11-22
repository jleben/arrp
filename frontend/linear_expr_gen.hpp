#ifndef STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED
#define STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED

#include "../common/functional_model.hpp"
#include "../common/linear_algebra.hpp"

namespace stream {
namespace functional {

linexpr to_linear_expr(expr_ptr);

linexpr maximum(const linexpr &);

}
}

#endif // STREAM_LANG_FUNCTIONAL_LINEAR_EXPR_GEN_INCLUDED
