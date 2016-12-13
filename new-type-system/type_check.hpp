#pragma once

#include "types.hpp"
#include "built_in_types.hpp"
#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/context.hpp"

namespace arrp  {

using namespace stream::functional;

class type_checker : stream::functional::visitor<type_ptr>
{
public:
    using var_ptr = stream::functional::var_ptr;

    type_checker(built_in_types * builtin);

    void process(const unordered_set<id_ptr> &);

private:
    type_ptr visit_int(const shared_ptr<int_const> &) override;
    type_ptr visit_real(const shared_ptr<real_const> &) override;
    type_ptr visit_func(const shared_ptr<function> & func) override;
    type_ptr visit_func_app(const shared_ptr<func_app> &app) override;
    type_ptr visit_ref(const shared_ptr<reference> &) override;

    type_ptr instance(type_ptr);

    bool is_free(type_var_ptr);

    using context_type = stream::context<var_ptr, type_ptr>;

    built_in_types * m_builtin;
    context_type m_context;
};

}
