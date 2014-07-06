#include "visitor.hpp"
#include <iostream>

using namespace stream;
using namespace std;

struct c1;
struct c2;
struct c3;

typedef visit<c1,c2,c3> the_visit;

struct base : public visitable_base<the_visit> {};

template <typename T> using the_visitable = visitable<T, base>;

struct c1 : public the_visitable<c1> {};
struct c2 : public the_visitable<c2> {};
struct c3 : public the_visitable<c3> {};

struct some_visitor : public visitor<some_visitor,int,the_visit>
{
    int process(c1 & visitable) { cout << "c1" << endl; return 1; }
    int process(c2 & visitable) { cout << "c2" << endl; return 2; }
    int process(c3 & visitable) { cout << "c3" << endl; return 3; }
};

int main()
{
    c1 x;
    c2 y;
    c3 z;
    some_visitor v;

    {
        base &t = x;
        cout << v.visit(t) << endl;
    }
    {
        base &t = y;
        cout << v.visit(t) << endl;
    }
    {
        base &t = z;
        cout << v.visit(t) << endl;
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
