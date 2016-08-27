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

        if (auto const_int = dynamic_pointer_cast<int_const>(expr))
        {
            return visit_int(const_int);
        }
        else if (auto r = dynamic_pointer_cast<real_const>(expr))
        {
            return visit_real(r);
        }
        else if (auto const_complex = dynamic_pointer_cast<complex_const>(expr))
        {
            return visit_complex(const_complex);
        }
        else if (auto const_bool = dynamic_pointer_cast<bool_const>(expr))
        {
            return visit_bool(const_bool);
        }
        else if (auto inf = dynamic_pointer_cast<infinity>(expr))
        {
            return visit_infinity(inf);
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
        else if (auto ap = dynamic_pointer_cast<array_patterns>(expr))
        {
            return visit_array_patterns(ap);
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

    virtual R visit_int(const shared_ptr<int_const> &) = 0;
    virtual R visit_real(const shared_ptr<real_const> &) = 0;
    virtual R visit_complex(const shared_ptr<complex_const> &) = 0;
    virtual R visit_bool(const shared_ptr<bool_const> &) = 0;
    virtual R visit_infinity(const shared_ptr<infinity> &) = 0;
    virtual R visit_ref(const shared_ptr<reference> &) = 0;
    virtual R visit_array_self_ref(const shared_ptr<array_self_ref> &) = 0;
    virtual R visit_primitive(const shared_ptr<primitive> & prim) = 0;
    virtual R visit_operation(const shared_ptr<operation> &) = 0;
    virtual R visit_affine(const shared_ptr<affine_expr> &) = 0;
    virtual R visit_cases(const shared_ptr<case_expr> & cexpr) = 0;
    virtual R visit_array(const shared_ptr<array> & arr) = 0;
    virtual R visit_array_patterns(const shared_ptr<array_patterns> &) = 0;
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

        if (!expr)
            return;

        if (auto const_int = dynamic_pointer_cast<int_const>(expr))
        {
            visit_int(const_int);
        }
        else if (auto r = dynamic_pointer_cast<real_const>(expr))
        {
            visit_real(r);
        }
        else if (auto r = dynamic_pointer_cast<complex_const>(expr))
        {
            visit_complex(r);
        }
        else if (auto const_bool = dynamic_pointer_cast<bool_const>(expr))
        {
            visit_bool(const_bool);
        }
        else if (auto inf = dynamic_pointer_cast<infinity>(expr))
        {
            visit_infinity(inf);
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
        else if (auto ap = dynamic_pointer_cast<array_patterns>(expr))
        {
            visit_array_patterns(ap);
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

    virtual void visit_int(const shared_ptr<int_const> &)
    {}
    virtual void visit_real(const shared_ptr<real_const> &)
    {}
    virtual void visit_complex(const shared_ptr<complex_const> &)
    {}
    virtual void visit_bool(const shared_ptr<bool_const> &)
    {}
    virtual void visit_infinity(const shared_ptr<infinity> &)
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
    virtual void visit_array_patterns(const shared_ptr<array_patterns> & ap)
    {
        for (auto & pattern : ap->patterns)
        {
            for (auto & index : pattern.indexes)
            {
                if (index.var && index.var->range)
                    visit(index.var->range);
            }
            if (pattern.domains)
                visit(pattern.domains);
            visit(pattern.expr);
        }
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

class rewriter_base : public visitor<expr_ptr>
{
public:
    virtual expr_ptr visit_int(const shared_ptr<int_const> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_real(const shared_ptr<real_const> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_complex(const shared_ptr<complex_const> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_bool(const shared_ptr<bool_const> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_infinity(const shared_ptr<infinity> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_ref(const shared_ptr<reference> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_array_self_ref(const shared_ptr<array_self_ref> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_primitive(const shared_ptr<primitive> & e) override
    {
        for(auto & operand : e->operands)
        {
            operand = visit(operand);
        }
        return e;
    }
    virtual expr_ptr visit_operation(const shared_ptr<operation> & e) override
    {
        for(auto & operand : e->operands)
        {
            operand = visit(operand);
        }
        return e;
    }
    virtual expr_ptr visit_affine(const shared_ptr<affine_expr> & e) override
    {
        return e;
    }
    virtual expr_ptr visit_cases(const shared_ptr<case_expr> & e) override
    {
        for (auto & c : e->cases)
        {
            c.first = visit(c.first);
            c.second = visit(c.second);
        }
        return e;
    }
    virtual expr_ptr visit_array(const shared_ptr<array> & arr) override
    {
        for (auto & var : arr->vars)
        {
            if (var->range)
                var->range = visit(var->range);
        }

        for(auto & id : arr->scope.ids)
            id->expr = visit(id->expr);

        arr->expr = visit(arr->expr);
        return arr;
    }
    virtual expr_ptr visit_array_patterns(const shared_ptr<array_patterns> & ap) override
    {
        for (auto & pattern : ap->patterns)
        {
            for (auto & index : pattern.indexes)
            {
                if (index.var && index.var->range)
                    index.var->range = visit(index.var->range);
            }
            if (pattern.domains)
                pattern.domains = visit(pattern.domains);
            pattern.expr = visit(pattern.expr);
        }
        return ap;
    }
    virtual expr_ptr visit_array_app(const shared_ptr<array_app> & app) override
    {
        app->object = visit(app->object);
        for (auto & arg : app->args)
            arg = visit(arg);
        return app;
    }
    virtual expr_ptr visit_array_size(const shared_ptr<array_size> & as) override
    {
        as->object = visit(as->object);
        if (as->dimension)
            as->dimension = visit(as->dimension);
        return as;
    }
    virtual expr_ptr visit_func_app(const shared_ptr<func_app> & app) override
    {
        app->object = visit(app->object);
        for (auto & arg : app->args)
            arg = visit(arg);
        return app;
    }
    virtual expr_ptr visit_func(const shared_ptr<function> & func) override
    {
        for(auto & id : func->scope.ids)
            id->expr = visit(id->expr);

        func->expr = visit(func->expr);
        return func;
    }
};

}
}

#endif // STREAM_LANG_FUNCTIONAL_MODEL_VISITOR_INCLUDED
