#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../utility/stacker.hpp"

#include <deque>
#include <unordered_set>

namespace stream {
namespace functional {

using std::deque;
using std::unordered_set;

// Goal: Mark recursive IDs.

// Set identifier::is_recursive to true if identifier
// is part of a reference loop.

class reference_analysis : public functional::visitor<void>
{
public:
    reference_analysis();

    void process(const vector<id_ptr> & ids);
    void process(id_ptr start);

private:
    virtual void visit_ref(const shared_ptr<reference> &) override;
    virtual void visit_array(const shared_ptr<array> & arr) override;
    virtual void visit_func(const shared_ptr<function> & func) override;

    using visited_id_stack_t = stack_adapter<deque<id_ptr>>;

    visited_id_stack_t m_visited_ids;

    bool m_in_recursion = false;

    unordered_set<id_ptr> m_done_ids;
};

}
}
