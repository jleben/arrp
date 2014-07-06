#include "visitor.hpp"
#include <iostream>

using namespace stream;
using namespace std;

struct c1;
struct c2;
struct c3;

typedef visitor<c1,c2,c3> the_visitor;

struct base : public visitable_base<the_visitor> {};

template <typename T> using the_visitable = visitable<T, base>;

struct c1 : public the_visitable<c1> {};
struct c2 : public the_visitable<c2> {};
struct c3 : public the_visitable<c3> {};

struct some_visitor : public the_visitor
{
    void visit(base & visitable) { visitable.accept(*this); }
    void visit(c1 & visitable) { cout << "c1" << endl; }
    void visit(c2 & visitable) { cout << "c2" << endl; }
    //void invalid() { cout << "Invalid visitable type." << endl; }
    void visit(c3 & visitable) { cout << "c3" << endl; }
};

int main()
{
    c1 x;
    c2 y;
    c3 z;
    some_visitor v;

    {
        base &t = x;
        v.visit(t);
        //t.accept(v);
    }
    {
        base &t = y;
        v.visit(t);
        //t.accept(v);
    }
    {
        base &t = z;
        t.accept(v);
    }
    /*a_visitor & v_ = v;

    //visitable<a_visitor> & a = x;

    base &a = x;
    cout << a.accept(v_) << endl;

    base &b = y;
    cout << b.accept(v_) << endl;

    base &c = z;
    cout << c.accept(v_) << endl;*/
}
