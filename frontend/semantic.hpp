#ifndef STREAM_LANG_SEMANTIC_INCLUDED
#define STREAM_LANG_SEMANTIC_INCLUDED

#include "ast.hpp"
#include "environment.hpp"
#include "types.hpp"

#include <string>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <memory>

namespace stream {
namespace semantic {

using std::string;
template <typename T> using sp = std::shared_ptr<T>;
template <typename T> using up = std::unique_ptr<T>;

struct iterator
{
    iterator(): hop(1), size(1), count(1) {}
    string id;
    int hop;
    int size;
    int count;
    sp<type> domain;
    sp<type> value;
};

sp<type> evaluate_expr_block( environment & env, const sp<ast::node> & root );
void evaluate_stmt_list( environment & env, const sp<ast::node> & root );
symbol * evaluate_statement( environment & env, const sp<ast::node> & root );
sp<type> evaluate_expression( environment & env, const sp<ast::node> & root );
sp<type> evaluate_binop( environment & env, const sp<ast::node> & root );
sp<type> evaluate_range( environment & env, const sp<ast::node> & root );
sp<type> evaluate_hash( environment & env, const sp<ast::node> & root );
sp<type> evaluate_call( environment & env, const sp<ast::node> & root );
sp<type> evaluate_iteration( environment & env, const sp<ast::node> & root );
sp<type> evaluate_reduction( environment & env, const sp<ast::node> & root );
iterator evaluate_iterator( environment & env, const sp<ast::node> & root );

struct semantic_error : public std::runtime_error
{
    semantic_error(const string & what, int line):
        std::runtime_error(what),
        m_line(line)
    {}
    int line() const { return m_line; }
    void report();
private:
    int m_line;
};

struct evaluation_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct call_error : public evaluation_error
{
    call_error(const string & name, const string & what):
        evaluation_error( message(name, what) )
    {}
private:
    static string message(const string & name, const string & what)
    {
        std::ostringstream msg;
        msg << "In call to '" << name << "': ";
        msg << what;
        return msg.str();
    }
};

struct wrong_arg_count : public call_error
{
    wrong_arg_count(const string & name, int required, int provided):
        call_error( name, message(required, provided) )
    {}
private:
    static string message(int required, int provided)
    {
        std::ostringstream msg;
        msg << "Wrong number of arguments ("
            << required << " required, "
            << provided << " provided"
            << ").";
        return msg.str();
    }
};

} // namespace semantic
} // namespace stream

#endif //  STREAM_LANG_SEMANTIC_INCLUDED
