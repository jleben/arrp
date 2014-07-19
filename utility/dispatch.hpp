#ifndef DISPATCH_INTERFACE_INCLUDED
#define DISPATCH_INTERFACE_INCLUDED

#include <type_traits>
#include <cstddef>

namespace dispatch
{

using std::size_t;

// type collection

template <typename ...Ts>
struct group;

template <typename C>
struct group_size;

template <typename ...Ts>
struct group_size<group<Ts...>> : public std::integral_constant<size_t, sizeof...(Ts)>
{};

template <size_t I, typename C>
struct group_member;

template <typename T, typename ...Ts>
struct group_member<1, group<T, Ts...>>
{
    typedef T type;
};

template <size_t I, typename T, typename ...Ts>
struct group_member<I, group<T, Ts...>>
{
    typedef typename group_member<I-1,group<Ts...>>::type type;
};

// abstract dispatcher

template <typename C, size_t I>
struct abstract_dispatch_stage;

template <typename C>
struct abstract_dispatch_stage<C,1>
{
    virtual void dispatch( typename group_member<1,C>::type &) = 0;
};


template <typename C, size_t I>
struct abstract_dispatch_stage: public abstract_dispatch_stage<C,I-1>
{
    using abstract_dispatch_stage<C,I-1>::dispatch;
    virtual void dispatch( typename group_member<I,C>::type &) = 0;
};

template <typename C>
using abstract_dispatcher = abstract_dispatch_stage<C, group_size<C>::value>;

// dispatcher

template <typename F, typename C, int I>
struct dispatch_stage;

template <typename F, typename C>
struct dispatch_stage <F,C,1>: public abstract_dispatcher<C>
{
    typedef typename group_member<1,C>::type object_type;

    dispatch_stage( F functor ): m_functor(functor){}

    void dispatch( object_type & object) { m_functor(object); }

protected:
    F m_functor;
};

template <typename F, typename C, int I>
struct dispatch_stage: public dispatch_stage<F,C,I-1>
{
    typedef typename group_member<I,C>::type object_type;
    typedef dispatch_stage<F,C,I-1> base;
    using base::dispatch_stage;
    using base::dispatch;
    void dispatch( object_type & object ) { this->m_functor(object); }
};

template <typename F, typename C>
using dispatcher = dispatch_stage<F, C, group_size<C>::value>;

// dispatchable

template<typename C>
struct abstract_dispatchable
{
    typedef abstract_dispatcher<C> dispatcher_type;
    virtual void dispatch_by( dispatcher_type & ) = 0;
};

template <typename D, typename B>
struct dispatchable : public B
{
    void dispatch_by( typename B::dispatcher_type & d )
    {
        d.dispatch( static_cast<D&>(*this) );
    }
};

} // namespace dispatch

#endif // DISPATCH_INTERFACE_INCLUDED
