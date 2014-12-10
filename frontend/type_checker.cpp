#include "type_checker.hpp"
#include "types.hpp"

#include <stdexcept>
#include <sstream>

using namespace std;

namespace stream {
namespace semantic {

struct type_error : public error
{
    type_error(const string & msg): error(msg) {}
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
    m_ctx.enter_scope();

    {
        vector<pair<string,intrinsic::type>> intrinsics = {
            {"log", intrinsic::log},
            {"log2", intrinsic::log2},
            {"log10", intrinsic::log10},
            {"exp", intrinsic::exp},
            {"exp2", intrinsic::exp2},
            {"sqrt", intrinsic::sqrt},
            {"sin", intrinsic::sin},
            {"cos", intrinsic::cos},
            {"tan", intrinsic::tan},
            {"asin", intrinsic::asin},
            {"acos", intrinsic::acos},
            {"atan", intrinsic::atan}
        };
        for (const auto & intrinsic : intrinsics)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = intrinsic.first;
            f->intrinsic_type = intrinsic.second;
            f->overloads.push_back( function_signature({type::real_num}, type::real_num) );
            m_ctx.bind(f->name, type_ptr(f));
        }
    }
    {
        vector<pair<string,intrinsic::type>> intrinsics = {
            {"ceil", intrinsic::ceil},
            {"floor", intrinsic::floor}
        };
        for (const auto & intrinsic : intrinsics)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = intrinsic.first;
            f->intrinsic_type = intrinsic.second;
            f->overloads.push_back( function_signature({type::real_num}, type::integer_num) );
            m_ctx.bind(f->name, type_ptr(f));
        }
    }
    {
        builtin_function_group * f = new builtin_function_group;
        f->name = "abs";
        f->intrinsic_type = intrinsic::abs;
        f->overloads.push_back( function_signature({type::integer_num}, type::integer_num) );
        f->overloads.push_back( function_signature({type::real_num}, type::real_num) );
        m_ctx.bind(f->name, type_ptr(f));
    }
    {
        vector<pair<string,intrinsic::type>> intrinsics = {
            {"min", intrinsic::min},
            {"max", intrinsic::max},
            {"pow", intrinsic::raise}
        };
        for (const auto & intrinsic : intrinsics)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = intrinsic.first;
            f->intrinsic_type = intrinsic.second;
            f->overloads.push_back( function_signature({type::integer_num, type::integer_num},
                                                       type::integer_num) );
            f->overloads.push_back( function_signature({type::real_num, type::real_num},
                                                       type::real_num) );
            m_ctx.bind(f->name, type_ptr(f));
        }
    }
}

type_ptr type_checker::check( const symbol & sym, const vector<type_ptr> & args )
{
    m_has_error = false;

    type_ptr result_type;

    try
    {
        type_ptr sym_type = symbol_type(sym);

        func_type_ptr func_type = dynamic_pointer_cast<abstract_function>(sym_type);
        if (func_type)
            result_type = process_function(func_type, args, m_ctx.root_scope()).second;
        else
            result_type = sym_type;
    }
    catch (type_error & e)
    {
        report(e);
    }
    catch (source_error & e)
    {
        report(e);
    }
    catch (abort_error &)
    {}

    return result_type;
}

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
        context_type::scope_holder(m_ctx, m_ctx.root_scope());
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
    }
    throw error("Unexpected symbol type.");
}

const function_signature & overload_resolution
(const vector<function_signature> & overloads, const vector<type::tag> & args)
{
    const function_signature * selected_candidate = nullptr;

    for ( const function_signature & candidate : overloads )
    {
        if (candidate.parameters.size() != args.size())
            continue;

        bool ok = true;
        bool perfect = true;
        for (int p = 0; p < args.size(); ++p)
        {
            type::tag param = candidate.parameters[p];
            type::tag arg = args[p];
            if (param == arg)
                continue;
            perfect = false;
            if (arg == type::integer_num && param == type::real_num)
                continue;
            ok = false;
            break;
        }
        if (perfect)
        {
            selected_candidate = &candidate;
            break;
        }
        else if (ok)
        {
            if (selected_candidate)
                throw type_error("Ambiguous overloaded function call.");
            else
                selected_candidate = &candidate;
        }
    }

    if (!selected_candidate)
    {
        throw type_error("Invalid arguments.");
    }

    return *selected_candidate;
}

std::pair<type_ptr, vector<int> > inner_type( const type_ptr & t )
{
    switch(t->get_tag())
    {
    case type::range:
    {
        range & r = t->as<range>();
        if (!r.is_constant())
            throw type_error("Non-constant range used where constant range required.");
        vector<int> extent = { r.const_size() };
        return make_pair( make_shared<integer_num>(), extent );
    }
    case type::stream:
    {
        stream & s = t->as<stream>();
        return make_pair( make_shared<real_num>(), s.size );
    }
    default:
        return make_pair( t, vector<int>() );
    }
}

pair<type_ptr, func_type_ptr>
type_checker::process_function( const func_type_ptr & func_type,
                                const vector<type_ptr> & args,
                                context_type::scope_iterator scope )
{
    switch (func_type->get_tag())
    {
    case type::function:
    {
        function &f = func_type->as<function>();
        if (args.size() != f.parameters.size())
            throw wrong_arg_count_error(f.parameters.size(), args.size());

        // Duplicate function in its static scope

        func_type_ptr f2_type = make_shared<function>();
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
    case type::builtin_function_group:
    {
        builtin_function_group &g = func_type->as<builtin_function_group>();

        auto result = process_intrinsic(g, args);

        builtin_function *f = new builtin_function;
        f->name = g.name;
        f->signature = result.second;

        return make_pair(result.first, func_type_ptr(f));
    }
    default:
        throw type_error("Callee not a function.");
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
    case ast::negate:
    {
        expr_type = process_negate(root);
        break;
    }
    case ast::add:
    case ast::subtract:
    case ast::multiply:
    case ast::divide:
    case ast::divide_integer:
    case ast::raise:
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
            auto success = m_ctx.root_scope()->emplace(id, sym_type);
            assert(success.second);
            return make_pair(sym_type, m_ctx.root_scope());
        }
    }
    assert(false);
    throw source_error("Name not in scope.", root->line);
}

type_ptr type_checker::process_negate( const sp<ast::node> & root )
{
    type_ptr operand_type = process_expression(root->as_list()->elements[0]);

    builtin_function_group func;
    func.intrinsic_type = intrinsic::negate;
    func.overloads.push_back(function_signature({type::integer_num}, type::integer_num));
    func.overloads.push_back(function_signature({type::real_num}, type::real_num));

    auto result = process_intrinsic(func, {operand_type});

    return result.first;
}

type_ptr type_checker::process_binop( const sp<ast::node> & root )
{
    ast::list_node *expr = root->as_list();
    type_ptr lhs_type = process_expression(expr->elements[0]);
    type_ptr rhs_type = process_expression(expr->elements[1]);

    function_signature iii({type::integer_num, type::integer_num},
                           type::integer_num);
    function_signature rrr({type::real_num, type::real_num},
                           type::real_num);
    function_signature rri({type::real_num, type::real_num},
                           type::integer_num);


    builtin_function_group func;

    switch(root->type)
    {
    case ast::add:
        func.intrinsic_type = intrinsic::add;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::subtract:
        func.intrinsic_type = intrinsic::subtract;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::multiply:
        func.intrinsic_type = intrinsic::multiply;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::divide:
        func.intrinsic_type = intrinsic::divide;
        func.overloads.push_back(rrr);
        break;
    case ast::divide_integer:
        func.intrinsic_type = intrinsic::divide_integer;
        func.overloads.push_back(iii);
        func.overloads.push_back(rri);
        break;
    case ast::raise:
        func.intrinsic_type = intrinsic::raise;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    default:
        throw error("Unexpected AST node type.");
    }

    auto result = process_intrinsic(func, {lhs_type, rhs_type});

    return result.first;
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

    if (size == stream::infinite)
    {
        throw source_error("Extent in requested dimension is infinite.", root->line);
    }

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

    const stream & source_stream = object_type->as<stream>();

    ast::list_node *range_list = ranges_node->as_list();

    if (range_list->elements.size() > source_stream.dimensionality())
        throw source_error("Too many slice dimensions.", ranges_node->line);

    stream result_stream(source_stream);
    int dim = 0;
    for( const auto & range_node : range_list->elements )
    {
        if (source_stream.size[dim] == stream::infinite)
        {
            throw source_error("Can not slice an infinite dimension.", range_node->line);
        }

        sp<type> selector = process_expression(range_node);
        switch(selector->get_tag())
        {
        case type::integer_num:
        {
            // TODO: Range checking? Required a constant.
            result_stream.size[dim] = 1;
            break;
        }
        case type::range:
        {
            range &r = selector->as<range>();
            if (!r.start)
                r.start = make_shared<integer_num>(1);
            if (!r.end)
                r.end = make_shared<integer_num>(source_stream.size[dim]);
            if (!r.is_constant())
                throw source_error("Non-constant slice size not supported.", range_node->line);
            int start = r.const_start();
            int end = r.const_end();
            int size = end - start + 1;
            if (size < 1)
                throw source_error("Invalid slice range: size less than 1.", range_node->line);
            if (start < 1 || end > source_stream.size[dim])
                throw source_error("Invalid slice range: out of bounds.", range_node->line);
            result_stream.size[dim] = size;
            break;
        }
        default:
            throw source_error("Invalid type of slice selector.", range_node->line);
        }
        ++dim;
    }

    return result_stream.reduced();
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
    assert(func_info.first);

    func_type_ptr func_type = dynamic_pointer_cast<abstract_function>(func_info.first);
    if (!func_type)
    {
        ostringstream text;
        text << "Function call object not a function: "
             << "'" << func_node->as_leaf<string>()->value << "'";
        throw source_error(text.str(), root->line);
    };

    auto & func_scope = func_info.second;

    // Get args

    std::vector<type_ptr> arg_types;
    ast::list_node * arg_list = args_node->as_list();
    for (const auto & arg_node : arg_list->elements)
    {
        arg_types.push_back( process_expression(arg_node) );
    }

    // Process function

    pair<type_ptr, func_type_ptr> result;
    try
    {
        result = process_function(func_type, arg_types, func_scope);
    }
    catch (type_error & e)
    {
        ostringstream text;
        text << "In function call to '" << func_type->name << "': "
             << e.what();
        throw source_error(text.str(), root->line);
    }

    const func_type_ptr & func_instance = result.second;
    const type_ptr & result_type = result.first;
    assert(func_instance);

    func_node->semantic_type = func_instance;
    func_node->as_leaf<string>()->value = func_instance->name;

    return result_type;
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

    if (domain_size == stream::infinite)
    {
        it.count = stream::infinite;
    }
    else
    {
        int iterable_size = domain_size - std::max(it.size, it.hop);
        if (iterable_size < 0)
            throw source_error("Iteration size larger than stream size.", iteration->line);
        if (iterable_size % it.hop != 0)
            throw source_error("Iteration does not cover stream size.", iteration->line);
        it.count = iterable_size / it.hop + 1;
    }

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

pair<type_ptr, function_signature>
type_checker::process_intrinsic( const builtin_function_group & group,
                                 const vector<type_ptr> & args )
{
    vector<pair<type_ptr, vector<int>>> reduced_types;
    for (const auto & arg : args)
    {
        reduced_types.push_back( inner_type(arg) );
    }

    vector<type::tag> reduced_type_tags;
    for (const auto & rt : reduced_types)
        reduced_type_tags.push_back( rt.first->get_tag() );

    const function_signature & signature =
            overload_resolution(group.overloads, reduced_type_tags);

    vector<int> result_size;
    for ( const auto & rt : reduced_types )
    {
        const auto & arg_size = rt.second;
        if (result_size.empty())
            result_size = arg_size;
        else if (!arg_size.empty() && result_size != arg_size)
            throw type_error("Argument size mismatch.");
    }

    type_ptr result_type;

    if (!result_size.empty())
        result_type = make_shared<stream>(result_size);
    else
    {
        builtin_function f;
        f.intrinsic_type = group.intrinsic_type;
        f.signature = signature;

        result_type = constant_for(f, args);

        if (!result_type)
        {
            switch(signature.result)
            {
            case type::integer_num:
                result_type = make_shared<integer_num>();
                break;
            case type::real_num:
                result_type = make_shared<real_num>();
                break;
            default:
                assert(false);
            }
        }
    }

    return make_pair(result_type, signature);
}

template<typename T> type::tag type_tag_for();
template<> type::tag type_tag_for<int>() { return type::integer_num; }
template<> type::tag type_tag_for<double>() { return type::real_num; }

type_ptr type_for(int value)
{
    return make_shared<integer_num>(value);
}

type_ptr type_for(double value)
{
    return make_shared<real_num>(value);
}

template<typename R, typename ...A>
bool signature_is(const function_signature & signature)
{
    return signature.result == type_tag_for<R>() &&
            signature.parameters == vector<type::tag>({type_tag_for<A>()...});
}

template <typename T>
T const_val(const type_ptr & type)
{
    switch(type->get_tag())
    {
    case type::integer_num:
        return type->as<integer_num>().constant_value();
    case type::real_num:
        return type->as<real_num>().constant_value();
    default:
        assert(false);
    }
}

template<intrinsic::type I>
struct intrinsic_processor;

template<>
struct intrinsic_processor<intrinsic::add>
{
    static int process(int a, int b) { return a + b; }
    static double process(double a, double b) { return a + b; }
};

template<>
struct intrinsic_processor<intrinsic::subtract>
{
    static int process(int a, int b) { return a - b; }
    static double process(double a, double b) { return a - b; }
};

template<>
struct intrinsic_processor<intrinsic::multiply>
{
    static int process(int a, int b) { return a * b; }
    static double process(double a, double b) { return a * b; }
};

template<>
struct intrinsic_processor<intrinsic::divide>
{
    static double process(double a, double b) { return a / b; }
};

template<>
struct intrinsic_processor<intrinsic::raise>
{
    static int process(int a, int b) { return std::pow(a, b); }
    static double process(double a, double b) { return std::pow(a, b); }
};

template<>
struct intrinsic_processor<intrinsic::negate>
{
    static int process(int a) { return -a; }
    static double process(double a) { return -a; }
};

template<>
struct intrinsic_processor<intrinsic::abs>
{
    static int process(int a) { return std::abs(a); }
    static double process(double a) { return std::abs(a); }
};

template<>
struct intrinsic_processor<intrinsic::max>
{
    static int process(int a, int b) { return std::max(a, b); }
    static double process(double a, double b) { return std::max(a, b); }
};

template<>
struct intrinsic_processor<intrinsic::min>
{
    static int process(int a, int b) { return std::min(a, b); }
    static double process(double a, double b) { return std::min(a, b); }
};

template<intrinsic::type I, typename R, typename ...A>
struct intrinsic_for
{
    template <typename ...T> static
    bool try_compute(type_ptr & result,
                     const function_signature & func,
                     const T & ...args)
    {
        if (signature_is<R,A...>(func))
        {
            result = compute(args...);
            return true;
        }
        else
            return false;
    }

    template <typename ...T> static
    type_ptr compute(const T & ...args)
    {
        R result = intrinsic_processor<I>::process(const_val<A>(args)...);
        return type_for(result);
    }
};


template<intrinsic::type I>
type_ptr const_arithmetic( const function_signature & func,
                           const vector<type_ptr> & args)
{
    type_ptr result;
    bool ok =
            intrinsic_for<I,int,int,int>::try_compute(result, func, args[0], args[1]) ||
            intrinsic_for<I,double,double,double>::try_compute(result, func, args[0], args[1]);
    if (ok)
        return result;
    else
        return type_ptr();
}

template<intrinsic::type I>
type_ptr const_unary_arithmetic( const function_signature & func,
                                 const type_ptr & arg)
{
    type_ptr result;
    intrinsic_for<I,int,int>::try_compute(result, func, arg) ||
            intrinsic_for<I,double,double>::try_compute(result, func, arg);
    return result;
}

type_ptr type_checker::constant_for( const builtin_function & func,
                                     const vector<type_ptr> & args )
{
    for (const auto  & arg : args)
    {
        bool is_constant;
        switch(arg->get_tag())
        {
        case type::integer_num:
            is_constant = arg->as<integer_num>().is_constant();
            break;
        case type::real_num:
            is_constant = arg->as<real_num>().is_constant();
            break;
        default:
            is_constant = false;
        }
        if (!is_constant)
            return type_ptr();
    }

    type_ptr result;
    bool ok;
    switch(func.intrinsic_type)
    {
    case intrinsic::add:
        return const_arithmetic<intrinsic::add>(func.signature, args);
    case intrinsic::subtract:
        return const_arithmetic<intrinsic::subtract>(func.signature, args);
    case intrinsic::multiply:
        return const_arithmetic<intrinsic::multiply>(func.signature, args);
    case intrinsic::divide:
        return type_for(const_val<double>(args[0]) / const_val<double>(args[1]));
    case intrinsic::divide_integer:
        if (signature_is<int,int,int>(func.signature))
            return type_for((const_val<int>(args[0]) / const_val<int>(args[1])));
        else
            return type_for((int)(const_val<double>(args[0]) / const_val<double>(args[1])));
    case intrinsic::raise:
        return const_arithmetic<intrinsic::raise>(func.signature, args);
    case intrinsic::negate:
        return const_unary_arithmetic<intrinsic::negate>(func.signature, args[0]);
    case intrinsic::log:
        return type_for( (double) std::log(const_val<double>(args[0])) );
    case intrinsic::log2:
        return type_for( (double) std::log2(const_val<double>(args[0])) );
    case intrinsic::log10:
        return type_for( (double) std::log10(const_val<double>(args[0])) );
    case intrinsic::exp:
        return type_for( (double) std::exp(const_val<double>(args[0])) );
    case intrinsic::exp2:
        return type_for( (double) std::exp2(const_val<double>(args[0])) );
    case intrinsic::sqrt:
        return type_for( (double) std::sqrt(const_val<double>(args[0])) );
    case intrinsic::cos:
        return type_for( (double) std::cos(const_val<double>(args[0])) );
    case intrinsic::tan:
        return type_for( (double) std::tan(const_val<double>(args[0])) );
    case intrinsic::asin:
        return type_for( (double) std::asin(const_val<double>(args[0])) );
    case intrinsic::acos:
        return type_for( (double) std::acos(const_val<double>(args[0])) );
    case intrinsic::atan:
        return type_for( (double) std::atan(const_val<double>(args[0])) );
    case intrinsic::ceil:
        return type_for( (int) std::ceil(const_val<double>(args[0])) );
    case intrinsic::floor:
        return type_for( (int) std::floor(const_val<double>(args[0])) );
    case intrinsic::abs:
        return const_unary_arithmetic<intrinsic::abs>(func.signature, args[0]);
    case intrinsic::max:
        return const_arithmetic<intrinsic::max>(func.signature, args);
    case intrinsic::min:
        return const_arithmetic<intrinsic::min>(func.signature, args);
    default:;
    }

    return result;
}

}
}
