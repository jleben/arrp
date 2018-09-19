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

    unordered_set<id_ptr> process(const unordered_set<id_ptr> & ids);

private:
    void process(id_ptr);
    expr_ptr reduce(expr_ptr);
    expr_ptr reduce(std::shared_ptr<array>);
    expr_ptr reduce(std::shared_ptr<array>, std::shared_ptr<array_patterns>);
    expr_ptr reduce(std::shared_ptr<array_app>);
    expr_ptr reduce(std::shared_ptr<func_app>);
    expr_ptr reduce(std::shared_ptr<primitive>);
    expr_ptr reduce(std::shared_ptr<operation>);
    expr_ptr reduce(std::shared_ptr<case_expr>);

    vector<int> array_size(expr_ptr);
    vector<int> array_size(std::shared_ptr<array>);
    vector<array_var_ptr> make_array_vars(const array_size_vec & size);
    vector<expr_ptr> broadcast(const vector<array_var_ptr> & vars, const array_size_vec & size);
    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args);
    expr_ptr substitute(expr_ptr);
    expr_ptr eta_expand(expr_ptr);
    expr_ptr lambda_lift(expr_ptr, const string & name);

    isl::context m_isl_ctx;

    unordered_set<id_ptr> m_processed_ids;

    printer m_printer;
    name_provider m_name_provider;
    copier m_copier;
    array_ref_sub m_sub;
};

}
}
#endif // STREAM_LANG_ARRAY_REDUCTION_INCLUDED
