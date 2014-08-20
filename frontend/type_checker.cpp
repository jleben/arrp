#include "type_checker.hpp"
#include "types.hpp"

#include <stdexcept>
#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

struct type_error : public error
{
    using error::error;
};

class wrong_arg_count_error : public type_error
{
public:
    wrong_arg_count_error(int required, int actual):
        type_error(msg(required,actual))
    {}
private:
    string msg(int required, int actual)
    {
        ostringstream text;
        text << " Wrong number of arguments ("
             << "required: " << required << ", "
             << "actual: " << actual
             << ")."
                ;
        return text.str();
    }
};

class invalid_types_error : public type_error
{
public:
    invalid_types_error( const string & what, const vector<type_ptr> & types ):
        type_error(msg(what, types))
    {}
private:
    static string msg(const string & what, const vector<type_ptr> & types)
    {
        ostringstream text;
        text << what << " ";
        int i = 0;
        for (const auto & type : types)
        {
            ++i;
            text << *type;
            if (i < types.size())
                text << ", ";
        }
        return text.str();
    }
};

class call_error : public source_error
{
public:
    call_error( const string & name, const string & what, int line ):
        source_error(msg(name, what), line)
    {}
private:
    static string msg(const string & name, const string & what)
    {
        ostringstream text;
        text << "In call to function '" << name << "': ";
        text << what;
        return text.str();
    }
};

type_checker::type_checker(environment &env):
    m_env(env),
    m_func_counter(0),
    m_has_error(false)
{

}

type_ptr type_checker::check( const symbol & sym, const vector<type_ptr> & args )
{
    m_has_error = false;

    context_type::scope_holder scope(m_ctx);

    m_root_scope = m_ctx.current_scope();

    type_ptr result_type;

    try
    {
        type_ptr sym_type = symbol_type(sym);
        switch(sym_type->get_tag())
        {
        case type::function:
        case type::builtin_unary_func:
        case type::builtin_binary_func:
            result_type = process_function(sym_type, args, m_root_scope).second;
            break;
        default:
            result_type = sym_type;
        }
    }
    catch (type_error & e)
    {
        report(e);
    }
    catch (source_error & e)
    {
        report(e);
    }
    catch (abort_error & e)
    {}

    m_root_scope.clear();

    return result_type;
}

#if 0
type_ptr type_checker::symbol_type( const symbol & sym, const vector<type_ptr> & args )
{
    switch(sym.type)
    {
    case symbol::expression:
        return process_expression(sym.expression);
    case symbol::function:
        return process_function(sym.expression, sym.parameter_names, args,
                                m_root_scope);
    case symbol::builtin_unary_math:
        if (args.size() != 1)
            throw wrong_arg_count_error(1, arg.size());
        return builtin_unary_func_type(args[0]);
    case symbol::builtin_binary_math:
        if (args.size() != 2)
            throw wrong_arg_count_error(2, arg.size());
        return builtin_binary_func_type(args[0], args[1]);
    }
}
#endif

type_ptr type_checker::symbol_type( const symbol & sym )
{
    if (sym.source && sym.source->semantic_type)
    {
        return sym.source->semantic_type;
    }

    switch(sym.type)
    {
    case symbol::expression:
    {
        context_type::scope_holder(m_ctx, m_root_scope);
        type_ptr t = process_block(sym.source->as_list()->elements[2]);
        sym.source->semantic_type = t;
        return t;
    }
    case symbol::function:
    {
        function *f = new function;
        f->name = sym.name;
        f->parameters = sym.parameter_names;
        f->statement = sym.source;
        type_ptr t(f);
        sym.source->semantic_type = t;
        return t;
    }
    case symbol::builtin_unary_math:
        return make_shared<type>( type::builtin_unary_func );
    case symbol::builtin_binary_math:
        return make_shared<type>( type::builtin_binary_func );
    }
}

pair<type_ptr, type_ptr>
type_checker::process_function( const type_ptr & func_type,
                                const vector<type_ptr> & args,
                                context_type::scope_iterator scope )
{
    switch (func_type->get_tag())
    {
    case type::builtin_unary_func:
    {
        if (args.size() != 1)
            throw wrong_arg_count_error(1, args.size());
        return make_pair(builtin_unary_func_type(args[0]), type_ptr());
    }
    case type::builtin_binary_func:
    {
        if (args.size() != 2)
            throw wrong_arg_count_error(2, args.size());
        return make_pair(builtin_binary_func_type(args[0], args[1]), type_ptr());
    }
    case type::function:
    {
        function &f = func_type->as<function>();
        if (args.size() != f.parameters.size())
            throw wrong_arg_count_error(f.parameters.size(), args.size());

        // Duplicate function in its static scope

        type_ptr f2_type = make_shared<function>();
        function &f2 = f2_type->as<function>();
        f2.name = generate_func_name(f.name);
        f2.parameters = f.parameters;
        f2.statement = f.statement->clone();
        f2.statement->semantic_type = f2_type;
        f2.statement->as_list()->elements[0]->as_leaf<string>()->value = f2.name;

        if (scope == m_ctx.root_scope())
        {
            symbol sym(symbol::function, f2.name);
            sym.parameter_names = f2.parameters;
            sym.source = f2.statement;
            m_env.emplace(sym.name, sym);
        }
        else
        {
            f2.statement_list = f.statement_list;
            f2.statement_list->as_list()->append( f2.statement );
            scope->emplace(f2.name, f2_type);
        }

        // Process the duplicate function

        context_type::scope_holder func_scope(m_ctx, scope);

        for (int i = 0; i < args.size(); ++i)
        {
            m_ctx.bind(f2.parameters[i], args[i]);
        }

        type_ptr result_type =
                process_block(f2.statement->as_list()->elements[2]);

        return make_pair(result_type, f2_type);
    }
    default:
        throw type_error("Callee not a function.");
    }
}

type_ptr type_checker::builtin_unary_func_type(const type_ptr & arg )
{
    switch(arg->get_tag())
    {
    case type::integer_num:
    case type::real_num:
        return make_shared<real_num>();
    case type::range:
    {
        range & r = arg->as<range>();
        if (r.is_constant())
            return make_shared<stream>(r.const_size());
        else
            // FIXME
            throw type_error("Non-constant range size not supported.");
    }
    case type::stream:
        return make_shared<stream>(arg->as<stream>());
    }
}

type_ptr type_checker::builtin_binary_func_type(const type_ptr & arg1, const type_ptr & arg2 )
{
    switch(arg1->get_tag())
    {
    case type::integer_num:
    case type::real_num:
        switch(arg2->get_tag())
        {
        case type::integer_num:
        case type::real_num:
            return make_shared<real_num>();
        case type::range:
        {
            range & r = arg2->as<range>();
            if (r.is_constant())
                return make_shared<stream>(r.const_size());
            else
                // FIXME
                throw type_error("Non-constant range size not supported.");
        }
        case type::stream:
            return make_shared<stream>( arg2->as<stream>() );
        }
    case type::range:
        switch(arg2->get_tag())
        {
        case type::integer_num:
        case type::real_num:
        {
            range & r = arg1->as<range>();
            if (r.is_constant())
                return make_shared<stream>(r.const_size());
            else
                // FIXME
                throw type_error("Non-constant range size not supported.");
        }
        default:
            throw invalid_types_error("Incompatible argument types:", {arg1, arg2});
        }
    case type::stream:
        switch(arg2->get_tag())
        {
        case type::integer_num:
        case type::real_num:
            return make_shared<stream>( arg1->as<stream>() );
        case type::range:
            throw invalid_types_error("Incompatible argument types:", {arg1, arg2});
        case type::stream:
            if (arg1->as<stream>().size != arg2->as<stream>().size)
            {
                string what("Incompatible argument types: streams of different sizes:");
                throw invalid_types_error(what, {arg1, arg2} ) ;
            }
            return make_shared<stream>( arg1->as<stream>() );
        }
    }
}

type_ptr type_checker::process_block( const sp<ast::node> & root )
{
    assert(root->type == ast::expression_block);
    ast::list_node *expr_block = root->as_list();
    assert(expr_block->elements.size() == 2);
    const auto & stmts = expr_block->elements[0];
    const auto & expr = expr_block->elements[1];

    if (stmts)
        process_stmt_list( stmts );

    type_ptr t = process_expression( expr );
    root->semantic_type = t;
    return t;
}

void type_checker::process_stmt_list( const ast::node_ptr & root )
{
    assert(root->type == ast::statement_list || root->type == ast::program);
    ast::list_node *stmts = root->as_list();
    for ( const sp<ast::node> & stmt : stmts->elements )
    {
        process_stmt(stmt, root);
    }
}

void type_checker::process_stmt( const sp<ast::node> & root, const ast::node_ptr & list )
{
    ast::list_node *stmt = root->as_list();
    const auto & id_node = stmt->elements[0];
    const auto & params_node = stmt->elements[1];
    const auto & expr_node = stmt->elements[2];

    const string & id = id_node->as_leaf<string>()->value;

    type_ptr result_type;

    if (params_node)
    {
        vector<string> parameters;
        ast::list_node *param_list = params_node->as_list();
        for ( const auto & param : param_list->elements )
        {
            string param_name = param->as_leaf<string>()->value;
            parameters.push_back(param_name);
        }

        function *f = new function;
        f->name = id;
        f->parameters = parameters;
        f->statement_list = list;
        f->statement = root;

        result_type = type_ptr(f);
    }
    else
    {
        result_type = process_block(expr_node);
    }

    root->semantic_type = result_type;

    m_ctx.bind(id, result_type);
}

type_ptr type_checker::process_expression( const sp<ast::node> & root )
{
    type_ptr expr_type;
    switch(root->type)
    {
    case ast::integer_num:
        expr_type = make_shared<integer_num>(root->as_leaf<int>()->value);
        break;
    case ast::real_num:
        expr_type = make_shared<real_num>( root->as_leaf<double>()->value );
        break;
    case ast::identifier:
        expr_type =  process_identifier(root).first;
        break;
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
        expr_type =  process_binop(root);
        break;
    case ast::range:
        expr_type =  process_range(root);
        break;
    case ast::hash_expression:
        expr_type =  process_extent(root);
        break;
    case ast::transpose_expression:
        expr_type =  process_transpose(root);
        break;
    case ast::slice_expression:
        expr_type =  process_slice(root);
        break;
    case ast::call_expression:
        expr_type =  process_call(root);
        break;
    case ast::for_expression:
        expr_type =  process_iteration(root);
        break;
    case ast::reduce_expression:
        expr_type =  process_reduction(root);
        break;
    default:
        assert(false);
        throw source_error("Unsupported expression.", root->line);
    }

    root->semantic_type = expr_type;

    return expr_type;
}

pair<type_ptr, type_checker::context_type::scope_iterator>
type_checker::process_identifier( const sp<ast::node> & root )
{
    string id = root->as_leaf<string>()->value;
    if (context_type::item item = m_ctx.find(id))
    {
        return make_pair(item.value(), item.scope());
    }
    else
    {
        environment::const_iterator it = m_env.find(id);
        if (it != m_env.end())
        {
            const symbol & sym = it->second;
            type_ptr sym_type = symbol_type(sym);
            if (sym.type == symbol::expression)
            {
                // cache result:
                auto success = m_root_scope->emplace(id, sym_type);
                assert(success.second);
            }
            return make_pair(sym_type, m_root_scope);
        }
    }
    assert(false);
    throw source_error("Name not in scope.", root->line);
}

type_ptr type_checker::process_binop( const sp<ast::node> & root )
{
    ast::list_node *expr = root->as_list();
    type_ptr lhs_type = process_expression(expr->elements[0]);
    type_ptr rhs_type = process_expression(expr->elements[1]);

    vector<int> lhs_size;
    vector<int> rhs_size;

    auto reduce_type = [&]( const type_ptr & t, vector<int> & size ) -> type_ptr
    {
        switch(t->get_tag())
        {
        case type::range:
        {
            range & r = t->as<range>();
            if (!r.is_constant())
                throw source_error("Invalid range.", root->line);
            size = { r.const_size() };
            return make_shared<integer_num>();
        }
        case type::stream:
        {
            stream & s = t->as<stream>();
            size = s.size;
            return make_shared<real_num>();
        }
        default:
            return t;
        }
    };

    lhs_type = reduce_type(lhs_type, lhs_size);
    rhs_type = reduce_type(rhs_type, rhs_size);

    if (lhs_size.empty() && rhs_size.empty())
    {
        if (lhs_type->is(type::integer_num) && rhs_type->is(type::integer_num))
            return make_shared<integer_num>();
        else
            return make_shared<real_num>();
    }

    if (!lhs_size.empty() && !rhs_size.empty() && lhs_size != rhs_size)
    {
        ostringstream text;
        text << "Binary operator (" << root->type << "): "
             << "Operand size mismatch.";
        throw source_error(text.str(), root->line);
    }

    if (!lhs_size.empty())
        return make_shared<stream>(lhs_size);
    else
        return make_shared<stream>(rhs_size);
}

type_ptr type_checker::process_range( const sp<ast::node> & root )
{
    assert(root->type == ast::range);
    ast::list_node *range_node = root->as_list();
    const auto & start_node = range_node->elements[0];
    const auto & end_node = range_node->elements[1];

    range *r = new range;

    bool abort = false;
    if (start_node)
    {
        r->start = process_expression(start_node);
        if (r->start->get_tag() != type::integer_num)
        {
            report(source_error("Range start not an integer.", start_node->line));
            abort = true;
        }
    }
    if (end_node)
    {
        r->end = process_expression(end_node);
        if (r->end->get_tag() != type::integer_num)
        {
            report(source_error("Range end not an integer.", start_node->line));
            abort = true;
        }
    }

    if (abort)
        throw abort_error();

    return type_ptr(r);
}

type_ptr type_checker::process_extent( const sp<ast::node> & root )
{
    assert(root->type == ast::hash_expression);
    ast::list_node *range_node = root->as_list();
    const auto & object_node = range_node->elements[0];
    const auto & dim_node = range_node->elements[1];

    type_ptr object_type = process_expression(object_node);

    if (object_type->get_tag() != type::stream)
        throw source_error("Extent object not a stream.", object_node->line);

    int dim = 1;
    if (dim_node)
    {
        type_ptr dim_type = process_expression(dim_node);
        if (dim_type->get_tag() != type::integer_num)
            throw source_error("Dimension not an integer.", dim_node->line);
        integer_num *dim_int = static_cast<integer_num*>(dim_type.get());
        if (!dim_int->is_constant())
            throw source_error("Dimension not a constant.", dim_node->line);
        dim = dim_int->constant_value();
    }

    stream & s = object_type->as<stream>();

    if (dim < 1 || dim > s.dimensionality())
    {
        ostringstream msg;
        msg << "Dimension " << dim << " out of bounds.";
        throw source_error(msg.str(), object_node->line);
    }

    int size = s.size[dim-1];

    return make_shared<integer_num>(size);
}

type_ptr type_checker::process_transpose( const sp<ast::node> & root )
{
    assert(root->type == ast::transpose_expression);
    ast::list_node *root_list = root->as_list();
    const auto & object_node = root_list->elements[0];
    const auto & dims_node = root_list->elements[1];

    type_ptr object_type = process_expression(object_node);
    if (object_type->get_tag() != type::stream)
        throw source_error("Transpose object not a stream.", object_node->line);
    stream & object = object_type->as<stream>();

    ast::list_node *dims = dims_node->as_list();

    if (dims->elements.size() > object.dimensionality())
        throw source_error("Transposition has too many dimensions.", root->line);

    vector<bool> selected_dims(object.dimensionality(), false);

    vector<int> transposed_size;
    transposed_size.reserve(object.dimensionality());

    for ( const auto & dim_node : dims->elements )
    {
        int dim = dim_node->as_leaf<int>()->value;
        if (dim < 1 || dim > object.dimensionality())
            throw source_error("Dimension selector element out of bounds.", dim_node->line);
        if (selected_dims[dim-1])
            throw source_error("Duplicate dimension selector element.", dim_node->line);
        transposed_size.push_back( object.size[dim-1] );
        selected_dims[dim-1] = true;
    }

    for (int dim = 0; dim < selected_dims.size(); ++dim)
    {
        if (!selected_dims[dim])
            transposed_size.push_back( object.size[dim] );
    }

    return make_shared<stream>( std::move(transposed_size) );
}

type_ptr type_checker::process_slice( const sp<ast::node> & root )
{
    assert(root->type == ast::slice_expression);
    const auto & object_node = root->as_list()->elements[0];
    const auto & ranges_node = root->as_list()->elements[1];

    type_ptr object_type = process_expression(object_node);
    if (object_type->get_tag() != type::stream)
        throw source_error("Slice object not a stream.", object_node->line);

    stream result_type(object_type->as<stream>());

    ast::list_node *range_list = ranges_node->as_list();

    if (range_list->elements.size() > result_type.dimensionality())
        throw source_error("Too many slice dimensions.", ranges_node->line);

    int dim = 0;
    for( const auto & range_node : range_list->elements )
    {
        sp<type> selector = process_expression(range_node);
        switch(selector->get_tag())
        {
        case type::integer_num:
        {
            // TODO: Range checking? Required a constant.
            result_type.size[dim] = 1;
            break;
        }
        case type::range:
        {
            range &r = selector->as<range>();
            if (!r.start)
                r.start = make_shared<integer_num>(1);
            if (!r.end)
                r.end = make_shared<integer_num>(result_type.size[dim]);
            if (!r.is_constant())
                throw source_error("Non-constant slice size not supported.", range_node->line);
            int start = r.const_start();
            int end = r.const_end();
            int size = end - start + 1;
            if (size < 1)
                throw source_error("Invalid slice range: size less than 1.", range_node->line);
            if (start < 1 || end > result_type.size[dim])
                throw source_error("Invalid slice range: out of bounds.", range_node->line);
            result_type.size[dim] = size;
            break;
        }
        default:
            throw source_error("Invalid type of slice selector.", range_node->line);
        }
        ++dim;
    }

    return result_type.reduced();
}

type_ptr type_checker::process_call( const sp<ast::node> & root )
{
    assert(root->type == ast::call_expression);

    ast::list_node * call = root->as_list();
    const auto & func_node = call->elements[0];
    const auto & args_node = call->elements[1];

    if (func_node->type != ast::identifier)
        throw source_error("Function call object not a function.", root->line);

    // Get function

    auto func_info = process_identifier(func_node);
    type_ptr func_type = func_info.first;
    auto & func_scope = func_info.second;

    // Get args

    std::vector<type_ptr> arg_types;
    ast::list_node * arg_list = args_node->as_list();
    for (const auto & arg_node : arg_list->elements)
    {
        arg_types.push_back( process_expression(arg_node) );
    }

    // Process function

    pair<type_ptr,type_ptr> result;
    try
    {
        result = process_function(func_type, arg_types, func_scope);
    }
    catch (type_error & e)
    {
        throw source_error(e.what(), root->line);
    }

    if (result.second)
    {
        function &new_func = result.second->as<function>();
        func_node->as_leaf<string>()->value = new_func.name;
    }

    return result.first;
}

type_ptr type_checker::process_iteration( const sp<ast::node> & root )
{
    //cout << "+++ iteration +++" << endl;

    assert(root->type == ast::for_expression);
    ast::list_node *iteration = root->as_list();
    assert(iteration->elements.size() == 2);

    assert(iteration->elements[0]->type == ast::for_iteration_list);
    ast::list_node *iterator_list = iteration->elements[0]->as_list();

    vector<type_ptr> iterators;
    iterators.reserve(iterator_list->elements.size());

    for( const sp<ast::node> & e : iterator_list->elements )
    {
        iterators.emplace_back( process_iterator(e) );
    }

    assert(!iterators.empty());

    int iteration_count = 0;

    for( const auto & t : iterators )
    {
        iterator & it = t->as<iterator>();
        if (!iteration_count)
            iteration_count = it.count;
        else if (it.count != iteration_count)
            throw source_error("Iterations with differing counts.", root->line);
    }

    type_ptr result_type;
    {
        context_type::scope_holder iteration_scope(m_ctx);
        for( const auto & t : iterators )
        {
            iterator & it = t->as<iterator>();
            //cout << "iterator " << it.id << ": " << *it.value_type << endl;
            m_ctx.bind(it.id, it.value_type);
        }
        result_type = process_block(iteration->elements[1]);
    }

    stream product_stream(iteration_count);

    switch(result_type->get_tag())
    {
    case type::stream:
    {
        stream &result_stream = result_type->as<stream>();
        product_stream.size.insert( product_stream.size.end(),
                                    result_stream.size.begin(),
                                    result_stream.size.end() );
        break;
    }
    case type::integer_num:
    case type::real_num:
        break;
    default:
        throw source_error("Unsupported iteration result type.", iteration->elements[1]->line);
    }

    //cout << "result: " << *result_type << endl;
    //cout << "product: " << *product_type << endl;
    //cout << "--- iteration ---" << endl;

    return product_stream.reduced();
}

type_ptr type_checker::process_iterator( const sp<ast::node> & root )
{
    assert(root->type == ast::for_iteration);
    ast::list_node *iteration = root->as_list();
    assert(iteration->elements.size() == 4);

    type_ptr iter_type = make_shared<iterator>();
    iterator & it = iter_type->as<iterator>();

    if (iteration->elements[0])
    {
        assert(iteration->elements[0]->type == ast::identifier);
        it.id = iteration->elements[0]->as_leaf<string>()->value;
    }

    if (iteration->elements[1])
    {
        sp<ast::node> & node = iteration->elements[1];
        sp<type> val = process_expression(node);
        if (val->get_tag() != type::integer_num)
            throw source_error("Iteration size not an integer.", node->line);
        integer_num *i = static_cast<integer_num*>(val.get());
        if (!i->is_constant())
            throw source_error("Iteration size not a constant.", node->line);
        it.size = i->constant_value();
        if (it.size < 1)
            throw source_error("Invalid iteration size.", node->line);
    }

    if (iteration->elements[2])
    {
        sp<ast::node> & node = iteration->elements[2];
        sp<type> val = process_expression(node);
        if (val->get_tag() != type::integer_num)
            throw source_error("Iteration hop not an integer.",node->line);
        integer_num *i = static_cast<integer_num*>(val.get());
        if (!i->is_constant())
            throw source_error("Iteration hop not a constant.",node->line);
        it.hop = i->constant_value();
        if (it.hop < 1)
            throw source_error("Invalid hop size.",node->line);
    }

    sp<ast::node> & domain_node = iteration->elements[3];
    process_expression(domain_node);
    it.domain = domain_node;

    // Get domains size:

    const type_ptr & domain_type = it.domain->semantic_type;
    assert(domain_type);

    int domain_size;

    switch(domain_type->get_tag())
    {
    case type::stream:
    {
        stream & domain_stream = domain_type->as<stream>();
        assert(domain_stream.dimensionality());
        domain_size = domain_stream.size[0];

        stream operand_stream(domain_stream);
        operand_stream.size[0] = it.size;
        it.value_type = operand_stream.reduced();

        break;
    }
    case type::range:
    {
        range & domain_range = domain_type->as<range>();
        if (!domain_range.is_constant())
            throw source_error("Non-constant range not supported as iteration domain.",
                               domain_node->line);
        domain_size = domain_range.const_size();

        if (it.size > 1)
        {
            range *operand_range = new range;
            operand_range->start.reset(new integer_num);
            operand_range->end.reset(new integer_num);
            it.value_type = type_ptr(operand_range);
        }
        else
        {
            it.value_type = type_ptr(new integer_num);
        }

        break;
    }
    default:
        throw source_error("Unsupported iteration domain type.", iteration->line);
    }

    // Compute iteration count:

    int iterable_size = domain_size - it.size;
    if (iterable_size < 0)
        throw source_error("Iteration size larger than stream size.", iteration->line);
    if (iterable_size % it.hop != 0)
        throw source_error("Iteration does not cover stream size.", iteration->line);
    it.count = iterable_size / it.hop + 1;

    //cout << "Iterator: " << it.id << " " << it.count << " x " << it.size << endl;

    root->semantic_type = iter_type;

    return iter_type;
}

type_ptr type_checker::process_reduction( const sp<ast::node> & root )
{
    assert(root->type == ast::reduce_expression);
    const auto & id1_node = root->as_list()->elements[0];
    const auto & id2_node = root->as_list()->elements[1];
    const auto & domain_node = root->as_list()->elements[2];
    const auto & body_node = root->as_list()->elements[3];

    string id1 = id1_node->as_leaf<string>()->value;
    string id2 = id2_node->as_leaf<string>()->value;

    sp<type> domain_type = process_expression(domain_node);

    sp<type> val1;
    sp<type> val2;

    switch(domain_type->get_tag())
    {
    case type::stream:
    {
        if (domain_type->as<stream>().dimensionality() > 1)
            throw source_error("Reduction of streams with more than"
                               " 1 dimension not supported.",
                               root->line);

        val1 = val2 = make_shared<real_num>();
        break;
    }
#if 0
    case type::range:
    {
        val1 = val2 = make_shared<integer_num>();
        break;
    }
#endif
    default:
        throw source_error("Invalid reduction domain type.", root->line);
    }

    context_type::scope_holder reduction_scope(m_ctx);
    m_ctx.bind(id1, val1);
    m_ctx.bind(id2, val2);

    type_ptr result_type = process_block(body_node);

    if (!result_type->is(type::real_num))
        throw source_error("Reduction result type must be a real number.", root->line);

    // Whatever the result type, it will be converted to val1 type (real):
    return val1;
}

}
}
