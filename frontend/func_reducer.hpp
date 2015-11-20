#ifndef STREAM_LANG_FUNCTION_REDUCER_INCLUDED
#define STREAM_LANG_FUNCTION_REDUCER_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"
#include "error.hpp"

#include <stack>

namespace stream {
namespace functional {

using std::stack;

class func_reduce_error : public source_error
{
public:
    func_reduce_error(const string & msg,
                      const stack<location_type> & trace,
                      const location_type & loc):
        source_error(msg, loc), trace(trace) {}
    stack<location_type> trace;
};

class func_reducer
{
public:
    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args,
                   const location_type &);
private:
    expr_ptr reduce(expr_ptr);
    expr_ptr copy(expr_ptr);
    expr_ptr no_function(expr_ptr);
    expr_ptr no_function(expr_ptr, const location_type &);
    func_reduce_error
    reduction_error(const string & msg, const location_type & loc)
    {
        return func_reduce_error(msg, m_trace, loc);
    }

    using copy_context_type = context<var_ptr, var_ptr>;
    using reduce_context_type = context<var_ptr, expr_ptr>;

    copy_context_type m_copy_context;
    reduce_context_type m_reduce_context;

    stack<location_type> m_trace;
};

}
}

#endif // STREAM_LANG_FUNCTION_REDUCER_INCLUDED
