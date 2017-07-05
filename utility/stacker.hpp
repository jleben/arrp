#ifndef STREAM_LANG_STACKER_INCLUDED
#define STREAM_LANG_STACKER_INCLUDED

#include <stack>
#include <iostream>
#include <string>

namespace stream {

using std::stack;
using std::string;

template <typename T, typename S = std::stack<T> >
struct stacker
{
public:
    stacker(S & stack):
        m_stack(stack),
        m_start_count((int)stack.size())
    {}

    stacker(const T & val, S & stack):
        m_stack(stack),
        m_start_count((int)stack.size())
    {
        push(val);
    }
    ~stacker()
    {
        while (m_stack.size() > m_start_count)
        {
            m_stack.pop();
        }
    }
    void push(const T & val)
    {
        m_stack.push(val);
    }
private:
    S & m_stack;
    int m_start_count;
};

template <typename T, typename S>
stacker<T,S> stack_scoped(const T & val, S & stack)
{
    return stacker<T,S>(val, stack);
}

template <typename T, typename C>
struct scoped_inserter
{
public:
    scoped_inserter(const T & item, C & container):
        m_container(container), m_item(&item)
    {
        m_container.insert(item);
    }

    ~scoped_inserter()
    {
        if (m_item)
        {
            auto it = m_container.find(*m_item);
            if (it != m_container.end())
                m_container.erase(it);
        }
    }

    scoped_inserter(const scoped_inserter & other) = delete;

    void operator=(const scoped_inserter & other) = delete;

    scoped_inserter(scoped_inserter && other):
        m_container(other.m_container),
        m_item(other.m_item)
    {
        other.m_item = nullptr;
    }

private:
    C & m_container;
    const T * m_item = nullptr;
};

template <typename T, typename C> inline
scoped_inserter<T,C> insert_scoped(const T & i, C & c)
{
    return scoped_inserter<T,C>(i,c);
}

template <typename T, typename S = std::stack<T> >
class tracing_stack : public S
{
public:
    tracing_stack(const string & item_name):
        m_item_name(item_name)
    {}

    void push(const T & item)
    {
        using namespace std;

        if (m_enabled)
            cout << "+ " << m_item_name << ' ' << item << endl;

        S::push(item);
    }

    void pop()
    {
        using namespace std;

        if (m_enabled)
            cout << "- " << m_item_name << ' ' << S::top() << endl;

        S::pop();
    }

    void set_enabled(bool enabled) { m_enabled = enabled; }

private:
    string m_item_name;
    bool m_enabled = true;
};

template <typename C>
class stack_adapter : public C
{
public:
    using V = typename C::value_type;

    V & top() { return C::back(); }

    const V & top() const { return C::back(); }

    void push(const V & v)
    {
        C::push_back(v);
    }

    void pop()
    {
        C::pop_back();
    }
};

template <typename T>
class revertable
{
public:
    revertable(T & d): data(d), initial_value(d) {}
    revertable(T & d, T v): data(d), initial_value(d) { data = v; }
    ~revertable() { data = initial_value; }
    revertable & operator=(const T & v)
    {
        data = v;
        return *this;
    }

    T & data;
    T initial_value;
};

}

#endif // STREAM_LANG_STACKER_INCLUDED
