#pragma once

#include "../common/functional_model.hpp"
#include "types.hpp"

#include <memory>
#include <list>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace arrp {

using namespace stream::functional;
using std::shared_ptr;
using std::list;
using std::ostream;
using std::string;
using std::unordered_set;
using std::unordered_map;

class type_graph
{
public:
    type_relation * add_relation(type_relation_kind k, type a, type b)
    {
        return add_relation(new type_relation(k, a, b));
    }

    type_relation * add_relation(type_relation * relation)
    {
        relations.push_back(relation);
        return relation;
    }

    void make_equal(type a, type b)
    {
        if (!a || !b)
            return;

        auto r = add_relation(equal_type, a, b);
        a->relations.push_back(r);
        b->relations.push_back(r);
    }

    void make_sub_type(type a, type b)
    {
        if (!a || !b)
            return;

        auto r = add_relation(sub_type, a, b);
        a->relations.push_back(r);
        b->relations.push_back(r);
    }

    void print(ostream &);

    list<type_relation*> relations;
};

class type_graph_printer
{
public:
    type_graph_printer() {}
    void print(const unordered_set<id_ptr> &, const type_graph & g, ostream &);
    void print(const id_ptr & id, ostream &);
    void print(const type & t, ostream &);
    void print(type_relation * rel, ostream &);
    void print(const type_class & c, ostream&);
    string name_for(const type & t);

private:

    int node_count = 0;
    unordered_map<concrete_type*, string> node_names;
};

}
