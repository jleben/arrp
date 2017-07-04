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
        if (!v->classes.empty())
        {
            out << "(";
            int i = 0;
            for (auto & c : v->classes)
            {
                if (i++ > 0)
                    out << ",";
                print(c, out);
            }
            out << ")";
        }
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

void type_graph_printer::print(const type_class & c, ostream & out)
{
    switch(c.kind)
    {
    case data_type:
        out << "data"; break;
    case array_like_type:
        out << "array"; break;
    case indexable_type:
        out << "indexable"; break;
    case scalar_data_type:
        out << "scalar"; break;
    case numeric_type:
        out << "numeric"; break;
    case simple_numeric_type:
        out << "simple numeric"; break;
    case real_numeric_type:
        out << "real numeric"; break;
    case complex_numeric_type:
        out << "simple numeric"; break;
    default:
        out << "?";
    }
    if (!c.parameters.empty())
    {
        out << "(";
        int i = 0;
        for(const auto & p : c.parameters)
        {
            if (i++ > 0)
                out << ",";
            print(p, out);
        }
        out << ")";
    }
}

}
