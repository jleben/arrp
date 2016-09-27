#ifndef STREAM_LANG_ARRAY_REDUCTION_INCLUDED
#define STREAM_LANG_ARRAY_REDUCTION_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"
#include "../utility/stacker.hpp"
#include "../common/func_model_printer.hpp"
#include "func_copy.hpp"
#include "name_provider.hpp"

#include <isl-cpp/context.hpp>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <deque>

namespace stream {
namespace functional {

using std::unordered_set;
using std::unordered_map;
using std::stack;

class array_ref_sub : public rewriter_base
{
public:
    using var_map = context<var_ptr, expr_ptr>;

    array_ref_sub(copier & c, printer & p):
        m_copier(c), m_printer(p) {}

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_array_self_ref(const shared_ptr<array_self_ref> &e) override;

    var_map vars;
    unordered_map<array_ptr, expr_ptr> array_recursions;
    unordered_map<array_ptr, array_ptr> arrays;

    expr_ptr operator()(const expr_ptr & e) { return visit(e); }

private:
    copier & m_copier;
    printer & m_printer;
};

class array_reducer
{
public:
    array_reducer(name_provider &);

    id_ptr process(id_ptr);

    const unordered_set<id_ptr> & ids() const { return m_final_ids; }

private:
    void reduce(id_ptr id);
    expr_ptr reduce(expr_ptr);
    expr_ptr reduce(std::shared_ptr<array>);
    expr_ptr reduce(std::shared_ptr<array>, std::shared_ptr<array_patterns>);
    expr_ptr reduce(std::shared_ptr<array_app>);
    expr_ptr reduce(std::shared_ptr<func_app>);
    expr_ptr reduce(std::shared_ptr<primitive>);
    expr_ptr reduce(std::shared_ptr<operation>);
    expr_ptr reduce(std::shared_ptr<case_expr>);
    expr_ptr reduce(std::shared_ptr<reference>);

    vector<int> array_size(expr_ptr);
    vector<int> array_size(std::shared_ptr<array>);
    vector<array_var_ptr> make_array_vars(const array_size_vec & size);
    vector<expr_ptr> broadcast(const vector<array_var_ptr> & vars, const array_size_vec & size);
    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args);
    expr_ptr substitute(expr_ptr);
    expr_ptr eta_expand(expr_ptr);

    isl::context m_isl_ctx;

    string new_var_name();
    int var_count = 0;

    unordered_set<id_ptr> m_final_ids;
    unordered_set<id_ptr> m_processed_ids;
    unordered_map<id_ptr, expr_ptr> m_id_sub;

    using decl_var_stack = tracing_stack<array_var_ptr, stack_adapter<deque<array_var_ptr>>>;
    using decl_var_stacker = stacker<array_var_ptr, decl_var_stack>;
    decl_var_stack m_declared_vars;

    stack<unordered_set<array_var_ptr>> m_unbound_vars;

    printer m_printer;
    name_provider m_name_provider;
    copier m_copier;
    array_ref_sub m_sub;
};

}
}
#endif // STREAM_LANG_ARRAY_REDUCTION_INCLUDED
