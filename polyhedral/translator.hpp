#ifndef STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
#define STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED

#include "model.hpp"
#include "../frontend/ast.hpp"
#include "../frontend/types.hpp"
#include "../frontend/context.hpp"
#include "../utility/matrix.hpp"

#include <deque>
#include <stack>

namespace stream {
namespace polyhedral {

using std::deque;
using std::stack;
using utility::matrix;

class translator
{
public:
    translator();

    void translate(const semantic::function & func,
                   const vector<semantic::type_ptr> & args);

    vector<statement*> & statements() { return m_statements; }

private:
    class symbol
    {
    public:
        symbol(expression *expr): source(expr) {}
        expression * source;
    };

    class stream_view : public expression
    {
    public:
        statement *target;
        mapping pattern;
        int current_iteration;
    };

    typedef stream::context<string,symbol> context;

    context m_context;
    deque<int> m_domain;
    vector<statement*> m_statements;


    void do_statement_list(const ast::node_ptr &node);
    void do_statement(const ast::node_ptr &node);
    expression * do_block(const ast::node_ptr &node);
    expression * do_expression(const ast::node_ptr &node);
    expression * do_identifier(const ast::node_ptr &node);
    expression * do_unary_op(const ast::node_ptr &node);
    expression * do_binary_op(const ast::node_ptr &node);
    expression * do_transpose(const ast::node_ptr &node);
    expression * do_slicing(const  ast::node_ptr &node);
    expression * do_mapping(const  ast::node_ptr &node);

    int current_dimension() const
    {
        return m_domain.size();
    }

    expression * translate_type(const semantic::type_ptr & type);

    mapping access(stream_view *source);

    stream_access * complete_access( stream_view * );
    stream_view * make_statement( expression *, const vector<int> & domain );

    void update_accesses(expression *, const mapping & map);


    //expression *complete_access( partial_stream_access * );
    //expression *partial_access( expression *, const vector<int> & additional_domain );
};

}
}

#endif // STREAM_POLYHEDRAL_TRANSLATOR_INCLUDED
