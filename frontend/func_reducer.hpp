#ifndef STREAM_LANG_FUNCTION_REDUCER_INCLUDED
#define STREAM_LANG_FUNCTION_REDUCER_INCLUDED

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/context.hpp"
#include "error.hpp"
#include "name_provider.hpp"
#include "func_copy.hpp"
#include "type_check.hpp"

#include <stack>
#include <unordered_set>

namespace stream {
namespace functional {

using std::stack;
using std::unordered_set;

class func_var_sub : public rewriter_base
{
public:
    using context_type = context<var_ptr, expr_ptr>;

    expr_ptr operator()(const expr_ptr & e)
    {
        return visit(e);
    }

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;

    context_type m_context;
};

class func_reducer : public rewriter_base
{
    // FIXME: when array vars are passed as function arguments,
    // they can propagate into other top-level expressions,
    // outside the array to which the var belongs.

public:
    func_reducer(name_provider &);

    id_ptr reduce(id_ptr, const vector<expr_ptr> & args);

    unordered_set<id_ptr> & ids() { return m_ids; }

private:
    expr_ptr reduce(expr_ptr);
    void reduce(scope & s);
    expr_ptr apply(expr_ptr e, const vector<expr_ptr> & args,
                   const location_type &);
    expr_ptr do_apply(shared_ptr<function>, const vector<expr_ptr> & args,
                      const location_type &);

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_primitive(const shared_ptr<primitive> & e) override;
    expr_ptr visit_array(const shared_ptr<array> & arr) override;
    expr_ptr visit_array_size(const shared_ptr<array_size> & as) override;
    expr_ptr visit_func(const shared_ptr<function> & func) override;
    expr_ptr visit_func_app(const shared_ptr<func_app> & app) override;


    source_error
    reduction_error(const string & msg, const location_type & loc)
    {
        return source_error(msg, loc, m_trace);
    }


    using copy_context_type = context<var_ptr, var_ptr>;
    using reduce_context_type = context<var_ptr, expr_ptr>;

    copy_context_type m_copy_context;

    stack<scope*> m_scope_stack;

    func_var_sub m_var_sub;

    tracing_stack<location_type> m_trace;
    stack<array_ptr> m_array_copy_stack;

    unordered_set<id_ptr> m_ids;

    name_provider & m_name_provider;
    copier m_copier;
    type_checker m_type_checker;
};

}
}

#endif // STREAM_LANG_FUNCTION_REDUCER_INCLUDED
