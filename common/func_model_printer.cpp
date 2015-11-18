#include "func_model_printer.hpp"

using namespace std;

namespace stream {
namespace functional {

printer::printer()
{

}

template<typename L>
void print_list(const L & list, const string & separator = ", ")
{
    int size = list.size();
    int count = 0;
    for(const auto & elem : list)
    {
        ++count;
    }
}

void printer::print(func_def_ptr func, ostream & out)
{
    out << indentation();
    out << func->name;
    out << "(";
    int i = 0;
    for (const auto & var : func->vars)
    {
        out << name(var);
        if(++i < func->vars.size())
            out << ", ";
    }
    out << ")";
    out << " = ";
    print(func->expr, out);
    if (func->defs.size())
    {
        out << endl;
        indent();
        out << indentation() << "where { " << endl;
        for(const auto & def : func->defs)
            print(def, out);
        unindent();
        out << indentation() << "}";
    }
    out << endl;
}

void printer::print(expr_ptr expr, ostream & out)
{
    if (auto const_int = dynamic_pointer_cast<const constant<int>>(expr))
    {
        out << const_int->value;
    }
    else if (auto const_double = dynamic_pointer_cast<const constant<double>>(expr))
    {
        out << const_double->value;
    }
    else if (auto avar_ref = dynamic_pointer_cast<array_var_ref>(expr))
    {
        out << name(avar_ref->var);
    }
    else if (auto fvar_ref = dynamic_pointer_cast<func_var_ref>(expr))
    {
        auto fvar = fvar_ref->var;
        if (fvar->name.empty())
            out << name(fvar);
        else
            out << fvar->name;
    }
    else if (auto fref = dynamic_pointer_cast<func_ref>(expr))
    {
        out << fref->id->def->name;
    }
    else if (auto prim = dynamic_pointer_cast<const primitive>(expr))
    {
        out << prim->type;

        out << "(";
        for(auto & operand : prim->operands)
        {
            print(operand, out);
            if (operand != prim->operands.back())
                out << ", ";
        }
        out << ")";
    }
    else if (auto array = dynamic_pointer_cast<const array_def>(expr))
    {
        out << "[";
        {
            int count = array->vars.size();
            for (auto var : array->vars)
            {
                out << name(var);
                if (var->range)
                {
                    out << ":";
                    print(var->range, out);
                }
                if (--count)
                    out << ", ";
            }
        }
        out << " -> ";
        print(array->expr, out);
        out << "]";
    }
    else if (auto aapp = dynamic_pointer_cast<array_app>(expr))
    {
        print(aapp->object, out);
        out << "[";
        int i = 0;
        for (const auto & arg : aapp->args)
        {
            print(arg, out);
            if(++i < aapp->args.size())
                out << ", ";
        }
        out << "]";
    }
    else if (auto fapp = dynamic_pointer_cast<func_app>(expr))
    {
        print(fapp->object, out);
        out << "(";
        int i = 0;
        for (const auto & arg : fapp->args)
        {
            print(arg, out);
            if(++i < fapp->args.size())
                out << ", ";
        }
        out << ")";
    }
    else
    {
        out << "?";
    }
}

}
}
