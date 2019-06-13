#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "name_provider.hpp"

#include <unordered_set>

namespace arrp {

namespace fn = stream::functional;

using fn::id_ptr;
using fn::var_ptr;
using fn::expr_ptr;
using fn::name_provider;
using fn::scope;
using std::shared_ptr;

class folding : public fn::rewriter_base
{
public:
    folding(name_provider & nmp): m_name_provider(nmp) {}

    void process(id_ptr);

private:
    expr_ptr visit_ref(const shared_ptr<fn::reference> &) override;
    expr_ptr visit_scope(const shared_ptr<fn::scope_expr> &) override;
    expr_ptr visit_primitive(const shared_ptr<fn::primitive> &) override;

    expr_ptr cleanup(scope &);

    name_provider m_name_provider;
    std::unordered_set<id_ptr> m_visited_ids;
};

}
