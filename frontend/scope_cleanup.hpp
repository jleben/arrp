#pragma once

#include "functional_model.hpp"
#include "func_model_visitor.hpp"

#include <unordered_set>

namespace arrp {

namespace fn = stream::functional;
using std::shared_ptr;
using fn::id_ptr;
using fn::expr_ptr;

// Purpose:
// - Remove unused IDs.
// - Fix ID reference counts.

class scope_cleanup : public fn::rewriter_base
{
public:
    void clean(fn::scope & scope, fn::id_ptr start_id);

private:
    virtual expr_ptr visit_scope(const shared_ptr<fn::scope_expr> & scope) override;
    virtual expr_ptr visit_ref(const shared_ptr<fn::reference> & ref) override;
    void clean(fn::scope & e);

    std::unordered_set<id_ptr> m_used_ids;
};

}
