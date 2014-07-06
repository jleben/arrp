#ifndef STREAM_LANG_UTILTIY_VISITOR_INCLUDED
#define STREAM_LANG_UTILTIY_VISITOR_INCLUDED

#include <stdexcept>

namespace stream {

// abstract visit

template <typename ...Ts>
struct visit;

template <typename T>
struct visit<T>
{
    virtual void on(T &) { throw std::runtime_error("Unimplemented visit."); }
};

template <typename T, typename ...Ts>
struct visit<T,Ts...> : public visit<Ts...>
{
    using visit<Ts...>::on;
    virtual void on(T &) { throw std::runtime_error("Unimplemented visit."); }
};

// concrete visit

template <typename A, typename V, typename ...Ts>
struct visit_impl;

template <typename A, typename V, typename T>
struct visit_impl <A,V,T>: public V
{
    typedef typename A::result_type R;

    visit_impl( A & actor ): m_actor(actor){}
    A & actor() { return m_actor; }
    R & result() { return m_result; }

    void on(T & target) { m_result = m_actor.process(target); }

protected:
    R m_result;
    A & m_actor;
};

template <typename A, typename V, typename T, typename ...Ts>
struct visit_impl <A,V,T,Ts...>: public visit_impl<A,V,Ts...>
{
    typedef visit_impl<A,V,Ts...> base;
    using base::visit_impl;
    using base::on;
    using base::result;
    using base::actor;
    void on(T & target) { result() = actor().process(target); }
};

template <typename A, typename V>
struct concrete_visit;

template <typename A, typename ...Ts>
struct concrete_visit<A,visit<Ts...>> : public visit_impl<A,visit<Ts...>,Ts...>
{
    using visit_impl<A,visit<Ts...>,Ts...>::visit_impl;
};

// visitable

template<typename V>
struct visitable_base
{
    typedef V visit_type;
    virtual void accept( V & visit ) = 0;
};

template <typename D, typename B>
struct visitable : public B
{
    void accept( typename B::visit_type & visit )
    {
        visit.on( static_cast<D&>(*this) );
    }
};

// visitor

template <typename D, typename R, typename V>
struct visitor
{
    typedef R result_type;
    typedef V visit_type;

    R visit(visitable_base<V> & target)
    {
        concrete_visit<D,V> v( static_cast<D&>(*this) );
        target.accept(v);
        return v.result();
    }

    //virtual R process(T & target) = 0;
};

} // namespace stream

#endif // STREAM_LANG_UTILTIY_VISITOR_INCLUDED
