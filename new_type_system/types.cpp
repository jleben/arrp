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

type_ptr unify_and_satisfy_constraints(const type_ptr & a, const type_ptr & b)
{
    // FIXME: Some constraints may become equivalent after unification.
    // Remove them.

    unordered_set<type_constraint_ptr> affected_constraints;
    auto t = unify(a, b, affected_constraints);
    satisfy(affected_constraints);
    return follow(t);
}

type_ptr unify(const type_ptr & a_raw, const type_ptr & b_raw,
               unordered_set<type_constraint_ptr> & affected_constraints)
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
            affected_constraints.insert(a_var->constraints.begin(), a_var->constraints.end());
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
            return unify(b, a, affected_constraints);
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
                auto unified_arg = unify(a_cons->arguments[i], b_cons->arguments[i],
                                         affected_constraints);
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

static bool matches_constraint(const type_ptr & actual, const type_ptr & required)
{
    auto actual_cons = dynamic_pointer_cast<type_cons>(follow(actual));
    auto required_cons = dynamic_pointer_cast<type_cons>(follow(required));

    if (actual_cons && required_cons)
    {
        if (actual_cons->kind != required_cons->kind)
            return false;

        for (int i = 0; i < actual_cons->arguments.size(); ++i)
        {
            if (!matches_constraint(actual_cons->arguments[i], required_cons->arguments[i]))
                return false;
        }

        return true;
    }
    else
    {
        return true;
    }
}

static void remove(const type_constraint_ptr &constraint, const type_ptr & t)
{
    if (auto v = dynamic_pointer_cast<type_var>(t))
    {
        v->constraints.erase(constraint);
    }
    else if (auto c = dynamic_pointer_cast<type_cons>(t))
    {
        for (const auto & arg : c->arguments)
            remove(constraint, arg);
    }
}

static void remove(const type_constraint_ptr & c)
{
    for (auto & param : c->params)
    {
        remove(c, param);
    }
}

// Returns true if the constraint was satisfied by exactly one instance.
// Returns false if the constraint satisfaction is ambiguous.
// Throws type_error if the constraint can not be satisfied.
// If the constraint is satisfied, it is removed.
//
// When selecting an instance, only type constructors are compared and
// not their arguments!
// So e.g. constraint params (C1 C2 C3) would match instance (C1 a a)
// although C2 and C3 are not equal.
// However, in the next step, the unification of the instance with the
// constraint would fail.
static bool satisfy(const type_constraint_ptr & c,
                    unordered_set<type_constraint_ptr> & affected_constraints)
{
    cout << "Trying to satisfy: " << *c << endl;

    vector<type_ptr> matching_instance;

    for (const auto & instantiator : c->klass->instances)
    {
        auto instance = instantiator();

        cout << "Candidate instance: " << printable(instance, ", ") << endl;

        if (instance.size() != c->params.size())
            throw stream::error("Parameter count mismatch between type constraint and class instance.");

        bool is_matching = true;
        for (int i = 0; i < c->params.size(); ++i)
        {
            bool ok = matches_constraint(c->params[i], instance[i]);
            is_matching &= ok;
            //cout << "Matching: " << c->params[i] << " and " << instance[i] << ": " << ok << endl;
        }

        if (is_matching)
        {
            if (matching_instance.size())
            {
                // Another instance already matched, so satisfaction is ambiguous.
                cout << "Ambiguous." << endl;
                return false;
            }
            matching_instance = instance;
        }
    }

    cout << "Matching instance: " << printable(matching_instance, ", ") << endl;

    if (matching_instance.empty())
    {
        ostringstream msg;
        msg << "Unsatisfied constraint: " << *c;
        throw type_error(msg.str());
    }

    for (int i = 0; i < c->params.size(); ++i)
    {
        unify(c->params[i], matching_instance[i], affected_constraints);
    }

    cout << "Satisfied: " << *c << endl;

    remove(c);

    return true;
}

// Tries to satisfy the given constraints and as well as all other constraints
// affected in the process.
void satisfy(unordered_set<type_constraint_ptr> constraints)
{
    while(!constraints.empty())
    {
        auto & c = *constraints.begin();
        bool is_satisfied = satisfy(c, constraints);
        if (is_satisfied)
            constraints.erase(c);
        else
            break;
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
void collect_constraints(type_ptr t, unordered_set<const type_constraint*> & constraints)
{
    t = follow(t);

    if (auto v = dynamic_cast<const type_var*>(t.get()))
    {
        for (auto & c : v->constraints)
            constraints.insert(c.get());
    }
    else if (auto c = dynamic_cast<const type_cons*>(t.get()))
    {
        for (auto & arg : c->arguments)
            collect_constraints(arg, constraints);
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
    collect_constraints(t, constraints);

    if (!constraints.empty())
    {
        int i = 0;
        for (auto * c : constraints)
        {
            if (i > 0) out << ", ";
            ++i;
            out << *c;
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

ostream & operator <<(ostream & out, const type_constraint & c)
{
    out << c.klass->name;
    for (const auto & param : c.params)
    {
        out << " " << param;
    }
    return out;
}

}
