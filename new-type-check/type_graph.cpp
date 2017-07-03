#include "type_graph.hpp"

using namespace std;

namespace arrp {

string type_graph_printer::name_for(const type & t)
{
    string & name = node_names[t.get()];
    if (name.empty())
    {
        name = "t" + to_string(node_count);
        ++node_count;
    }
    return name;
}

void type_graph_printer::print(const unordered_set<id_ptr> & ids, const type_graph & g, ostream & out)
{
    for(const auto & id : ids)
    {
        print(id, out);
        out << endl;
    }

    for (type_relation * rel : g.relations)
    {
        print(rel, out);
        out << endl;
    }
}

void type_graph_printer::print(const id_ptr & id, ostream & out)
{
    out << id->name << " :: ";
    print(id->type2, out);
}

void type_graph_printer::print(type_relation *rel, std::ostream & out)
{
    print(rel->a, out);

    switch(rel->kind)
    {
    case equal_type:
        out << " = ";
        break;
    case sub_type:
        out << " < ";
        break;
    }

    print(rel->b, out);
}

void type_graph_printer::print(const type & t, ostream & out)
{
    if (!t)
    {
        out << "!";
        return;
    }

    out << name_for(t);
    out << ':';

    if (auto s = dynamic_pointer_cast<scalar_type>(t))
    {
        out << s->type;
    }
    else if (auto a = dynamic_pointer_cast<array_type>(t))
    {
        out << "[";
        print(a->element, out);
        out << "]";
    }
    else if (auto al = dynamic_pointer_cast<array_like_type>(t))
    {
        out << "{";
        print(al->element, out);
        out << "}";
    }
    else if (auto f = dynamic_pointer_cast<function_type>(t))
    {
        out << "(";
        for (const auto & p : f->parameters)
            print(p, out);
        out << ") -> ";
        print(f->value, out);
    }
    else if (auto v = dynamic_pointer_cast<type_variable>(t))
    {
        out << "*";
        if (v->value)
        {
            out << " = ";
            print(v->value, out);
        }
    }
    else
    {
        out << "?";
    }
}

}
