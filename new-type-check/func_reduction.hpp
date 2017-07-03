#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/context.hpp"
#include "../frontend/error.hpp"
#include "../frontend/name_provider.hpp"
#include "../frontend/func_copy.hpp"

#include <stack>
#include <unordered_set>

namespace arrp {

using namespace stream;
using namespace stream::functional;

using std::stack;
using std::unordered_set;

class type_checker;

class func_var_sub : public rewriter_base
{
public:
    using context_type = context<var_ptr, expr_ptr>;

    func_var_sub(copier & c): m_copier(c) {}

    expr_ptr operator()(const expr_ptr & e)
    {
        return visit(e);
    }

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;

    context_type m_context;

private:
    copier & m_copier;
};

class func_reducer : public rewriter_base
{
public:
    func_reducer(name_provider &);

    unordered_set<id_ptr> & ids() { return m_ids; }

    void process(const vector<id_ptr> & ids);

private:
    void process(const id_ptr &);

    using reduce_context_type = context<var_ptr, expr_ptr>;
    using processing_id_stack_type = stack_adapter<deque<id_ptr>>;

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_func(const shared_ptr<function> & func) override;
    expr_ptr visit_func_app(const shared_ptr<func_app> & app) override;

    expr_ptr apply(expr_ptr e, const vector<expr_ptr> & args,
                   const location_type &);
    expr_ptr do_apply(shared_ptr<function>, const vector<expr_ptr> & args,
                      const location_type &);

    expr_ptr lambda_lift(expr_ptr, const string & name);

    source_error
    type_error(const string & msg, const location_type & loc)
    {
        return source_error(msg, loc, m_trace);
    }

    tracing_stack<location_type> m_trace;
    processing_id_stack_type m_processing_ids;
    stack<scope*> m_scope_stack;
    stack<array_ptr> m_array_copy_stack;

    unordered_set<id_ptr> m_processed_ids;
    unordered_set<id_ptr> m_ids;

    name_provider & m_name_provider;
    copier m_copier;
    func_var_sub m_var_sub;
};

}

