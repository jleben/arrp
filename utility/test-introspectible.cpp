#include <iostream>
#include <vector>
#include <tuple>

using namespace std;

enum zip {
    one,
    two,
    three
};


class node {};

template<typename ...Ts>
class tuple_node : public node
{
public:

    template<size_t I>
    const typename tuple_element<I, tuple<Ts...> >::type & get() const
    {
        return std::get<I>(d);
    }

    size_t size() const
    {
        return std::tuple_size<tuple<Ts...>>::value;
    }

protected:
    tuple<Ts...> d;
};

template<typename T, typename ...Ts>
struct tag_tuple_node : public tuple_node<Ts...>
{
    template<T tag>
    typename tuple_element<tag,tuple<Ts...> >::type & get()
    {
        return std::get<tag>(this->d);
    }
};

template<typename T>
class list_node : public node
{
public:
    const T & operator[](size_t i) { return d[i]; }
    size_t size() const { return d.size(); }
protected:
    vector<T> d;
};

template<typename T>
class mutable_list_node : public list_node<T>
{
public:
    float & operator[](size_t i) { return this->d[i]; }
    void append(float v) { this->d.push_back(v); }
};

struct my_tuple_node : public tag_tuple_node<zip,int,int,float>
{
};

struct my_list_node : public mutable_list_node<float>
{
};

#if 1
template<typename ...Ts>
void f( tuple_node<Ts...> & t )
{
    cout << "tuple<0>: " << t.get<0>() << endl;
}

template<typename T>
void f( list_node<T> & l )
{
    cout << "list[0]: " << l[0] << endl;
}
#if 1
void f( my_tuple_node & n )
{
    cout << "my<one>: " << n.get<one>() << endl;
}

void f( my_list_node & n )
{
    cout << "my[0]: " << n[0] << endl;
}
#endif
#endif
void f( node & n )
{
    cout << "node" << endl;
}


int main()
{
    my_tuple_node t;
    t.get<one>() = 2.5;

    my_list_node l;
    l.append(3.33);

    f(t);
    f(l);

    return 0;
}
