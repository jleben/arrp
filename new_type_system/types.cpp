#include "types.hpp"
#include "../common/error.hpp"
#include "../utility/printing.hpp"

#include <ostream>

using namespace std;

namespace arrp {

static void add_constraint(const type_constraint_ptr & c, const type_ptr & t)
{
    if (auto v = dynamic_pointer_cast<type_var>(t))
    {
        v->constraints.insert(c);
    }
    else if (auto cons = dynamic_pointer_cast<type_cons>(t))
    {
        for (auto & arg : cons->arguments)
        {
            add_constraint(c, arg);
        }
    }
}

type_constraint_ptr add_constraint(type_class_ptr klass, const vector<type_ptr> & params)
{
    auto constraint = shared(new type_constraint { klass, params });
    for (auto & param : params)
        add_constraint(constraint, param);
    return constraint;
}

type_ptr unify(const type_ptr & a_raw, const type_ptr & b_raw)
{
    type_ptr a = follow(a_raw);
    type_ptr b = follow(b_raw);

    cout << "Unifying " << *a << " and " << *b << " ..." << endl;

    if (auto a_var = dynamic_pointer_cast<type_var>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            auto result_var = shared(new type_var);
            result_var->constraints = a_var->constraints;
            result_var->constraints.insert(b_var->constraints.begin(), b_var->constraints.end());
            result_var->is_universal = a_var->is_universal || b_var->is_universal;
            a_var->constraints.clear();
            b_var->constraints.clear();

            a_var->value = b_var->value = result_var;

            cout << "Unified to " << *result_var << endl;
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
            a_var->value = b;

            type_ptr u = b;

            for (auto & constraint : a_var->constraints)
            {
                // FIXME: Picking an instance could be ambiguous
                // (result dependent on order of unifications)
                // if instanes overlap (parts of different instances are equal).
                // We should enforce a set of unambiguous instances.

                const auto & cls = constraint->klass;
                auto & args = constraint->params;
                bool satisfied = false;
                for (const auto & instantiator : cls->instances)
                {
                    auto instance = instantiator();
                    if (args.size() != instance.size())
                        throw stream::error("Unexpected: Constraint parameter count"
                                            " does not match class instance parameter count.");
                    try
                    {
                        for (int i = 0; i < args.size(); ++i)
                        {
                            unify(args[i], instance[i]);
                        }
                        satisfied = true;
                        break;
                    }
                    catch (type_error &) {}
                }

                if (!satisfied)
                {
                    ostringstream msg;
                    msg << "Constraint " << cls << " not satisfied by " << *u << ".";
                    throw type_error(msg.str());
                }
            }

            a_var->constraints.clear();

            cout << "Unified to " << *b << endl;

            return b;
        }
    }
    else if (auto a_cons = dynamic_pointer_cast<type_cons>(a))
    {
        if (auto b_var = dynamic_pointer_cast<type_var>(b))
        {
            // Already handled above
            return unify(b, a);
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
                auto unified_arg = unify(a_cons->arguments[i], b_cons->arguments[i]);
                a_cons->arguments[i] = unified_arg;
                b_cons->arguments[i] = unified_arg;
            }

            cout << "Unified to " << *a_cons << endl;

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
        {
            return v;
        }
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
        out << printable(arguments, ", ");
        out << ")";
    }
}

static
void collect_constraints(const type & t, unordered_set<const type_constraint*> & constraints)
{
    if (auto v = dynamic_cast<const type_var*>(&t))
    {
        for (auto & c : v->constraints)
            constraints.insert(c.get());
    }
    else if (auto c = dynamic_cast<const type_cons*>(&t))
    {
        for (auto & arg : c->arguments)
            collect_constraints(*arg, constraints);
    }
}

ostream & operator<<(ostream & out, const type_var & v)
{
    if (v.value)
    {
        out << v.value;
        //out << "<" << &v << "> = " << *v.value;
    }
    else
    {
        out << "<" << &v << ">";
    }

    return out;
}

ostream & operator<<(ostream & out, const type_cons & t)
{
    t.kind->print(out, t.arguments);
    return out;
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

ostream & operator<<(ostream & out, const printable_type_with_constraints & type_with_constraints)
{
    const auto & t = type_with_constraints.t;

    unordered_set<const type_constraint*> constraints;
    collect_constraints(*t, constraints);

    if (!constraints.empty())
    {
        int i = 0;
        for (auto * c : constraints)
        {
            if (i > 0)
                out << ", ";
            ++i;
            out << c->klass->name;
            for (const auto & arg : c->params)
            {
                out << " " << arg;
            }
        }
        out << " => ";
    }

    out << t;

    return out;
}

ostream & operator<<(ostream & out, const type_class & c)
{
    out << c.name;
    return out;
}

}
