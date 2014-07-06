#ifndef STREAM_LANG_UTILTIY_VISITOR_INCLUDED
#define STREAM_LANG_UTILTIY_VISITOR_INCLUDED

#include <stdexcept>

namespace stream {

// visitor

template <typename ...Ts>
struct visitor;

template <typename T>
struct visitor<T>
{
    virtual void invalid() { throw std::runtime_error("Unimplemented visit."); }
    virtual void visit(T &) { invalid(); }
};

template <typename T, typename ...Ts>
struct visitor<T,Ts...> : public visitor<Ts...>
{
    using visitor<Ts...>::visit;
    using visitor<Ts...>::invalid;
    virtual void visit(T &) { invalid(); }
};

// visitable

template<typename V>
struct visitable_base
{
    typedef V visitor_type;
    virtual void accept( V & visitor ) = 0;
};

template <typename D, typename B>
struct visitable : public B
{
    void accept( typename B::visitor_type & visitor )
    {
        visitor.visit( static_cast<D&>(*this) );
    }
};

} // namespace stream

#endif // STREAM_LANG_UTILTIY_VISITOR_INCLUDED
