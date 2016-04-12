#ifndef STREAM_LANG_ARRAY_REDUCTION_INCLUDED
#define STREAM_LANG_ARRAY_REDUCTION_INCLUDED

#include "../common/functional_model.hpp"
#include "../utility/context.hpp"

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

class array_reducer
{
public:
    void process(id_ptr);

    const unordered_set<id_ptr> & ids() const { return m_ids; }
private:
    void reduce(id_ptr id);
    expr_ptr reduce(expr_ptr);
    expr_ptr reduce(std::shared_ptr<array>);
    expr_ptr reduce(std::shared_ptr<array_app>);
    expr_ptr reduce(std::shared_ptr<functional::array_size>);
    expr_ptr reduce(std::shared_ptr<primitive>);
    expr_ptr reduce(std::shared_ptr<case_expr>);
    vector<int> array_size(expr_ptr);
    vector<int> array_size(std::shared_ptr<array>);
    expr_ptr apply(expr_ptr, const vector<expr_ptr> & args);
    expr_ptr substitute(expr_ptr);
    expr_ptr eta_expand(expr_ptr);
    expr_ptr eta_expand(std::shared_ptr<reference>);

    using context_type = context<var_ptr, expr_ptr>;
    context_type m_context;

    isl::context m_isl_ctx;

    string new_var_name();
    int var_count = 0;

    unordered_set<id_ptr> m_ids;
    unordered_map<id_ptr, expr_ptr> m_id_sub;

    deque<array_var_ptr> m_declared_vars;
    stack<unordered_set<array_var_ptr>> m_unbound_vars;
};

}
}
#endif // STREAM_LANG_ARRAY_REDUCTION_INCLUDED
