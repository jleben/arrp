#pragma once

#include "../common/error.hpp"
#include <memory>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <string>

namespace arrp {

using std::shared_ptr;
using std::ostream;
using std::unordered_set;
using std::vector;
using std::string;

template <typename T>
inline
std::shared_ptr<T> shared(T * value)
{
    return shared_ptr<T>(value);
}

class type_error : public stream::error
{
public:
    type_error(const string & msg):
        error(msg)
    {}
};

class type;
class type_var;
class type_constructor;
class type_cons;
class type_class;

using type_ptr = std::shared_ptr<type>;
using type_var_ptr = std::shared_ptr<type_var>;
using type_constructor_ptr = std::shared_ptr<type_constructor>;
using type_cons_ptr = std::shared_ptr<type_cons>;
using type_class_ptr = std::shared_ptr<type_class>;

class type
{
public:
    virtual ~type() {}
};

class type_var : public type
{
public:
    type_var() {}

    unordered_set<type_class_ptr> constraints;
    type_ptr value;
    bool is_universal = false;
};

class type_constructor
{
public:
    string name;

    type_constructor(const string & name):
        name(name) {}

    virtual ~type_constructor() {}
    virtual void print(ostream & out, const vector<type_ptr> & arguments);
};

class type_cons : public type
{
public:
    type_cons(type_constructor_ptr kind, const vector<type_ptr> & args = vector<type_ptr>()):
        kind(kind), arguments(args) {}

    type_constructor_ptr kind;
    vector<type_ptr> arguments;
};


class type_class
{
public:
    type_class(const string & name): name(name) {}

    string name;
    vector<type_ptr> members;
    vector<type_class_ptr> superclasses;
};

type_ptr unify(const type_ptr &, const type_ptr &);

type_ptr follow(const type_ptr &);
type_ptr collapse(const type_ptr &);

bool is_contained(const type_var_ptr &, const type_ptr &);

ostream & operator<<(ostream & out, const type &);
ostream & operator<<(ostream & out, const type_var & v);
ostream & operator<<(ostream & out, const type_cons & t);
ostream & operator<<(ostream & out, const type_class & c);

inline ostream & operator<<(ostream & out, const type_ptr & t)
{
    out << *t;
    return out;
}

}
