#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"

#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <string>

namespace arrp {

using std::unordered_map;
using std::unordered_set;
using std::stack;
using std::shared_ptr;
using std::string;

using namespace stream::functional;

class topological_sort : stream::functional::visitor<void>
{
public:
    void process(scope &);

private:
    void visit_id(const id_ptr &);
    virtual void visit_ref(const shared_ptr<reference> &) override;
    virtual void visit_func(const shared_ptr<function> &) override;
    virtual void visit_array(const shared_ptr<array> &) override;

    struct sorted_scope
    {
        vector<id_ptr> ids;
    };

    unordered_map<id_ptr, sorted_scope*> m_scopes;
    unordered_set<id_ptr> m_visited_ids;
    unordered_set<id_ptr> m_visiting_ids;

    static void print_topology(scope & s, int indent = 0);
};

class topology_printer : stream::functional::visitor<void>
{
public:
    void visit_scope(scope &);
    virtual void visit_func(const shared_ptr<function> &) override;
    virtual void visit_array(const shared_ptr<array> &) override;
    string indent() { return string(m_indent * 2, ' '); }
    int m_indent = 0;
};

}
