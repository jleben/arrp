#ifndef STREAM_FUNCTIONAL_TYPE_CHECKER_INCLUDED
#define STREAM_FUNCTIONAL_TYPE_CHECKER_INCLUDED

#include "../common/primitives.hpp"
#include "../common/functional_model.hpp"
#include "../utility/context.hpp"

#include <memory>

namespace stream {
namespace functional {

#if 0
class type
{
public:
    virtual ~type(){}
};
typedef std::shared_ptr<type> type;
#endif

class array_type
{
public:
    array_type() {}
    array_type(primitive_type t):
        elem_type(t) {}
    array_type(primitive_type t, const vector<int> & size):
        elem_type(t), size(size) {}
    primitive_type elem_type;
    vector<int> size;
};

class type_checker
{
public:
    array_type check(func_def_ptr, const vector<array_type> & args);

private:
    using context_type = context<func_var_ptr, array_type>;

    context_type m_context;

private:
    array_type check
    (func_def_ptr, const vector<array_type> & args, const location_type & loc);
    array_type check(expr_ptr);
    array_type check(std::shared_ptr<primitive>);
    array_type check(std::shared_ptr<func_app>);
};

}
}

#endif // STREAM_FUNCTIONAL_TYPE_CHECKER_INCLUDED
