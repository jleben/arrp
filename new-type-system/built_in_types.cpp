#include "built_in_types.hpp"
#include <cassert>

using namespace std;

namespace arrp {

struct array_type_constructor : type_constructor
{
    array_type_constructor(): type_constructor("array") {}

    void print(ostream & out, const vector<type_ptr> & arguments) override
    {
        assert(arguments.size() == 1);
        out << "[" << arguments[0] << "]";
    }
};

struct func_type_constructor : type_constructor
{
    func_type_constructor(): type_constructor("function") {}

    void print(ostream & out, const vector<type_ptr> & arguments) override
    {
        assert(arguments.size() == 2);
        out << "(" << arguments[0] << ")"
            << " -> "
            << "(" << arguments[1] << ")";
    }
};

type_cons_ptr make_primitive_type(const string & name)
{
    return shared(new type_cons(shared(new type_constructor(name))));
}

built_in_types::built_in_types()
{
    m_array_cons = shared(new array_type_constructor);
    m_func_cons = shared(new func_type_constructor);

    m_boolean = make_primitive_type("bool");
    m_integer32 = make_primitive_type("int32");
    m_integer64 = make_primitive_type("int64");
    m_real32 = make_primitive_type("real32");
    m_real64 = make_primitive_type("real64");
    m_complex32 = make_primitive_type("complex32");
    m_complex64 = make_primitive_type("complex64");
    m_infinity = make_primitive_type("inf");

    m_integral = shared(new type_class("Integral"));
    m_integral->members.push_back(m_integer32);
    m_integral->members.push_back(m_integer64);

    m_real = shared(new type_class("Real"));
    m_real->members.push_back(m_real32);
    m_real->members.push_back(m_real64);

    m_complex = shared(new type_class("Complex"));
    m_complex->members.push_back(m_complex32);
    m_complex->members.push_back(m_complex64);

    m_numeric = shared(new type_class("Numeric"));
    m_integral->superclasses.push_back(m_numeric);
    m_real->superclasses.push_back(m_numeric);
    m_complex->superclasses.push_back(m_numeric);

    m_indexable = shared(new type_class("Indexable"));
    m_numeric->superclasses.push_back(m_indexable);

    m_indexable->members.push_back(array(shared(new type_var)));
}

type_cons_ptr built_in_types::array(type_ptr elem)
{
    return shared(new type_cons(m_array_cons, { elem }));
}

type_cons_ptr built_in_types::function(type_ptr param, type_ptr result)
{
    return shared(new type_cons(m_func_cons, { param, result }));
}

}

