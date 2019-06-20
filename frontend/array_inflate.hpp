#include "../common/func_model_visitor.hpp"

#include <unordered_set>
#include <unordered_map>

namespace arrp {

using namespace stream::functional;
using std::unordered_set;
using std::unordered_map;

// Finish lambda-lifting expressions with free array variables.
// For each free variable "i" in a named expression "n = e",
// wrap the expression into an array "n = [i -> e]", thus binding the variable.
// Then, also update references to the expression "n" by applying a new
// variable to them: "n[i]".
// If "i" in the updated reference "n[i]" is again unbounded,
// repeate the procedure for the named expression containing the reference.

class array_inflate : public rewriter_base
{
public:
    struct array_var_info
    {
        id_ptr source;
        unordered_set<id_ptr> free;
    };

    using array_var_info_set = unordered_map<array_var_ptr, array_var_info>;
    using array_reference_set = unordered_map<id_ptr, unordered_set<id_ptr>>;

    void process(const scope &);
    void process(const unordered_set<id_ptr> & ids);

private:
    void close_under_references(unordered_set<id_ptr> & ids, const unordered_set<id_ptr> & exceptions);

    void inflate(const id_ptr & id);
    virtual expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    virtual void visit_local_id(const id_ptr &) override {} // Ignore

    array_var_info_set m_free_vars;
    array_reference_set m_references;

    struct
    {
        unordered_set<id_ptr> involved_ids;
        array_var_ptr var;
        array_var_ptr substitute_var;
        array_var_info var_info;
    }
    m_inflation;
};

class lift_local_ids : rewriter_base
{
public:
    lift_local_ids(scope & global);
private:
    virtual expr_ptr visit_scope(const shared_ptr<scope_expr> & scope) override;
    scope & m_global;
};

int array_size(const array_var_ptr & var);
type_ptr inflate_type(const type_ptr & t, const array_var_ptr & var);
type_ptr inflate_type(const type_ptr & t, const array_size_vec & size);

}
