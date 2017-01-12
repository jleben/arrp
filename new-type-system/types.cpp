#include "types.hpp"
#include "../common/error.hpp"
#include "../utility/containers.hpp"

#include <ostream>

using namespace std;

namespace arrp {

static type_ptr unify_collapsed(const type_ptr & a, const type_ptr & b);

type_ptr unify(const type_ptr & a_raw, const type_ptr & b_raw)
{
    type_ptr a = collapse(a_raw);
    type_ptr b = collapse(b_raw);
    return unify_collapsed(a,b);
}

type_ptr unify_collapsed(const type_ptr & a, const type_ptr & b)
{
    if (auto a_var = dynamic_pointer_cast<type_var>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            auto result_var = shared(new type_var);
            result_var->constraints.insert(a_var->constraints.begin(), a_var->constraints.end());
            result_var->constraints.insert(b_var->constraints.begin(), b_var->constraints.end());
            a_var->constraints.clear();
            b_var->constraints.clear();

            a_var->value = b_var->value = result_var;

            return result_var;
        }
        else if (is_contained(a_var, b))
        {
            ostringstream msg;
            msg << "Variable " << *a << " is contained in type " << *b << ".";
            throw type_error(msg.str());
        }
        else
        {
            a_var->constraints.clear();
            a_var->value = b;
            return b;
        }
    }
    else if (auto a_cons = dynamic_pointer_cast<type_cons>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            // Already handled above
            return unify_collapsed(b, a);
        }
        else if (auto b_cons = dynamic_pointer_cast<type_cons>(b))
        {
            if (a_cons->kind != b_cons->kind ||
                    a_cons->arguments.size() != b_cons->arguments.size())
            {
                ostringstream msg;
                msg << "Type mismatch: " << *a << " != " << *b;
                throw type_error(msg.str());
            }

            for (int i = 0; i < (int)a_cons->arguments.size(); ++i)
            {
                auto unified_arg = unify_collapsed(a_cons->arguments[i], b_cons->arguments[i]);
                a_cons->arguments[i] = unified_arg;
                b_cons->arguments[i] = unified_arg;
            }

            return a_cons;
        }
    }

    throw stream::error("Unexpected kind of type.");
}

type_ptr follow(const type_ptr & t)
{
    if (auto v = dynamic_pointer_cast<type_var>(t))
    {
        if (v->value)
            return follow(v->value);
        else
            return v;
    }

    return t;
}

type_ptr collapse(const type_ptr & t)
{
    if (auto v = dynamic_pointer_cast<type_var>(t))
    {
        if (v->value)
            return collapse(v->value);
        else
            return v;
    }
    else if (auto cons = dynamic_pointer_cast<type_cons>(t))
    {
        for (auto & arg : cons->arguments)
        {
            arg = collapse(arg);
        }
        return cons;
    }
    else
    {
        throw stream::error("Unexpected kind of type.");
    }
}

bool is_contained(const type_var_ptr & v, const type_ptr & t)
{
    if (auto cons = dynamic_pointer_cast<type_cons>(t))
    {
        for (auto & arg : cons->arguments)
        {
            if (is_contained(v, arg))
                return true;
        }
        return false;
    }
    else
    {
        return v == t;
    }
}


void type_constructor::print(ostream & out, const vector<type_ptr> & arguments)
{
    out << name;
    if (!arguments.empty())
    {
        out << "(";
        out << print_sequence(arguments, ", ");
        out << ")";
    }
}

ostream & operator<<(ostream & out, const type & t)
{
    if (auto v = dynamic_cast<const type_var*>(&t))
    {
        out << *v;
    }
    else if (auto c = dynamic_cast<const type_cons*>(&t))
    {
        out << *c;
    }
    else
    {
        out << "?";
    }

    return out;
}

ostream & operator<<(ostream & out, const type_var & v)
{
    if (v.value)
    {
        out << "<" << &v << "> = " << *v.value;
    }
    else
    {
        if (!v.constraints.empty())
        {
            out << print_sequence(v.constraints, ", ");
            out << " => ";
        }
        out << "<" << &v << ">";
    }

    return out;
}

ostream & operator<<(ostream & out, const type_cons & t)
{
    t.kind->print(out, t.arguments);
    return out;
}

ostream & operator<<(ostream & out, const type_class & c)
{
    out << c.name;
    return out;
}

}
