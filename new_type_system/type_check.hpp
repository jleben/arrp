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
    using context_type = stream::context<var_ptr, type_ptr>;
    using type_var_map = unordered_map<type_var_ptr, type_var_ptr>;
    using type_constr_map = unordered_map<type_constraint_ptr, type_constraint_ptr>;

    void do_process_scope(const scope &);
    virtual type_ptr visit(const expr_ptr & expr) override;
    virtual type_ptr visit_ref(const shared_ptr<reference> &) override;
    virtual type_ptr visit_bool(const shared_ptr<bool_const> &) override;
    virtual type_ptr visit_int(const shared_ptr<int_const> &) override;
    virtual type_ptr visit_real(const shared_ptr<real_const> &) override;
    virtual type_ptr visit_complex(const shared_ptr<complex_const> &) override;
    virtual type_ptr visit_infinity(const shared_ptr<infinity> &) override;
    virtual type_ptr visit_primitive(const shared_ptr<primitive> &prim) override;
    virtual type_ptr visit_func(const shared_ptr<function> &) override;
    virtual type_ptr visit_func_app(const shared_ptr<func_app> &app) override;
    virtual type_ptr visit_array(const shared_ptr<array> & arr) override;
    virtual type_ptr visit_array_patterns(const shared_ptr<array_patterns> & ap) override;
    virtual type_ptr visit_cases(const shared_ptr<case_expr> & cexpr) override;
    virtual type_ptr visit_array_app(const shared_ptr<array_app> & app) override;

    // Duplicates universal variables and clones the rest
    type_ptr instance(const type_ptr &);
    type_ptr recursive_instance(type_ptr, type_var_map &, type_constr_map &, bool universal);

    built_in_types * m_builtin;
    context_type m_context;
    stream::functional::printer m_printer;

    unordered_set<id_ptr> m_done_ids;

    using bound_type_stack = stream::stack_adapter<std::deque<type_ptr>>;
    bound_type_stack m_bound_types;
};

class type_printer : stream::functional::visitor<void>
{
public:
    void print(scope &);
private:
    virtual void visit_func(const shared_ptr<function> &) override;
    virtual void visit_array(const shared_ptr<array> &) override;
    void print(const type_ptr &);
    string indent() { return string(m_indent * 2, ' '); }
    int m_indent = 0;
    unordered_map<type_var_ptr, int> m_var_names;
};

}
