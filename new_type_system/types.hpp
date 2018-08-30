#pragma once

#include "../common/error.hpp"
#include <memory>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <utility>

namespace arrp {

using std::shared_ptr;
using std::ostream;
using std::unordered_set;
using std::vector;
using std::string;
using std::pair;

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

class type_constraint
{
public:
    type_class_ptr klass;
    vector<type_ptr> params;
};

using type_constraint_ptr = std::shared_ptr<type_constraint>;

type_constraint_ptr add_constraint(type_class_ptr, const vector<type_ptr> & params);

class type_var : public type
{
public:
    type_var() {}

    unordered_set<type_constraint_ptr> constraints;
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

// Type construction (application of constructor to arguments)
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
    // Returns a type constructed using a list of class variables
    // A variable constrained with this class must unfy with the returned class instance.
    using instantiator = std::function<vector<type_ptr>()>;
    vector<instantiator> instances;
};

type_ptr unify(const type_ptr &, const type_ptr &,
               unordered_set<type_constraint_ptr> &);
void unify_and_satisfy_constraints(const type_ptr &, const type_ptr &);
void satisfy(unordered_set<type_constraint_ptr>);
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

struct printable_type_with_constraints { type_ptr t; };

inline
printable_type_with_constraints with_constraints(type_ptr t)
{
    return { t };
}

ostream & operator<<(ostream & out, const printable_type_with_constraints &);

ostream & operator <<(ostream & out, const type_constraint &);

}
