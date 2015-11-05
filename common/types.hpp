/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef STREAM_LANG_TYPES_INCLUDED
#define STREAM_LANG_TYPES_INCLUDED

#include "../common/primitives.hpp"

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <cassert>
#include <initializer_list>

namespace stream {

namespace ast {
struct node;
using node_ptr = std::shared_ptr<node>;
}

namespace semantic {

using std::string;
using std::vector;
using std::list;
using std::ostream;
template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using up = std::unique_ptr<T>;

struct integer_num;
struct real_num;
struct stream;

struct type
{
    enum tag
    {
        boolean,
        integer_num,
        real_num,
        stream,
        iterator,
        function,
        builtin_function,
        builtin_function_group
    };

    type(tag t): m_tag(t) {}
    virtual ~type() {}

    tag get_tag() const { return m_tag; }

    bool is(tag t) const { return m_tag == t; }

    bool is_scalar() const
    {
        return m_tag == tag::boolean || m_tag == tag::integer_num || m_tag == real_num;
    }

    template<typename T>
    T & as() { return static_cast<T&>(*this); }

    template<typename T>
    const T & as() const { return static_cast<const T&>(*this); }

    virtual void print_on( ostream & s ) const
    {
        s << "<tag = " << m_tag << ">";
    }

private:
    tag m_tag;
};

using type_ptr = std::shared_ptr<type>;

type_ptr type_for_scalar(primitive_type pt);
type_ptr type_for_scalar(type::tag);

primitive_type primitive_type_for(type::tag);
primitive_type primitive_type_for(const type_ptr &);

inline ostream & operator<<( ostream & s, const type & t )
{
    t.print_on(s);
    return s;
}

template <type::tag TAG>
class tagged_type : public type
{
public:
    tagged_type(): type(TAG)
    {}
};

struct basic_scalar : public type
{
    basic_scalar(type::tag tag, bool is_constant):
        type(tag),
        m_is_constant(is_constant)
    {}

    bool is_constant() const { return m_is_constant; }

protected:
    bool m_is_constant;
};

template<typename T, type::tag TAG>
struct scalar : public basic_scalar
{
    using basic_scalar::m_is_constant;

    scalar():
        basic_scalar(TAG, false)
    {}

    scalar( const T & value ):
        basic_scalar(TAG, true),
        m_value(value)
    {}

    T constant_value() const
    {
        assert(m_is_constant);
        return m_value;
    }

    void set_constant( const T & value )
    {
        m_value = value;
        m_is_constant = true;
    }

private:
    T m_value;
};

struct boolean : public scalar<bool, type::boolean>
{
    boolean() {}
    boolean(bool b): scalar(b) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<b:" << (constant_value() ? "true" : "false") << ">";
        else
            s << "<b>";
    }
};

struct integer_num : public scalar<int, type::integer_num>
{
    integer_num() {}
    integer_num(int v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<i:" << constant_value() << ">";
        else
            s << "<i>";
    }
};

struct real_num : public scalar<double, type::real_num>
{
    real_num() {}
    real_num(double v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<r:" << constant_value() << ">";
        else
            s << "<r>";
    }
};

struct stream : public tagged_type<type::stream>
{
    enum
    {
        infinite = -1
    };

    explicit stream( const vector<int> & s, primitive_type et ) : size(s), element_type(et) {}
    explicit stream( int s, primitive_type et ) : size({s}), element_type(et) {}

    int dimensionality() const { return size.size(); }

    vector<int> size;
    primitive_type element_type;

    virtual void print_on( ostream & s ) const
    {
        s << "[";

        switch(element_type)
        {
        case primitive_type::boolean:
            s << "b:"; break;
        case primitive_type::integer:
            s << "i:"; break;
        case primitive_type::real:
            s << "r:"; break;
        }

        for (int i = 0; i < size.size(); ++i)
        {
            int sz = size[i];
            if (i > 0)
                s << ',';
            if (sz == infinite)
                s << "inf";
            else
                s << sz;
        }

        s << "]";
    }

    type_ptr reduced()
    {
        vector<int> new_size = size;
        new_size.erase( std::remove(new_size.begin(), new_size.end(), 1), new_size.end() );

        if (new_size.empty())
            return type_for_scalar(element_type);
        else
            return std::make_shared<semantic::stream>(new_size, element_type);
    }
};

struct iterator : public tagged_type<type::iterator>
{
    iterator(): hop(1), size(1), count(1) {}
    string id;
    int hop;
    int size;
    int count;
    ast::node_ptr domain;
    type_ptr value_type;
};

struct abstract_function : public type
{
    abstract_function(type::tag t):
        type(t)
    {}

    string name;
};

struct function : public abstract_function
{
    function(): abstract_function(type::function) {}

    vector<string> parameters;
    ast::node_ptr statement_list;
    ast::node_ptr statement;

    ast::node_ptr expression() const;
    type_ptr result_type() const;

    virtual void print_on( ostream & s ) const;
};

struct function_signature
{
    function_signature() {}
    function_signature( const vector<type::tag> & parameters, type::tag result ):
        result(result), parameters(parameters)
    {}
    bool operator==(const function_signature & other) const
    {
        return result == other.result && parameters == other.parameters;
    }

    type::tag result;
    vector<type::tag> parameters;
};

struct builtin_function : public abstract_function
{
    builtin_function(): abstract_function(type::builtin_function) {}
    function_signature signature;
    primitive_op op;
};

struct builtin_function_group : public abstract_function
{
    builtin_function_group(): abstract_function(type::builtin_function_group) {}
    vector<function_signature> overloads;
    primitive_op op;
};

using func_type_ptr = std::shared_ptr<abstract_function>;

#if 0
    virtual void print_on( ostream & s ) const
    {
        s << name;
        if (overloads.size() == 1)
        {
            auto & types = overloads[0];
            int i = 0;
            s << "(";
            for (int i = 0; i < types.size() - 1; ++i)
            {
                if (i > 0)
                    s << ", ";
                s << *types[i];
            }
            s << ")";
            s << " -> " << *types.back();
        }
        else
        {
            s << "<overloaded>";
        }
    }
#endif

struct type_structure
{
    vector<int> size;
    primitive_type type;

    bool is_scalar() const { return type_structure::is_scalar(size); }

    static
    bool is_scalar(const vector<int> & size)
    {
        return size.size() == 1 && size[0] == 1;
    }
};

type_structure structure(const type_ptr & t);

primitive_type operator+ (primitive_type, primitive_type);

type_ptr operator+ (const type_ptr & a, const type_ptr & b);

} // namespace semantic
} // namespace stream

namespace std {

template<> struct hash<stream::semantic::function_signature>
{
    std::size_t operator()( const stream::semantic::function_signature & signature ) const
    {
        using namespace stream::semantic;
        string key;
        key.reserve(3);

        auto tag_to_key = []( const type::tag &tag, string & key )
        {
            switch(tag)
            {
            case type::integer_num:
                key.push_back('i'); break;
            case type::real_num:
                key.push_back('r'); break;
            default:
                assert(false);
            }
        };

        tag_to_key(signature.result, key);
        for (const auto & tag : signature.parameters)
            tag_to_key(tag, key);

        return hash<string>()(key);
    }
};

}

#endif // STREAM_LANG_TYPES_INCLUDED
