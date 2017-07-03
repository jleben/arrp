#include "../common/functional_model.hpp"
#include "types.hpp"

#include <memory>
#include <list>

namespace arrp {

using namespace stream::functional;
using std::shared_ptr;
using std::list;

class type_graph
{
public:
    type add_variable()
    {
        return add_node(new type_variable);
    }

    template <typename T>
    shared_ptr<T> add_node()
    {
        auto t = std::make_shared<T>();
        nodes.push_back(t);
        return t;
    }

    type add_node(type t)
    {
        nodes.push_back(t);
        return t;
    }

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

    list<type> nodes;
    list<type_relation*> relations;
};

}
