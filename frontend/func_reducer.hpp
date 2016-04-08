#ifndef STREAM_LANG_FUNCTION_REDUCER_INCLUDED
#define STREAM_LANG_FUNCTION_REDUCER_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"
#include "error.hpp"

#include <stack>
#include <unordered_set>

namespace stream {
namespace functional {

using std::stack;
using std::unordered_set;

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
    // FIXME: when array vars are passed as function arguments,
    // they can propagate into other top-level expressions,
    // outside the array to which the var belongs.

public:
    id_ptr reduce(id_ptr, const vector<expr_ptr> & args);

    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args,
                   const location_type &);

    unordered_set<id_ptr> & ids() { return m_ids; }

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

    reduce_context_type m_beta_reduce_context;
    stack<int> m_bound_var_count;

    stack<location_type> m_trace;
    stack<array_ptr> m_array_copy_stack;

    unordered_set<id_ptr> m_ids;

    string new_id_name(const string & base);
    unordered_map<string,int> id_counts;
};

}
}

#endif // STREAM_LANG_FUNCTION_REDUCER_INCLUDED
