#ifndef STREAM_LANG_STACKER_INCLUDED
#define STREAM_LANG_STACKER_INCLUDED

#include <stack>

namespace stream {

using std::stack;

template<typename T>
struct stacker
{
    stacker(const T & val, stack<T> & stack):
        m_stack(stack)
    {
        stack.push(val);
    }
    ~stacker()
    {
        m_stack.pop();
    }
private:
    stack<T> & m_stack;
};

}

#endif // STREAM_LANG_STACKER_INCLUDED
