#ifndef STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED
#define STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED

#include "../common/polyhedral_model.hpp"
#include "../utility/context.hpp"
#include "../common/ast.hpp"
#include "../common/environment.hpp"
#include "../common/array_func_printer.hpp"

#include <stack>
#include <deque>

namespace stream {
namespace polyhedral {

using std::stack;
using std::deque;

class model_generator
{
public:
    model_generator(const semantic::environment &);

    void generate(ast::node_ptr);

    model generate(const semantic::symbol &,
                   const vector<semantic::type_ptr> & args);

private:
    struct array_view : expression
    {
        array_view():
            expression(primitive_type::integer) {}
        array_view(array_ptr a):
            expression(primitive_type::integer),
            array(a), relation(mapping::identity(a->size.size()))
        {}
        array_view(array_ptr a, const mapping & m):
            expression(primitive_type::integer),
            array(a), relation(m)
        {}
        array_ptr array;
        mapping relation;
        int current_in_dim = 0;
    };

    enum array_storage_mode
    {
        storage_required,
        storage_not_required
    };

    typedef std::shared_ptr<array_view> array_view_ptr;

    typedef stream::context<string, expression_ptr> context;
    typedef stream::context<array_var_ptr, array_index_expr> array_context;

    expression_ptr generate_input(const semantic::type_ptr & type, int index);

    expression_ptr generate_array(ast::node_ptr, array_storage_mode = storage_not_required);
    array_view_ptr generate_array_old(ast::node_ptr, array_storage_mode = storage_not_required);
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
    expression_ptr generate_reduction(ast::node_ptr);
    expression_ptr generate_unary_op(ast::node_ptr);
    expression_ptr generate_binary_op(ast::node_ptr);

    expression_ptr reduce(expression_ptr expr);
    array_index_vector reduce(const array_index_vector &);
    array_index_expr reduce(const array_index_expr &);

    mapping & transform() { return m_transform.top(); }

    expression_ptr apply(expression_ptr, const array_index_vector &);
    expression_ptr bind(const array_var_vector & vars, expression_ptr);
    array_ptr add_array(primitive_type);
    array_ptr add_array(primitive_type, const array_var_vector &);
    array_ptr add_array(primitive_type, const deque<array_var_ptr> &);
    statement_ptr add_statement();

    const semantic::environment &m_env;
    model * m_model;
    context m_context;
    array_context m_array_context;
    deque<array_var_ptr> m_bound_array_vars;
    vector<int> m_domain;
    stack<mapping> m_transform;
    array_func_printer m_printer;
};

}
}

#endif // STREAM_LANG_POLYHEDRAL_MODEL_GENERATOR_INCLUDED
