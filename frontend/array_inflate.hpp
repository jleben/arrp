#include "../common/func_model_visitor.hpp"

#include <unordered_set>
#include <unordered_map>

namespace arrp {

using namespace stream::functional;
using std::unordered_set;
using std::unordered_map;

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

}
