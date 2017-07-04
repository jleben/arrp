#pragma once

#include "../common/primitives.hpp"

#include <memory>
#include <vector>

namespace arrp {

using stream::primitive_type;
using std::vector;
struct type_relation;

struct concrete_type
{
    virtual ~concrete_type() {}
    virtual bool is_data() = 0;

    vector<type_relation*> relations;
};

struct type_relation;

struct type : public std::shared_ptr<concrete_type>
{
    type() {}
    type(concrete_type * t): shared_ptr(t) {}

    template <typename T>
    type(const std::shared_ptr<T> & other): shared_ptr(other) {}

    template <typename T>
    type & operator=(const std::shared_ptr<T> & other)
    {
        shared_ptr::operator=(other);
        return *this;
    }

    template<typename T>
    type as()
    {
        return std::dynamic_pointer_cast<T>(*this);
    }
};

enum type_relation_kind
{
    equal_type,
    sub_type
};

struct type_relation
{
    type_relation(type_relation_kind k, const type & a, const type & b):
        kind(k), a(a), b(b) {}

    type_relation_kind kind;

    type a;
    type b;
};


struct infinity_type : public concrete_type
{
    bool is_data() override { return false; }
};

struct scalar_type : public concrete_type
{
    scalar_type(primitive_type t): type(t) {}
    bool is_data() override { return true; }

    primitive_type type;
};

struct array_type : public concrete_type
{
    array_type(const type & e): element(e) {}
    bool is_data() override { return true; }

    type element;
};

struct function_type : public concrete_type
{
    function_type() {}
    function_type(const vector<type> & p, const type & v): parameters(p), value(v) {}
    bool is_data() override { return false; }

    vector<type> parameters;
    type value;
};


enum type_class_kind
{
    data_type, // Not function
    scalar_data_type,
    numeric_type,
    real_numeric_type,
    complex_numeric_type, // parameter = real type
    simple_numeric_type,
    indexable_type, // parameter = the result of indexing
    array_like_type // parameter = the innermost element type
};

struct type_class
{
    type_class(type_class_kind k, const vector<type> & p = vector<type>()):
        kind(k), parameters(p) {}

    type_class_kind kind;
    vector<type> parameters;
};

struct type_variable : public concrete_type
{
    type_variable() {}

    type_variable(type_class_kind k, const vector<type> & p = vector<type>())
    {
        classes.emplace_back(k, p);
    }

    type_variable(const type & t): value(t) {}
    bool is_data() override { return false; }

    vector<type_class> classes;
    type value;
};

}
