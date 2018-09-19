#include "../common/func_model_visitor.hpp"
#include <memory>
#include <unordered_set>

namespace arrp {

using namespace stream::functional;
using std::unordered_set;

class collect_ids : public visitor<void>
{
public:
    unordered_set<id_ptr> collect(id_ptr id)
    {
        m_ids.clear();
        m_ids.insert(id);

        visit(id->expr);

        return m_ids;
    }

    virtual void visit_ref(const shared_ptr<reference> & ref)
    {
        if (auto id = std::dynamic_pointer_cast<identifier>(ref->var))
        {
            if (m_ids.count(id))
                return;
            m_ids.insert(id);
            visit(id->expr);
        }
    }

private:
    unordered_set <id_ptr> m_ids;
};

}
