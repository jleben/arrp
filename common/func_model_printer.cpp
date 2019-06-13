#include "func_model_printer.hpp"

#include <cmath>

using namespace std;

namespace stream {
namespace functional {

printer::printer()
{

}

void printer::print(const scope & s, ostream & out)
{
    using namespace std;
    for (const auto & id : s.ids)
    {
        print(id, out);
        out << endl;
    }
}

void printer::print(id_ptr id, ostream & out)
{
    out << id->name;
    out << "{";
    out << id->ref_count;
    if (id->is_recursive)
        out << "@";
    out << "}";

    if (id->expr->type)
    {
        out << " :: ";
        out << *id->expr->type;
    }
    else if (id->type_expr)
    {
        out << " :: ";
        out << '\'';
        print(id->type_expr, out);
        out << '\'';
    }
    out << " = ";
    print(id->expr, out);
}

void printer::print(expr_ptr expr, ostream & out)
{
    if (!expr)
    {
        out << "{}";
    }
    else if (auto const_int = dynamic_pointer_cast<int_const>(expr))
    {
        out << const_int->value;
    }
    else if (auto const_double = dynamic_pointer_cast<real_const>(expr))
    {
        out << const_double->value;
    }
    else if (auto const_bool = dynamic_pointer_cast<bool_const>(expr))
    {
        out << (const_bool->value ? "true" : "false");
    }
    else if (auto inf = dynamic_pointer_cast<infinity>(expr))
    {
        out << "~";
    }
    else if (auto const_complex = dynamic_pointer_cast<complex_const>(expr))
    {
        auto & v = const_complex->value;
        if (v.real())
            out << v.real() << '+';
        out << v.imag() << 'i';
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (m_print_var_address)
            out << ref->var;
        else
            out << name(ref->var);
    }
    else if (auto aref = dynamic_pointer_cast<array_self_ref>(expr))
    {
        out << "this";
    }
    else if (auto prim = dynamic_pointer_cast<const primitive>(expr))
    {
        out << prim->kind;

        out << "(";
        int count = 0;
        for(auto & operand : prim->operands)
        {
            print(operand, out);
            if (++count < prim->operands.size())
                out << ",";
        }
        out << ")";
    }
    else if (auto op = dynamic_pointer_cast<const operation>(expr))
    {
        switch(op->kind)
        {
        case operation::array_concatenate:
            out << "++"; break;
        case operation::array_enumerate:
            out << ".."; break;
        default:
            out << "?";
        }

        out << "(";
        int count = 0;
        for(auto & operand : op->operands)
        {
            print(operand, out);
            if (++count < op->operands.size())
                out << ",";
        }
        out << ")";
    }
    else if (auto ae = dynamic_pointer_cast<const affine_expr>(expr))
    {
        print(ae->expr, out);
    }
    else if (auto c = dynamic_pointer_cast<const case_expr>(expr))
    {
        int ci = 0;
        for (auto & a_case : c->cases)
        {
            if (ci++ > 0)
                out << ' ';
            out << "| ";
            print(a_case.first, out);
            out << " -> ";
            print(a_case.second, out);
        }
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
                    out << ",";
            }
        }
        out << ": ";
        print(ar->expr, out);
        if (m_print_scopes && ar->scope.ids.size())
        {
            out << " (where) ";
            int i = 0;
            for(const auto & id : ar->scope.ids)
            {
                if (i++ > 0)
                    out << ", ";
                print(id, out);
            }
        }
        out << "]";
    }
    else if (auto p = dynamic_pointer_cast<array_patterns>(expr))
    {
        auto pi = 0;
        for (auto & pattern : p->patterns)
        {
            if (pi++ > 0)
                out << ' ';
            auto dim = 0;
            for (auto & index : pattern.indexes)
            {
                if (dim++ > 0)
                    out << ',';

                if (index.is_fixed)
                    out << index.value;
                else
                    out << '_';
            }
            if (pattern.domains)
            {
                out << " ";
                print(pattern.domains, out);
                out << " | ";
                print(pattern.expr, out);
            }
            else
            {
                out << " -> ";
                print(pattern.expr, out);
            }
            out << ';';
        }
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
                out << ",";
        }
        out << "]";
    }
    else if (auto as = dynamic_pointer_cast<array_size>(expr))
    {
        out << '#';
        print(as->object, out);
        if (as->dimension)
        {
            out << '@';
            print(as->dimension, out);
        }
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
        if (m_print_scopes && func->scope.ids.size())
        {
            out << " (where) ";
            int i = 0;
            for(const auto & id : func->scope.ids)
            {
                if (i++ > 0)
                    out << ", ";
                print(id, out);
            }
        }
        out << "\\";
    }
    else if (auto ext = dynamic_pointer_cast<external>(expr))
    {
        out << "external ";
        out << ext->name;
        out << " :: ";
        out << '\'';
        print(ext->type_expr, out);
        out << '\'';
    }
    else if (auto type_name = dynamic_pointer_cast<type_name_expr>(expr))
    {
        out << type_name->name;
    }
    else if (auto array_type = dynamic_pointer_cast<array_type_expr>(expr))
    {
        out << "[";
        int i = 0;
        for (const auto & size : array_type->size)
        {
            if (i++ > 0)
                out << ",";
            print(size, out);
        }
        out << "]";
        out << array_type->element;
    }
    else if (auto func_type = dynamic_pointer_cast<func_type_expr>(expr))
    {
        for (const auto & param : func_type->params)
        {
            print(param, out);
            out << " -> ";
        }
        print(func_type->result, out);
    }
    else if (auto scope = dynamic_pointer_cast<scope_expr>(expr))
    {
        bool print_local_ids = m_print_scopes && scope->local.ids.size();

        if (print_local_ids)
            out << "{ ";

        print(scope->value, out);

        if (print_local_ids)
        {
            out << " where ";
            int i = 0;
            for(const auto & id : scope->local.ids)
            {
                if (i++ > 0)
                    out << ", ";
                print(id, out);
            }

            out << " }";
        }
    }
    else
    {
        out << "?";
    }
}

void printer::print(const linexpr & expr, ostream & out)
{
    out << "{";
    int t = 0;
    for(const auto & term : expr)
    {
        const auto var = term.first;
        int coef = term.second;
        int abs_coef = std::abs(coef);
        char sign = (coef > 0 ? '+' : '-');
        if (t > 0 || coef < 0)
            out << sign;
        if (!var || abs_coef != 1)
            out << abs_coef;
        if (var)
            out << var->name;
        ++t;
    }
    out << "}";
}

}
}
