#ifndef STREAM_LANG_TYPES_INCLUDED
#define STREAM_LANG_TYPES_INCLUDED

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <cassert>
#include <initializer_list>

namespace stream {

namespace ast {
class node;
using node_ptr = std::shared_ptr<node>;
}

namespace semantic {

using std::string;
using std::vector;
using std::ostream;
template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using up = std::unique_ptr<T>;

struct integer_num;
struct real_num;
struct stream;
struct range;

struct type
{
    enum tag
    {
        integer_num,
        real_num,
        range,
        stream,
        function,
        builtin_unary_func,
        builtin_binary_func
    };

    type(tag t): m_tag(t) {}
    virtual ~type() {}

    tag get_tag() const { return m_tag; }

    bool is(tag t) { return m_tag == t; }

    template<typename T>
    T & as() { return static_cast<T&>(*this); }

    virtual void print_on( ostream & s ) const
    {
        s << "<tag = " << m_tag << ">";
    }

private:
    tag m_tag;
};

using type_ptr = std::shared_ptr<type>;

inline ostream & operator<<( ostream & s, const type & t )
{
    t.print_on(s);
    return s;
}

template <type::tag TAG>
class tagged_type : public type
{
public:
    tagged_type(): type(TAG)
    {}
};

template<typename T>
struct scalar
{
    scalar():
        m_is_constant(false)
    {}

    scalar( const T & value ):
        m_is_constant(true),
        m_value(value)
    {}

    bool is_constant() const { return m_is_constant; }

    T constant_value() const
    {
        assert(m_is_constant);
        return m_value;
    }

    void set_constant( const T & value )
    {
        m_value = value;
        m_is_constant = true;
    }

private:
    bool m_is_constant;
    T m_value;
};

struct integer_num : public scalar<int>, public tagged_type<type::integer_num>
{
    integer_num() {}
    integer_num(int v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<i:" << constant_value() << ">";
        else
            s << "<i>";
    }
};

struct real_num : public scalar<double>, public tagged_type<type::real_num>
{
    real_num() {}
    real_num(double v): scalar(v) {}
    virtual void print_on( ostream & s ) const
    {
        if (is_constant())
            s << "<r:" << constant_value() << ">";
        else
            s << "<r>";
    }
};

struct range : public tagged_type<type::range>
{
    range() {}

    sp<type> start;
    sp<type> end;

    bool start_is_constant()
    {
        return !start || start->as<semantic::integer_num>().is_constant();
    }

    bool end_is_constant()
    {
        return !end || end->as<semantic::integer_num>().is_constant();
    }

    bool is_constant()
    {
        return start_is_constant() && end_is_constant();
    }

    int const_start()
    {
        return start->as<semantic::integer_num>().constant_value();
    }

    int const_end()
    {
        return end->as<semantic::integer_num>().constant_value();
    }

    int const_size()
    {
        return std::abs(const_end() - const_start()) + 1;
    }

    virtual void print_on( ostream & s ) const
    {
        s << "[";
        if (start)
            s << *start;
        s << "...";
        if (end)
            s << *end;
        s << "]";
    }
};

struct stream : public tagged_type<type::stream>
{
    stream( const vector<int> & s ) : size(s) {}
    stream( int s ) : size{s} {}
    int dimensionality() const { return size.size(); }
    vector<int> size;

    virtual void print_on( ostream & s ) const
    {
        s << "[";
        if (size.size())
            s << size.front();
        for (int i = 1; i < size.size(); ++i)
        {
            s << ',';
            s << size[i];
        }
        s << "]";
    }

    void reduce()
    {
        size.erase( std::remove(size.begin(), size.end(), 1), size.end() );
        if (size.empty())
            size.push_back(1);
    }
};

struct function : public tagged_type<type::function>
{
    string name;
    vector<string> parameters;
    ast::node_ptr statement_list;
    ast::node_ptr statement;

    virtual void print_on( ostream & s ) const
    {
        s << name;
        int p = 0;
        s << "(";
        for (const string & param : parameters)
        {
            s << param;
            if (p < parameters.size())
                s << ", ";
        }
        s << ")";
    }
};

struct node
{
    ast::node_ptr ast_node;
    type_ptr type;
};

} // namespace semantic
} // namespace stream

#endif // STREAM_LANG_TYPES_INCLUDED
