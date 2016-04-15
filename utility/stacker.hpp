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

}

#endif // STREAM_LANG_STACKER_INCLUDED
