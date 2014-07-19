#include "dispatch.hpp"
#include <iostream>
#include <stack>

using namespace dispatch;
using namespace std;

struct A;
struct B;
struct P;

typedef group<A,B,P> C;

struct O : public abstract_dispatchable<C>{};
struct AB : public O {};
struct A : public dispatchable<A,AB> { void report() { cout << "A" << endl; } };
struct B : public dispatchable<B,AB> { void report() { cout << "B" << endl; } };

struct P : public dispatchable<P,O>
{
    P( O&l, O&r ): x(l), y(r) {}
    O & x;
    O & y;
};



template<typename F, typename C>
class applicator
{
    struct helper
    {
        applicator & a;
        F f;
        helper(applicator &a, F f): a(a), f(f) {}
        helper(applicator &a): a(a) {}
        template <typename O> void operator()( O & object )
        {
            f(object, a);
        }
    };

    dispatcher<helper,C> m_dispatcher;

public:
    applicator( F f ): m_dispatcher(helper(*this,f)) {}
    applicator(): m_dispatcher(helper(*this)) {}

    template <typename O>
    void operator()( O & object )
    {
        object.dispatch_by(m_dispatcher);
    }
};

#if 0
template<typename F, typename T>
struct evaluator : private stack<T>
{
    evaluator( F f ): m_functor(f), m_dispatcher(helper_functor(*this)) {}

    template <typename O>
    T operator()( O & object )
    {
        object.dispatch_by(m_dispatcher);
        T v = this->top();
        this->pop();
        return v;
    }

private:
    struct helper_functor
    {
        evaluator & e;
        helper_functor(evaluator & e): e(e) {}

        template <typename TO>
        void operator()( TO & object )
        {
            e.push_value_of(object);
        }
    };

    friend struct helper_functor;

    template <typename TypedObject>
    void push_value_of( TypedObject & typed_object )
    {
        this->push( m_functor(typed_object) );
    }

    F m_functor;
    dispatcher<helper_functor,C> m_dispatcher;
};

struct actor
{
    actor(): m_evaluator(*this) {}

    int operator()( O & o ) { return m_evaluator(o); }
    int operator()( A & a ) { return 1; }
    int operator()( B & b ) { return 10; }
    int operator()( AB & o ) { return 100; }
    int operator()( P & p )
    {
        return m_evaluator(p.x) + m_evaluator(p.y);
    }
//private:
    evaluator<actor,int,O> m_evaluator;
};
#endif

struct my_func
{
    void operator()( A & a, applicator<my_func,C> & app ) { cout << "A"; }
    //void operator()( B & b, applicator<my_func,C> & app ) { cout << "B"; }
    void operator()( AB & o, applicator<my_func,C> & app ) { cout << "{A|B}"; }
    void operator()( P & p, applicator<my_func,C> & app )
    {
        cout << "{";
        app(p.x);
        cout << ".";
        app(p.y);
        cout << "}";
    }
};

int main()
{
    A a;
    B b;
    P p { a, b };

    O &o = p;

    my_func f;
    applicator<my_func,C> apply( f );
    //actor ac;
    apply.operator()(o);

    cout << endl;

    return 0;
}
