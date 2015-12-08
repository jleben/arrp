#ifndef STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
#define STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED

#include "../common/functional_model.hpp"

#include <unordered_set>

namespace stream {
namespace functional {

using std::unordered_set;

class type_checker
{
public:
    void process(const unordered_set<id_ptr> & ids);
private:
    void process(id_ptr);
    primitive_type process(expr_ptr, id_ptr);
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_TYPE_CHECK_INCLUDED
