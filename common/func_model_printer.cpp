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

void printer::print(id_ptr id, ostream & out)
{
    out << id->name << " = ";
    print(id->expr, out);
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
    else if (auto const_bool = dynamic_pointer_cast<const constant<bool>>(expr))
    {
        out << (const_bool->value ? "true" : "false");
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        out << name(ref->var);
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
    else if (auto ar = dynamic_pointer_cast<const array>(expr))
    {
        out << "[";
        {
            int count = ar->vars.size();
            for (auto var : ar->vars)
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
        if (!ar->bounded_exprs.empty())
        {
            for (auto & expr : ar->bounded_exprs)
            {
                print(expr.first, out);
                out << ": ";
                print(expr.second, out);
                out << "; ";
            }
            out << "else: ";
        }
        print(ar->expr, out);
        out << "]";
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        print(app->object, out);
        out << "[";
        int i = 0;
        for (const auto & arg : app->args)
        {
            print(arg, out);
            if(++i < app->args.size())
                out << ", ";
        }
        out << "]";
    }
    else if (auto app = dynamic_pointer_cast<func_app>(expr))
    {
        print(app->object, out);
        out << "(";
        int i = 0;
        for (const auto & arg : app->args)
        {
            print(arg, out);
            if(++i < app->args.size())
                out << ", ";
        }
        out << ")";
    }
    else if (auto func = dynamic_pointer_cast<function>(expr))
    {
        out << "\\";
        int i = 0;
        for (const auto & var : func->vars)
        {
            out << name(var);
            if(++i < func->vars.size())
                out << ", ";
        };
        out << " -> ";
        print(func->expr, out);
        out << "\\";
    }
    else if (auto scope = dynamic_pointer_cast<expr_scope>(expr))
    {
        out << "{";
        print(scope->expr, out);
        out << " where ";
        int i = 0;
        for(const auto & id : scope->ids)
        {
            if (i++ > 0)
                out << "; ";
            print(id, out);
        }
        out << "}";
    }
    else
    {
        out << "?";
    }
}

}
}
