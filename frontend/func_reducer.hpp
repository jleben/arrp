#ifndef STREAM_LANG_FUNCTION_REDUCER_INCLUDED
#define STREAM_LANG_FUNCTION_REDUCER_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"

namespace stream {
namespace functional {

class func_reducer
{
public:
    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args,
                   const location_type &);
private:
    expr_ptr reduce(expr_ptr);
    expr_ptr copy(expr_ptr);

    using copy_context_type = context<var_ptr, var_ptr>;
    using reduce_context_type = context<var_ptr, expr_ptr>;

    copy_context_type m_copy_context;
    reduce_context_type m_reduce_context;
};

}
}

#endif // STREAM_LANG_FUNCTION_REDUCER_INCLUDED
