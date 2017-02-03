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
class meta_type;

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
    virtual bool is_data() const { return false; }
    virtual bool is_meta() const { return false; }

    virtual bool operator==(const type & other) const = 0;
    virtual bool operator<=(const type & other) const { return *this == other; }
    bool operator!=(const type & other) const { return !(*this == other); }
    scalar_type * scalar();
    array_type * array();
    function_type * func();
    meta_type * meta();

    static type_ptr undefined();
    static type_ptr infinity();
};

class scalar_type : public type
{
public:
    scalar_type(primitive_type p): primitive(p) {}

    bool is_scalar() const override { return true; }
    bool is_data() const override { return primitive != primitive_type::infinity; }

    virtual bool operator==(const type & t) const override
    {
        if (!t.is_scalar())
            return false;

        const auto & s = static_cast<const scalar_type &>(t);
        return primitive == s.primitive;
    }

    virtual bool operator<=(const type & rhs) const override;

    void print(ostream &) const override;

    primitive_type primitive;
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
    return t;
}

inline type_ptr type::infinity()
{
    auto t = std::make_shared<scalar_type>(primitive_type::infinity);
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
    bool is_data() const override { return element != primitive_type::infinity; }
    bool is_infinite() const
    {
        for (auto & s : size)
            if (s < 0)
                return true;
        return false;
    }
    virtual bool operator==(const type & t) const override
    {
        if (!t.is_array())
            return false;
        const auto & a = static_cast<const array_type &>(t);
        return size == a.size &&
                element == a.element;
    }
    virtual bool operator<=(const type & other) const override
    {
        if (!other.is_array())
            return false;
        const auto & a = static_cast<const array_type &>(other);
        return size == a.size &&
                element <= a.element;
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
    function_type() {}

    function_type(int param_count):
        params(param_count)
    {}

    void print(ostream &) const override;

    bool is_function() const override { return true; }

    int param_count() const { return (int) params.size(); }

    virtual bool operator==(const type & t) const override
    {
        if (!t.is_function())
            return false;
        const auto & other = static_cast<const function_type &>(t);
        if (params.size() != other.params.size())
            return false;
        for (int p = 0; p < (int)params.size(); ++p)
            if (*params[p] != *other.params[p])
                return false;
        if (*value != *other.value)
            return false;
        return true;
    }

    vector<type_ptr> params;
    type_ptr value;
};

inline function_type * type::func()
{
    if (!is_function())
        return nullptr;
    return static_cast<function_type*>(this);
}

class meta_type : public type
{
public:
    meta_type() {}
    meta_type(type_ptr c): concrete(c) {}

    bool is_meta() const override { return true; }

    void print(ostream & out) const override;

    virtual bool operator==(const type & other) const
    {
        if (!other.is_meta())
            return false;
        const auto & other_meta = static_cast<const meta_type &>(other);
        return *this->concrete == *other_meta.concrete;
    }

    type_ptr concrete;
};

inline meta_type * type::meta()
{
    if (!is_meta())
        return nullptr;
    return static_cast<meta_type*>(this);
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
