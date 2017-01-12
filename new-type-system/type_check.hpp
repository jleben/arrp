#pragma once

#include "types.hpp"
#include "built_in_types.hpp"
#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/context.hpp"
#include "../utility/stacker.hpp"

#include <deque>
#include <unordered_map>

namespace arrp  {

using std::deque;
using std::unordered_map;

using namespace stream::functional;

class type_checker : stream::functional::visitor<type_ptr>
{
public:
    using var_ptr = stream::functional::var_ptr;

    type_checker(built_in_types * builtin);

    void process(const scope &);

private:
    type_ptr visit_bool(const shared_ptr<bool_const> &) override;
    type_ptr visit_int(const shared_ptr<int_const> &) override;
    type_ptr visit_real(const shared_ptr<real_const> &) override;
    type_ptr visit_complex(const shared_ptr<complex_const> &) override;
    type_ptr visit_infinity(const shared_ptr<infinity> &) override;

    type_ptr visit_primitive(const shared_ptr<primitive> &prim) override;

    type_ptr visit_func(const shared_ptr<function> & func) override;
    type_ptr visit_func_app(const shared_ptr<func_app> &app) override;
    type_ptr visit_ref(const shared_ptr<reference> &) override;

    type_ptr unify(const type_ptr & a, const type_ptr & b);

    type_ptr instance(type_ptr);
    type_ptr subterm_instance(type_ptr);

    static void make_universal(type_ptr);

    bool is_free(type_var_ptr);

    bool is_active(scope::group * group) const
    {
      const auto & all_active = m_active_scope_groups;
      return std::find(all_active.begin(), all_active.end(), group)
          != all_active.end();
    }

    using context_type = stream::context<var_ptr, type_ptr>;

    built_in_types * m_builtin;
    context_type m_context;

    using type_var_stack = stream::stack_adapter<deque<type_var_ptr>>;
    using scope_group_stack = stream::stack_adapter<deque<scope::group*>>;

    scope_group_stack m_active_scope_groups;

    unordered_map<type_var_ptr, type_var_ptr> m_var_instances;

    unordered_set<id_ptr> m_done_ids;

    stream::functional::printer m_printer;
};

}
