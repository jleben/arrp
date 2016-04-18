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
    expr_ptr reduce(std::shared_ptr<array_app>);
    expr_ptr reduce(std::shared_ptr<functional::array_size>);
    expr_ptr reduce(std::shared_ptr<primitive>);
    expr_ptr reduce(std::shared_ptr<case_expr>);
    expr_ptr reduce(std::shared_ptr<reference>);
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

    unordered_set<id_ptr> m_final_ids;
    unordered_set<id_ptr> m_processed_ids;
    unordered_map<id_ptr, expr_ptr> m_id_sub;
    unordered_map<array_ptr, array_ptr> m_array_ref_sub;

    using decl_var_stack = tracing_stack<array_var_ptr, stack_adapter<deque<array_var_ptr>>>;
    using decl_var_stacker = stacker<array_var_ptr, decl_var_stack>;
    decl_var_stack m_declared_vars;

    stack<unordered_set<array_var_ptr>> m_unbound_vars;

    name_provider m_name_provider;

    copier m_copier;

    printer m_printer;
};

}
}
#endif // STREAM_LANG_ARRAY_REDUCTION_INCLUDED
