#ifndef STREAM_LANG_FUNC_TYPES_INCLUDED
#define STREAM_LANG_FUNC_TYPES_INCLUDED

#include "primitives.hpp"

#include <memory>
#include <iostream>

namespace stream {
namespace functional {

using std::shared_ptr;

typedef vector<int> array_size_vec;

class scalar_type;
class array_type;
class function_type;

class type;
typedef shared_ptr<type> type_ptr;

class type
{
public:
    virtual ~type() {}
    virtual void print(ostream &) const = 0;
    virtual bool is_scalar() const { return false; }
    virtual bool is_array() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_constant() const { return false; }
    virtual bool is_affine() const { return false; }
    virtual bool is_data() const { return false; }
    virtual bool operator==(const type & other) const = 0;
    scalar_type * scalar();
    array_type * array();
    function_type * func();

    static type_ptr undefined();
    static type_ptr infinity();
};

class scalar_type : public type
{
public:
    scalar_type(primitive_type p): primitive(p) {}

    bool is_scalar() const override { return true; }
    bool is_constant() const override { return constant_flag; }
    bool is_affine() const override { return affine_flag; }
    bool is_data() const override { return data_flag; }

    virtual bool operator==(const type & t) const override
    {
        if (!t.is_scalar())
            return false;

        const auto & s = static_cast<const scalar_type &>(t);
        return primitive == s.primitive &&
                affine_flag == s.affine_flag &&
                constant_flag == s.constant_flag &&
                data_flag == s.data_flag;
    }

    void print(ostream &) const override;

    primitive_type primitive;
    bool affine_flag = false;
    bool constant_flag = false;
    bool data_flag = true;
};

inline scalar_type * type::scalar()
{
    if (!is_scalar())
        return nullptr;
    return static_cast<scalar_type*>(this);
}

inline shared_ptr<scalar_type> make_int_type()
{ return std::make_shared<scalar_type>(primitive_type::integer); }

inline type_ptr type::undefined()
{
    auto t = std::make_shared<scalar_type>(primitive_type::undefined);
    t->data_flag = true;
    t->affine_flag = true;
    t->constant_flag = true;
    return t;
}

inline type_ptr type::infinity()
{
    auto t = std::make_shared<scalar_type>(primitive_type::infinity);
    t->data_flag = false;
    t->affine_flag = false;
    t->constant_flag = false;
    return t;
}

class array_type : public type
{
public:
    array_type() {}
    array_type(const array_size_vec & size, const type_ptr & elem_type);
    array_type(const array_size_vec & s, primitive_type e): size(s), element(e) {}
    void print(ostream &) const override;
    bool is_array() const override { return true; }
    bool is_data() const override { return true; }
    virtual bool operator==(const type & t) const override
    {
        if (!t.is_array())
            return false;
        const auto & a = static_cast<const array_type &>(t);
        return size == a.size &&
                element == a.element;
    }
    array_size_vec size;
    primitive_type element;
};

inline array_type * type::array()
{
    if (!is_array())
        return nullptr;
    return static_cast<array_type*>(this);
}

class function_type : public type
{
public:
    function_type(int a): arg_count(a) {}
    void print(ostream &) const override;
    bool is_function() const override { return true; }
    virtual bool operator==(const type & t) const override
    {
        if (!t.is_function())
            return false;
        const auto & f = static_cast<const function_type &>(t);
        return arg_count == f.arg_count;
    }
    int arg_count;
};

inline function_type * type::func()
{
    if (!is_function())
        return nullptr;
    return static_cast<function_type*>(this);
}

array_size_vec common_array_size(const array_size_vec &, const array_size_vec &);

inline type_ptr type_for(const array_size_vec & size, primitive_type elem)
{
    if (size.empty())
        return std::make_shared<scalar_type>(elem);
    else
        return std::make_shared<array_type>(size, elem);
}

inline std::ostream & operator<<(std::ostream & s, const stream::functional::type & t)
{
    t.print(s);
    return s;
}

std::ostream & operator<<(std::ostream &, const array_size_vec &);

}
}



#endif // STREAM_LANG_FUNC_TYPES_INCLUDED
