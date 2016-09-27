#include "func_types.hpp"
#include "error.hpp"

using namespace std;

namespace stream {
namespace functional {

void scalar_type::print(ostream & s) const
{
    s << primitive;
}

void array_type::print(ostream & s) const
{
    s << size << element;
}

void function_type::print(ostream & s) const
{
    s << 'f' << '(';
    for (int p = 0; p < (int)params.size(); ++p)
    {
        if (p > 0)
            s << ", ";
        if (params[p])
            s << *params[p];
        else
            s << "?";
    }
    s << ')';
    s << " -> ";
    if (value)
        s << *value;
    else
        s << "?";
}

std::ostream & operator<<(std::ostream & s, const array_size_vec & v)
{
    s <<'[';
    int i = 0;
    for (auto & e : v)
    {
        if (i > 0)
            s << ",";
        if (e != -1)
            s << e;
        else
            s << '*';
        ++i;
    }
    s << ']';
    return s;
}

array_type::array_type(const array_size_vec & size, const type_ptr & elem_type)
{
    array_size_vec full_size = size;

    if (auto func = dynamic_pointer_cast<function_type>(elem_type))
    {
        throw error("Function not allowed as array element.");
    }
    else if (auto nested_arr = dynamic_pointer_cast<array_type>(elem_type))
    {
        full_size.insert(full_size.end(), nested_arr->size.begin(), nested_arr->size.end());
        element = nested_arr->element;
    }
    else if (auto scalar = dynamic_pointer_cast<scalar_type>(elem_type))
    {
        element = scalar->primitive;
    }

    this->size = full_size;
}

array_size_vec common_array_size(const array_size_vec & a, const array_size_vec & b)
{
    if (a.empty())
        return b;
    if (b.empty())
        return a;

    int max_dims = max(a.size(), b.size());
    int common_dims = min(a.size(), b.size());

    array_size_vec common_size(max_dims);

    for (int d = 0; d < common_dims; ++d)
    {
        if (a[d] != b[d] && a[d] != 1 && b[d] != 1)
            throw no_type();
        if (a[d] == 1)
            common_size[d] = b[d];
        else
            common_size[d] = a[d];
    }

    for (int d = common_dims; d < max_dims; ++d)
    {
        if (d < a.size())
            common_size[d] = a[d];
        else
            common_size[d] = b[d];
    }

    return common_size;
}

}
}
