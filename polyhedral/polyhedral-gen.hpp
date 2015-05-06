#ifndef STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED
#define STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED

#include "../common/polyhedral_model.hpp"
#include "../utility/context.hpp"
#include "../common/ast.hpp"
#include "../common/environment.hpp"

#include <stack>

namespace stream {
namespace polyhedral {

using std::stack;

class model_generator
{
public:
    model_generator(const semantic::environment &);

    void generate(ast::node_ptr);

    model generate(const semantic::symbol &,
                   const vector<semantic::type_ptr> & args);

private:
    struct array_view
    {
        array_view() {}
        array_view(array_ptr a):
            array(a), relation(mapping::identity(a->size.size()))
        {}
        array_view(array_ptr a, const mapping & m):
            array(a), relation(m)
        {}
        array_ptr array;
        mapping relation;
        int current_in_dim = 0;
    };

    typedef std::shared_ptr<array_view> array_view_ptr;

    typedef stream::context<string, array_view_ptr> context;

    array_view_ptr generate_input(const semantic::type_ptr & type, int index);

    array_view_ptr generate_array(ast::node_ptr);
    expression_ptr generate_expression(ast::node_ptr);

    expression_ptr generate_block(ast::node_ptr);
    array_view_ptr generate_definition(ast::node_ptr);
    expression_ptr generate_call(ast::node_ptr);

    expression_ptr generate_id(ast::node_ptr);
    expression_ptr generate_range(ast::node_ptr);
    expression_ptr generate_slice(ast::node_ptr);
    expression_ptr generate_transpose(ast::node_ptr);
    expression_ptr generate_conditional(ast::node_ptr);
    expression_ptr generate_mapping(ast::node_ptr);
    expression_ptr generate_unary_op(ast::node_ptr);
    expression_ptr generate_binary_op(ast::node_ptr);

    mapping & transform() { return m_transform.top(); }

    array_ptr add_array(primitive_type);
    statement_ptr add_statement();

    const semantic::environment &m_env;
    model * m_model;
    context m_context;
    vector<int> m_domain;
    stack<mapping> m_transform;
};

}
}

#endif // STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED
