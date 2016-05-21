#ifndef STREAM_LANG_FUNCTIONAL_MODEL_VISITOR_INCLUDED
#define STREAM_LANG_FUNCTIONAL_MODEL_VISITOR_INCLUDED

#include "functional_model.hpp"
#include "error.hpp"

#include <memory>

namespace stream {
namespace functional {

using std::shared_ptr;

template<typename R>
class visitor
{
public:
    virtual R visit(const expr_ptr & expr)
    {
        using namespace std;

        if (auto const_int = dynamic_pointer_cast<constant<int>>(expr))
        {
            return visit_int(const_int);
        }
        else if (auto const_double = dynamic_pointer_cast<constant<double>>(expr))
        {
            return visit_double(const_double);
        }
        else if (auto const_complex = dynamic_pointer_cast<complex_const>(expr))
        {
            return visit_complex(const_complex);
        }
        else if (auto const_bool = dynamic_pointer_cast<constant<bool>>(expr))
        {
            return visit_bool(const_bool);
        }
        else if (auto ref = dynamic_pointer_cast<reference>(expr))
        {
            return visit_ref(ref);
        }
        else if (auto aref = dynamic_pointer_cast<array_self_ref>(expr))
        {
            return visit_array_self_ref(aref);
        }
        else if (auto prim = dynamic_pointer_cast<primitive>(expr))
        {
            return visit_primitive(prim);
        }
        else if (auto op = dynamic_pointer_cast<operation>(expr))
        {
            return visit_operation(op);
        }
        else if (auto ae = dynamic_pointer_cast<affine_expr>(expr))
        {
            return visit_affine(ae);
        }
        else if (auto c = dynamic_pointer_cast<case_expr>(expr))
        {
            return visit_cases(c);
        }
        else if (auto ar = dynamic_pointer_cast<array>(expr))
        {
            return visit_array(ar);
        }
        else if (auto app = dynamic_pointer_cast<array_app>(expr))
        {
            return visit_array_app(app);
        }
        else if (auto as = dynamic_pointer_cast<array_size>(expr))
        {
            return visit_array_size(as);
        }
        else if (auto app = dynamic_pointer_cast<func_app>(expr))
        {
            return visit_func_app(app);
        }
        else if (auto func = dynamic_pointer_cast<function>(expr))
        {
            return visit_func(func);
        }
        else
        {
            throw error("Unexpected expression type");
        }
    }

    virtual R visit_int(const shared_ptr<constant<int>> &) = 0;
    virtual R visit_double(const shared_ptr<constant<double>> &) = 0;
    virtual R visit_complex(const shared_ptr<complex_const> &) = 0;
    virtual R visit_bool(const shared_ptr<constant<bool>> &) = 0;
    virtual R visit_ref(const shared_ptr<reference> &) = 0;
    virtual R visit_array_self_ref(const shared_ptr<array_self_ref> &) = 0;
    virtual R visit_primitive(const shared_ptr<primitive> & prim) = 0;
    virtual R visit_operation(const shared_ptr<operation> &) = 0;
    virtual R visit_affine(const shared_ptr<affine_expr> &) = 0;
    virtual R visit_cases(const shared_ptr<case_expr> & cexpr) = 0;
    virtual R visit_array(const shared_ptr<array> & arr) = 0;
    virtual R visit_array_app(const shared_ptr<array_app> & app) = 0;
    virtual R visit_array_size(const shared_ptr<array_size> & as) = 0;
    virtual R visit_func_app(const shared_ptr<func_app> & app) = 0;
    virtual R visit_func(const shared_ptr<function> & func) = 0;
};

template<>
class visitor<void>
{
public:
    virtual void visit(const expr_ptr & expr)
    {
        using namespace std;

        if (auto const_int = dynamic_pointer_cast<constant<int>>(expr))
        {
            visit_int(const_int);
        }
        else if (auto const_double = dynamic_pointer_cast<constant<double>>(expr))
        {
            visit_double(const_double);
        }
        else if (auto const_double = dynamic_pointer_cast<complex_const>(expr))
        {
            visit_complex(const_double);
        }
        else if (auto const_bool = dynamic_pointer_cast<constant<bool>>(expr))
        {
            visit_bool(const_bool);
        }
        else if (auto ref = dynamic_pointer_cast<reference>(expr))
        {
            visit_ref(ref);
        }
        else if (auto aref = dynamic_pointer_cast<array_self_ref>(expr))
        {
            visit_array_self_ref(aref);
        }
        else if (auto prim = dynamic_pointer_cast<primitive>(expr))
        {
            visit_primitive(prim);
        }
        else if (auto op = dynamic_pointer_cast<operation>(expr))
        {
            visit_operation(op);
        }
        else if (auto ae = dynamic_pointer_cast<affine_expr>(expr))
        {
            visit_affine(ae);
        }
        else if (auto c = dynamic_pointer_cast<case_expr>(expr))
        {
            visit_cases(c);
        }
        else if (auto ar = dynamic_pointer_cast<array>(expr))
        {
            visit_array(ar);
        }
        else if (auto app = dynamic_pointer_cast<array_app>(expr))
        {
            visit_array_app(app);
        }
        else if (auto as = dynamic_pointer_cast<array_size>(expr))
        {
            visit_array_size(as);
        }
        else if (auto app = dynamic_pointer_cast<func_app>(expr))
        {
            visit_func_app(app);
        }
        else if (auto func = dynamic_pointer_cast<function>(expr))
        {
            visit_func(func);
        }
        else
        {
            throw error("Unexpected expression type");
        }
    }

    virtual void visit_int(const shared_ptr<constant<int>> &)
    {}
    virtual void visit_double(const shared_ptr<constant<double>> &)
    {}
    virtual void visit_complex(const shared_ptr<complex_const> &)
    {}
    virtual void visit_bool(const shared_ptr<constant<bool>> &)
    {}
    virtual void visit_ref(const shared_ptr<reference> &)
    {}
    virtual void visit_array_self_ref(const shared_ptr<array_self_ref> &)
    {}
    virtual void visit_primitive(const shared_ptr<primitive> & prim)
    {
        for(auto & operand : prim->operands)
        {
            visit(operand);
        }
    }
    virtual void visit_operation(const shared_ptr<operation> & op)
    {
        for(auto & operand : op->operands)
        {
            visit(operand);
        }
    }
    virtual void visit_affine(const shared_ptr<affine_expr> &)
    {}
    virtual void visit_cases(const shared_ptr<case_expr> & cexpr)
    {
        for (auto & c : cexpr->cases)
        {
            visit(c.first);
            visit(c.second);
        }
    }
    virtual void visit_array(const shared_ptr<array> & arr)
    {
        for (auto & var : arr->vars)
        {
            if (var->range)
                visit(var->range);
        }
        visit(arr->expr);
    }
    virtual void visit_array_app(const shared_ptr<array_app> & app)
    {
        visit(app->object);
        for (auto & arg : app->args)
            visit(arg);
    }
    virtual void visit_array_size(const shared_ptr<array_size> & as)
    {
        visit(as->object);
        if (as->dimension)
            visit(as->dimension);
    }
    virtual void visit_func_app(const shared_ptr<func_app> & app)
    {
        visit(app->object);
        for (auto & arg : app->args)
            visit(arg);
    }
    virtual void visit_func(const shared_ptr<function> & func)
    {
        for(auto & id : func->scope.ids)
        {
            visit(id->expr);
        }

        visit(func->expr);
    }
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_MODEL_VISITOR_INCLUDED
