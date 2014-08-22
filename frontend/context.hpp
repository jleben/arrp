#ifndef STREAM_LANG_CONTEXT_INCLUDED
#define STREAM_LANG_CONTEXT_INCLUDED

#include <unordered_map>
#include <deque>
#include <list>
#include <stdexcept>
#include <cassert>
#include <string>

namespace stream {

using std::unordered_map;
using std::deque;
using std::list;
using std::string;

struct context_error : public std::runtime_error
{
    context_error(const string & what): std::runtime_error(what) {}
};

template <typename K, typename V, typename S = unordered_map<K,V> >
class context
{
public:
    typedef K key_type;
    typedef V value_type;
    typedef S scope_type;

private:
    struct scope_link
    {
        scope_link(): parent(nullptr) {}
        scope_link(scope_link * parent): parent(parent) {}
        scope_type scope;
        scope_link *parent;
    };

    typedef list<scope_link> stack_type;

public:
    class item;

    class scope_iterator
    {
    public:
        scope_iterator(): link(nullptr) {}

        scope_type * operator->()
        {
            return &link->scope;
        }
        scope_type & operator*()
        {
            return link->scope;
        }
        scope_iterator parent()
        {
            return scope_iterator(link->parent);
        }
        bool operator==( const scope_iterator & other )
        {
            return link == other.link;
        }
        operator bool()
        {
            return link != nullptr;
        }
        void clear()
        {
            link = nullptr;
        }

    private:
        friend class context;
        friend class item;
        scope_iterator(scope_link *l): link(l) {}
        scope_link *link;
    };

    class item
    {
    public:
        item(): m_value(nullptr), m_scope(nullptr) {}
        V & value() { return *m_value; }
        scope_iterator scope() { return scope_iterator(m_scope); }
        operator bool()
        {
            return m_value != nullptr;
        }
    private:
        friend class context;
        item(V* value, scope_link *scope):
            m_value(value), m_scope(scope)
        {}
        V * m_value;
        scope_link * m_scope;
    };

    class scope_holder
    {
    public:
        scope_holder(context & ctx):
            m_ctx(ctx)
        {
            m_ctx.enter_scope();
        }
        scope_holder(context & ctx, const scope_iterator & scope):
            m_ctx(ctx)
        {
            m_ctx.enter_scope(scope);
        }
        ~scope_holder()
        {
            m_ctx.exit_scope();
        }
    private:
        context & m_ctx;
    };

    int level()
    {
        return m_stack.size();
    }

    void enter_scope()
    {
        if (m_stack.empty())
            m_stack.emplace_front();
        else
            m_stack.emplace_front(&m_stack.front());
    }

    void enter_scope( const scope_iterator & parent )
    {
        m_stack.emplace_front(parent.link);
    }

    void enter_isolated_scope()
    {
        m_stack.emplace_front();
    }

    void exit_scope()
    {
        m_stack.pop_front();
    }

    scope_iterator current_scope()
    {
        return scope_iterator( &m_stack.front() );
    }

    scope_iterator root_scope()
    {
        return scope_iterator( & m_stack.back() );
    }

    item find(const key_type & key)
    {
        if (m_stack.empty())
            return item();

        scope_link *link = &m_stack.front();
        while(link)
        {
            auto value_iter = link->scope.find(key);
            if (value_iter != link->scope.end())
            {
                return item(&value_iter->second, link);
            }
            link = link->parent;
        }

        return item();
    }

    void bind(const key_type & key, const value_type & value)
    {
        assert(!m_stack.empty());
        auto outcome = m_stack.front().scope.emplace(key, value);
        if (!outcome.second)
            throw context_error("Name already in scope.");
    }

    bool current_scope_has_bound(const key_type & key)
    {
        const scope_type & scope = m_stack.front().scope;
        return scope.find(key) != scope.end();
    }

    bool has_bound(const key_type & key)
    {
        if (m_stack.empty())
            return false;

        scope_link *link = &m_stack.front();
        while(link)
        {
            auto value_iter = link->scope.find(key);
            if (value_iter != link->scope.end())
            {
                return true;
            }
            link = link->parent;
        }

        return false;
    }

private:
    stack_type m_stack;
};

} // namespace stream

#endif // STREAM_LANG_CONTEXT_INCLUDED
