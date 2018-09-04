#pragma once

#include "types.hpp"
#include "../common/primitives.hpp"
#include <unordered_map>

namespace arrp {

using std::unordered_map;

class built_in_types
{
public:
    built_in_types();

    type_constructor_ptr array_cons() { return m_array_cons; }
    type_constructor_ptr function_cons() { return m_func_cons; }

    type_cons_ptr array(type_ptr size, type_ptr elem);
    type_cons_ptr array(const vector<type_ptr> & size, type_ptr elem);
    type_cons_ptr finite_array(type_ptr elem);
    type_cons_ptr infinite_array(type_ptr elem);

    type_cons_ptr function(type_ptr param, type_ptr result);
    type_cons_ptr function(const vector<type_ptr> & params, type_ptr result);

    type_cons_ptr infinity() { return m_infinity; }
    type_cons_ptr boolean() { return m_boolean; }
    type_cons_ptr integer32() { return m_integer32; }
    type_cons_ptr integer64() { return m_integer64; }
    type_cons_ptr real32() { return m_real32; }
    type_cons_ptr real64() { return m_real64; }
    type_cons_ptr complex32() { return m_complex32; }
    type_cons_ptr complex64() { return m_complex64; }

    type_class_ptr integral() { return m_integral; }
    type_class_ptr real() { return m_real; }
    type_class_ptr complex() { return m_complex; }
    type_class_ptr numeric() { return m_numeric; }
    type_class_ptr scalar() { return m_scalar; }
    type_class_ptr indexable() { return m_indexable; }
    type_class_ptr array_size() { return m_array_size; }

    type_ptr primitive_op(stream::primitive_op);

private:
    type_constructor_ptr m_array_cons;
    type_constructor_ptr m_func_cons;

    type_cons_ptr m_boolean;
    type_cons_ptr m_integer32;
    type_cons_ptr m_integer64;
    type_cons_ptr m_real32;
    type_cons_ptr m_real64;
    type_cons_ptr m_complex32;
    type_cons_ptr m_complex64;
    type_cons_ptr m_infinity;

    type_class_ptr m_integral;
    type_class_ptr m_real;
    type_class_ptr m_complex;
    type_class_ptr m_numeric;
    type_class_ptr m_scalar;
    type_class_ptr m_indexable;
    type_class_ptr m_array_size;

    unordered_map<stream::primitive_op, type_ptr> m_primitive_ops;
};

}
