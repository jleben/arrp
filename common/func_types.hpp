#ifndef STREAM_LANG_FUNC_TYPES_INCLUDED
#define STREAM_LANG_FUNC_TYPES_INCLUDED

#include "primitives.hpp"

#include <memory>
#include <iostream>

namespace stream {
namespace functional {

using std::shared_ptr;

typedef vector<int> array_size_vec;

class type
{
public:
    virtual ~type() {}
    virtual void print(ostream &) const = 0;
};
typedef shared_ptr<type> type_ptr;

class scalar_type : public type
{
public:
    scalar_type(primitive_type p): primitive(p) {}
    primitive_type primitive;
    void print(ostream &) const override;
};

class array_type : public type
{
public:
    array_type() {}
    array_type(const array_size_vec & s, primitive_type e): size(s), element(e) {}
    void print(ostream &) const override;
    array_size_vec size;
    primitive_type element;
};

class function_type : public type
{
public:
    function_type(int a): arg_count(a) {}
    void print(ostream &) const override;
    int arg_count;
};

inline std::ostream & operator<<(std::ostream & s, const stream::functional::type & t)
{
    t.print(s);
    return s;
}

std::ostream & operator<<(std::ostream &, const array_size_vec &);

}
}



#endif // STREAM_LANG_FUNC_TYPES_INCLUDED
