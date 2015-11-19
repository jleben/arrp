#ifndef STREAM_LANG_FUNCTION_REDUCER_INCLUDED
#define STREAM_LANG_FUNCTION_REDUCER_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"

namespace stream {
namespace functional {

class func_reducer
{
public:
    func_def_ptr reduce(func_def_ptr, const vector<expr_ptr> & args,
                        const location_type &);
private:
    expr_ptr reduce(expr_ptr);

    using context_type = context<func_var_ptr, expr_ptr>;
    using func_context_type = context<func_def_ptr, func_def_ptr>;
    using array_context_type = context<array_var_ptr, array_var_ptr>;

    context_type m_context;
    func_context_type m_func_context;
    array_context_type m_array_context;
};

}
}

#endif // STREAM_LANG_FUNCTION_REDUCER_INCLUDED
