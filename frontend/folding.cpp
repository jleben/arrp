#include "folding.hpp"

#include "func_copy.hpp"

using namespace stream;
using namespace stream::functional;
using namespace std;

namespace arrp {

// FIXME: Implement constant expression folding

class foldable_expr_check : private visitor<bool>
{
public:
    bool operator()(const expr_ptr & e)
    {
        return visit(e);
    }

protected:
    virtual bool visit_int(const shared_ptr<int_const> &) override { return true; }
    virtual bool visit_real(const shared_ptr<real_const> &) override { return true; }
    virtual bool visit_complex(const shared_ptr<complex_const> &) override { return true; }
    virtual bool visit_bool(const shared_ptr<bool_const> &) override { return true; }
    virtual bool visit_infinity(const shared_ptr<infinity> &) override { return true; }
    virtual bool visit_ref(const shared_ptr<reference> &) override { return true; }
    virtual bool visit_array_self_ref(const shared_ptr<array_self_ref> &) override { return true; }
    virtual bool visit_array_app(const shared_ptr<array_app> & app) override {
        return visit(app->object);
    }
    virtual bool visit_array_size(const shared_ptr<array_size> & as) override { return true; }
    // Not foldable:
    // - scope expression, because it would duplicate local ids.
};

void folding::process(id_ptr id)
{
    id->type_expr = visit(id->type_expr);
    id->expr = visit(id->expr);
}

expr_ptr folding::visit_ref(const shared_ptr<fn::reference> & ref)
{
    auto id = dynamic_pointer_cast<identifier>(ref->var);
    if (!id) return ref;

    if (!m_visited_ids.count(id))
    {
        m_visited_ids.insert(id);
        if (id->type_expr) id->type_expr = visit(id->type_expr);
        id->expr = visit(id->expr);
    }

    foldable_expr_check is_foldable_expr;
    bool can_fold = !id->is_recursive && !id->type_expr && is_foldable_expr(id->expr);
    if (!can_fold) return ref;

    --id->ref_count;

    unordered_set<id_ptr> ids;
    copier copy(ids, m_name_provider);
    return copy.copy(id->expr);
}

expr_ptr folding::visit_scope(const shared_ptr<fn::scope_expr> & scope)
{
    // We should remove all folded IDs to increase chance of folding this expression.
    // A scope expression with remaining non-foldable IDs will not be folded itself.

    scope->value = visit(scope->value);

    vector<id_ptr> remaining_ids;
    for (auto & id : scope->local.ids)
    {
        if (id->ref_count > 0)
            remaining_ids.push_back(id);
    }

    if (remaining_ids.empty())
    {
        return scope->value;
    }
    else
    {
        scope->local.ids = remaining_ids;
        return scope;
    }
}

}
