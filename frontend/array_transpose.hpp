#ifndef STREAM_LANG_ARRAY_TRANSPOSITION_INCLUDED
#define STREAM_LANG_ARRAY_TRANSPOSITION_INCLUDED

#include "../common/func_model_visitor.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>

namespace stream {
namespace functional {

using std::unordered_map;
using std::unordered_set;
using std::vector;
using std::list;

class array_transposer : private visitor<void>
{
public:
    void process(unordered_set<id_ptr> & ids);

private:
    void transpose_arrays(unordered_set<id_ptr> & ids);
    void transpose_array(const id_ptr & id);

    void transpose_accesses(unordered_set<id_ptr> & ids);
    void transpose_access(const id_ptr & id);

    void visit_array_app(const shared_ptr<array_app> & app) override;

    unordered_map<id_ptr, vector<int>> m_transpositions;
    id_ptr m_current_id;
};

}
}

#endif // STREAM_LANG_ARRAY_TRANSPOSITION_INCLUDED
