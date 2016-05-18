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

#ifndef STREAM_LANG_AST_INCLUDED
#define STREAM_LANG_AST_INCLUDED

#include <vector>
#include <memory>
#include <string>
#include <cassert>
#include <iostream>

#include "../frontend/location.hh"
#include "../common/primitives.hpp"

namespace stream
{

using std::vector;
using std::string;
template <typename T> using sp = std::shared_ptr<T>;

namespace semantic {
struct type;
using type_ptr = std::shared_ptr<type>;
}

namespace ast {

struct output; // verbose output;

using location_type = parsing::location;

struct node;

using node_ptr = std::shared_ptr<node>;

typedef node_ptr semantic_value_type;

enum node_type
{
    anonymous,

    program,

    constant,
    identifier,
    qualified_id,
    primitive,
    case_expr,

    array_self_ref,
    array_def,
    array_enum,
    array_params,
    array_param,
    array_apply,
    array_size,
    array_concat,

    func_def,
    func_apply,

    node_type_count
};

struct list_node;
template <typename T> struct leaf_node;

struct node
{
    node_type type;
    location_type location;
    semantic::type_ptr semantic_type;

    node( const node & other ):
        type(other.type),
        location(other.location),
        semantic_type(other.semantic_type)
    {}

    node( node_type type, const location_type & loc ): type(type), location(loc) {}
    virtual ~node() {}

    virtual bool is_list() { return false; }
    virtual bool is_leaf() { return false; }

    virtual node_ptr clone()
    {
        node *n = new node(*this);
        return node_ptr(n);
    }

    inline list_node *as_list();

    template <typename T>
    inline leaf_node<T> *as_leaf();
};

struct list_node : public node
{
    vector<sp<node>> elements;

    list_node( node_type type, const location_type & loc ):
        node(type, loc)
    {}


    list_node( node_type type, const location_type & loc,
               std::initializer_list<node_ptr> elements ):
        node(type, loc),
        elements(elements)
    {}

    list_node( const list_node & other ):
        node(other)
    {
        elements.reserve(other.elements.size());
        for(const node_ptr & e : other.elements)
        {
            if (e)
                elements.push_back(e->clone());
            else
                elements.push_back(node_ptr());
        }
    }

    bool is_list() { return true; }

    virtual node_ptr clone()
    {
        list_node *n = new list_node(*this);
        return node_ptr(n);
    }

    void append( const node_ptr & element )
    {
        elements.push_back(element);
    }

    void append( const vector<node_ptr> & other_elements )
    {
        elements.insert( elements.end(),
                         other_elements.begin(), other_elements.end() );
    }
};

template<typename T>
struct leaf_node : public node
{
    T value;

    leaf_node (node_type type, const location_type & loc, const T & v):
        node(type, loc),
        value(v)
    {}

    leaf_node(const leaf_node<T> & other):
        node(other),
        value(other.value)
    {}

    virtual node_ptr clone()
    {
        leaf_node<T> *n = new leaf_node<T>(*this);
        return node_ptr(n);
    }

    bool is_leaf() { return true; }
};

inline list_node *node::as_list()
{
    assert(is_list());
    return static_cast<list_node*>(this);
}

template <typename T>
inline leaf_node<T> *node::as_leaf()
{
    assert(is_leaf());
    return static_cast<leaf_node<T>*>(this);
}

inline
node_ptr make_node(ast::node_type type, const location_type & loc)
{
    return std::make_shared<node>(type, loc);
}

inline
node_ptr make_list(ast::node_type type, const location_type & loc,
                   std::initializer_list<node_ptr> elements)
{
    return std::make_shared<list_node>(type, loc, elements);
}

inline
node_ptr make_list(const location_type & loc,
                   std::initializer_list<node_ptr> elements)
{
    return std::make_shared<list_node>(anonymous, loc, elements);
}

inline
node_ptr make_id(const location_type & loc,
                 const string & name)
{
    return std::make_shared<leaf_node<string>>(identifier, loc, name);
}

template <typename T> inline
node_ptr make_const(const location_type & loc,
                    const T & value)
{
    return std::make_shared<leaf_node<T>>(constant, loc, value);
}

}

}

#endif // STREAM_LANG_AST_INCLUDED
