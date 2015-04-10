/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "type_checker.hpp"

#include <stdexcept>
#include <sstream>
#include <cmath>

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

type_ptr promote(const type_ptr & in, type::tag promotion)
{
    switch(promotion)
    {
    case type::boolean:
        if (!in->is(type::boolean))
            throw type_error("Invalid type promotion.");
        return in;
    case type::integer_num:
        if (!in->is(type::integer_num))
            throw type_error("Invalid type promotion.");
        return in;
    case type::real_num:
        if (in->is(type::real_num))
            return in;
        if (in->is(type::integer_num))
        {
            const integer_num & i = in->as<integer_num>();
            if (i.is_constant())
                return make_shared<real_num>(i.constant_value());
            else
                return make_shared<real_num>();
        }
        throw type_error("Invalid type promotion.");
    default:
        throw type_error("Invalid type promotion.");
    }
}

type::tag max_type(const vector<type::tag> & ins)
{
    if (ins.empty())
        throw error("Empty type list.");

    type::tag max_type_tag = ins[0];

    for (const type::tag & in : ins)
    {
        switch(in)
        {
        case type::boolean:
            if (max_type_tag != type::boolean)
                throw type_error("Incompatible types.");
            break;
        case type::integer_num:
            if (max_type_tag == type::boolean)
                throw type_error("Incompatible types.");
            break;
        case type::real_num:
            if (max_type_tag == type::boolean)
                throw type_error("Incompatible types.");
            max_type_tag = type::real_num;
            break;
        default:
            throw error("Unexpected type.");
        }
    }

    return max_type_tag;
}

type_checker::type_checker(environment &env):
    m_env(env),
    m_func_counter(0),
    m_has_error(false)
{
    m_ctx.enter_scope();

    {
        vector<pair<string,primitive_op>> primitives = {
            {"log", primitive_op::log},
            {"log2", primitive_op::log2},
            {"log10", primitive_op::log10},
            {"exp", primitive_op::exp},
            {"exp2", primitive_op::exp2},
            {"sqrt", primitive_op::sqrt},
            {"sin", primitive_op::sin},
            {"cos", primitive_op::cos},
            {"tan", primitive_op::tan},
            {"asin", primitive_op::asin},
            {"acos", primitive_op::acos},
            {"atan", primitive_op::atan}
        };
        for (const auto & primitive : primitives)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = primitive.first;
            f->op = primitive.second;
            f->overloads.push_back( function_signature({type::real_num}, type::real_num) );
            m_ctx.bind(f->name, type_ptr(f));
        }
    }
    {
        vector<pair<string,primitive_op>> primitives = {
            {"ceil", primitive_op::ceil},
            {"floor", primitive_op::floor}
        };
        for (const auto & primitive : primitives)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = primitive.first;
            f->op = primitive.second;
            f->overloads.push_back( function_signature({type::real_num}, type::integer_num) );
            m_ctx.bind(f->name, type_ptr(f));
        }
    }
    {
        builtin_function_group * f = new builtin_function_group;
        f->name = "abs";
        f->op = primitive_op::abs;
        f->overloads.push_back( function_signature({type::integer_num}, type::integer_num) );
        f->overloads.push_back( function_signature({type::real_num}, type::real_num) );
        m_ctx.bind(f->name, type_ptr(f));
    }
    {
        vector<pair<string,primitive_op>> primitives = {
            {"min", primitive_op::min},
            {"max", primitive_op::max},
            {"pow", primitive_op::raise}
        };
        for (const auto & primitive : primitives)
        {
            builtin_function_group * f = new builtin_function_group;
            f->name = primitive.first;
            f->op = primitive.second;
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
        context_type::scope_holder expr_scope(m_ctx, m_ctx.root_scope());
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
        throw type_error("Incompatible argument types.");
    }

    return *selected_candidate;
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
            ast::list_node *stmt_list = f2.statement_list->as_list();
            auto pos = std::find(stmt_list->elements.begin(),
                                 stmt_list->elements.end(),
                                 f.statement);
            assert(pos != stmt_list->elements.end());
            stmt_list->elements.insert(++pos, f2.statement);

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

        auto result = process_primitive(g, args);

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

    int base_ctx_level = m_ctx.level();

    if (stmts)
    {
        try
        {
            process_stmt_list( stmts );
        }
        catch(...)
        {
            m_ctx.roll_back_to(base_ctx_level);
            throw;
        }
    }

    type_ptr t = process_expression( expr );
    root->semantic_type = t;

    m_ctx.roll_back_to(base_ctx_level);

    return t;
}

void type_checker::process_stmt_list( const ast::node_ptr & root )
{
    assert(root->type == ast::statement_list || root->type == ast::program);
    ast::list_node *stmts = root->as_list();
    for ( const sp<ast::node> & stmt : stmts->elements )
    {
        process_stmt(stmt, root);
        if (stmt->semantic_type->is(semantic::type::function))
            m_ctx.enter_scope();
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
    case ast::boolean:
        expr_type = make_shared<boolean>(root->as_leaf<bool>()->value);
        break;
    case ast::integer_num:
        expr_type = make_shared<integer_num>(root->as_leaf<int>()->value);
        break;
    case ast::real_num:
        expr_type = make_shared<real_num>( root->as_leaf<double>()->value );
        break;
    case ast::identifier:
        expr_type =  process_identifier(root).first;
        break;
    case ast::oppose:
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
    case ast::modulo:
    case ast::raise:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::equal:
    case ast::not_equal:
    case ast::logic_and:
    case ast::logic_or:
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
    case ast::if_expression:
        expr_type = process_conditional(root);
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
            (void) success;
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
    func.op = primitive_op::negate;
    switch(root->type)
    {
    case ast::negate:
        func.overloads.push_back(function_signature({type::integer_num}, type::integer_num));
        func.overloads.push_back(function_signature({type::real_num}, type::real_num));
        break;
    case ast::oppose:
        func.overloads.push_back(function_signature({type::boolean}, type::boolean));
        break;
    default:
        throw error("Unexpected AST node type.");
    }

    try {
        auto result = process_primitive(func, {operand_type});
        return result.first;
    }
    catch (error & e)
    {
        throw source_error(string("Negate: ") + e.what(), root->line);
    }
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
    function_signature iib({type::integer_num, type::integer_num},
                           type::boolean);
    function_signature rrb({type::real_num, type::real_num},
                           type::boolean);
    function_signature bbb({type::boolean, type::boolean},
                           type::boolean);

    builtin_function_group func;

    switch(root->type)
    {
    case ast::equal:
    case ast::not_equal:
    case ast::lesser:
    case ast::greater:
    case ast::lesser_or_equal:
    case ast::greater_or_equal:
    case ast::logic_and:
    case ast::logic_or:
        if (!lhs_type->is_scalar() || !rhs_type->is_scalar())
            throw source_error("Operands to logical operator are not both scalars.", root->line);
    default:
        break;
    }

    switch(root->type)
    {
    case ast::equal:
        func.op = primitive_op::compare_eq; break;
    case ast::not_equal:
        func.op = primitive_op::compare_neq; break;
    case ast::lesser:
        func.op = primitive_op::compare_l; break;
    case ast::greater:
        func.op = primitive_op::compare_g; break;
    case ast::lesser_or_equal:
        func.op = primitive_op::compare_leq; break;
    case ast::greater_or_equal:
        func.op = primitive_op::compare_geq; break;
    case ast::logic_and:
        func.op = primitive_op::logic_and; break;
    case ast::logic_or:
        func.op = primitive_op::logic_or; break;
    case ast::add:
        func.op = primitive_op::add;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::subtract:
        func.op = primitive_op::subtract;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::multiply:
        func.op = primitive_op::multiply;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::divide:
        func.op = primitive_op::divide;
        func.overloads.push_back(rrr);
        break;
    case ast::divide_integer:
        func.op = primitive_op::divide_integer;
        func.overloads.push_back(iii);
        func.overloads.push_back(rri);
        break;
    case ast::modulo:
        func.op = primitive_op::modulo;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    case ast::raise:
        func.op = primitive_op::raise;
        func.overloads.push_back(iii);
        func.overloads.push_back(rrr);
        break;
    default:
        throw error("Unexpected AST node type.");
    }

    switch(func.op)
    {
    case primitive_op::compare_eq:
    case primitive_op::compare_neq:
        func.overloads.push_back(iib);
        func.overloads.push_back(rrb);
        func.overloads.push_back(bbb);
        break;
    case primitive_op::compare_g:
    case primitive_op::compare_l:
    case primitive_op::compare_geq:
    case primitive_op::compare_leq:
        func.overloads.push_back(iib);
        func.overloads.push_back(rrb);
        break;
    case primitive_op::logic_and:
    case primitive_op::logic_or:
        func.overloads.push_back(bbb);
    default:
        break;
    }

    try
    {
        auto result = process_primitive(func, {lhs_type, rhs_type});
        return result.first;
    }
    catch (error & e)
    {
        throw source_error(string("Binary operator: ") + e.what(), root->line);
    }
}

type_ptr type_checker::process_range( const sp<ast::node> & root )
{
    assert(root->type == ast::range);
    ast::list_node *range_node = root->as_list();
    const auto & start_node = range_node->elements[0];
    const auto & end_node = range_node->elements[1];

    if (!start_node || !end_node)
    {
        throw source_error("Range not finite.", root->line);
    }

    auto start_type = process_expression(start_node);
    auto end_type = process_expression(end_node);

    bool abort = false;
    if (start_type->get_tag() != type::integer_num)
    {
        report(source_error("Range start not an integer.", start_node->line));
        abort = true;
    }
    if (end_type->get_tag() != type::integer_num)
    {
        report(source_error("Range end not an integer.", end_node->line));
        abort = true;
    }
    if (abort)
        throw abort_error();

    integer_num & start_int = start_type->as<integer_num>();
    integer_num & end_int = end_type->as<integer_num>();

    if (!start_int.is_constant() || !end_int.is_constant())
    {
        throw source_error("Range bounds not constant.", root->line);
    }

    int start_value = start_int.constant_value();
    int end_value = end_int.constant_value();
    int size = std::abs(end_value - start_value) + 1;
    return make_shared<stream>(size, primitive_type::integer);
}

type_ptr type_checker::process_extent( const sp<ast::node> & root )
{
    assert(root->type == ast::hash_expression);
    const auto & object_node = root->as_list()->elements[0];
    const auto & dim_node = root->as_list()->elements[1];

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

    return make_shared<stream>( std::move(transposed_size), object.element_type );
}

type_ptr type_checker::process_slice( const sp<ast::node> & root )
{
    assert(root->type == ast::slice_expression);
    const auto & object_node = root->as_list()->elements[0];
    const auto & selectors_node = root->as_list()->elements[1];

    type_ptr object_type = process_expression(object_node);
    if (object_type->get_tag() != type::stream)
        throw source_error("Slice object not a stream.", object_node->line);

    const stream & source_stream = object_type->as<stream>();

    ast::list_node *selector_list = selectors_node->as_list();

    if (selector_list->elements.size() > source_stream.dimensionality())
        throw source_error("Too many slice dimensions.", selectors_node->line);

    stream result_stream(source_stream);
    int dim = 0;
    for( const auto & selector_node : selector_list->elements )
    {
        if (source_stream.size[dim] == stream::infinite)
        {
            throw source_error("Can not slice an infinite dimension.", selector_node->line);
        }

        if (selector_node->type == ast::range)
        {
            const auto & start_node = selector_node->as_list()->elements[0];
            const auto & end_node = selector_node->as_list()->elements[1];

            int start;
            int end;

            if (start_node)
            {
                auto start_type = process_expression(start_node);
                if (start_type->get_tag() != type::integer_num)
                    throw source_error("Slice range start not an integer.", start_node->line);
                auto & start_int = start_type->as<integer_num>();
                if (!start_int.is_constant())
                    throw source_error("Slice range start not constant.", start_node->line);
                start = start_int.constant_value();
            }
            else
            {
                start = 1;
            }

            if (end_node)
            {
                auto end_type = process_expression(end_node);
                if (end_type->get_tag() != type::integer_num)
                    throw source_error("Slice range start not an integer.", end_node->line);
                auto & end_int = end_type->as<integer_num>();
                if (!end_int.is_constant())
                    throw source_error("Slice range start not constant.", end_node->line);
                end = end_int.constant_value();
            }
            else
            {
                end = source_stream.size[dim];
            }

            int size = end - start + 1;
            if (size < 1)
                throw source_error("Slice range size less than 1.", selector_node->line);

            if (start < 1 || end > source_stream.size[dim])
                throw source_error("Slice range out of bounds.", selector_node->line);

            result_stream.size[dim] = size;
        }
        else
        {
            auto selector = process_expression(selector_node);
            if (selector->get_tag() != type::integer_num)
                throw source_error("Invalid type of slice selector.", selector_node->line);

            auto & selector_int = selector->as<integer_num>();
            if (!selector_int.is_constant())
                throw source_error("Slice selector not constant.", selector_node->line);

            int offset = selector_int.constant_value();
            if (offset < 1 || offset > source_stream.size[dim])
                throw source_error("Slice selector out of bounds.", selector_node->line);

            result_stream.size[dim] = 1;
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

type_ptr type_checker::process_conditional( const ast::node_ptr & root )
{
    assert(root->type == ast::if_expression);
    const auto & condition_node = root->as_list()->elements[0];
    const auto & true_node = root->as_list()->elements[1];
    const auto & false_node = root->as_list()->elements[2];

    auto condition_type = process_expression(condition_node);
    auto true_type = process_block(true_node);
    auto false_type = process_block(false_node);

    if (condition_type->get_tag() != type::boolean)
        throw source_error("Condition expression not a boolean.", condition_node->line);

    try
    {
        return true_type + false_type;
    }
    catch (undefined)
    {
        throw source_error("Incompatible types of true and false parts.", root->line);
    }
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

    type_structure ts = structure(result_type);

    vector<int> product_size { iteration_count };
    product_size.insert( product_size.end(),
                         ts.size.begin(),
                         ts.size.end() );

    type_ptr product_type = stream(product_size, ts.type).reduced();

    //cout << "result: " << *result_type << endl;
    //cout << "product: " << *product_type << endl;
    //cout << "--- iteration ---" << endl;

    return product_type;
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
type_checker::process_primitive( const builtin_function_group & group,
                                 const vector<type_ptr> & args )
{
    vector<type_structure> type_structs;

    for (const auto & arg : args)
    {
        type_structs.push_back( structure(arg) );
    }

    vector<type::tag> primitive_type_tags;
    for (const auto & ts : type_structs)
    {
        switch(ts.type)
        {
        case primitive_type::boolean:
            primitive_type_tags.push_back(type::boolean); break;
        case primitive_type::integer:
            primitive_type_tags.push_back(type::integer_num); break;
        case primitive_type::real:
            primitive_type_tags.push_back(type::real_num); break;
        }
    }

    const function_signature & signature =
            overload_resolution(group.overloads, primitive_type_tags);

    vector<int> result_size;
    for ( const auto & ts : type_structs )
    {
        const auto & arg_size = ts.size;
        if (result_size.empty())
            result_size = arg_size;
        else if (!arg_size.empty() && result_size != arg_size)
            throw type_error("Argument size mismatch.");
    }

    type_ptr result_type;

    if (!result_size.empty())
    {
        primitive_type result_primitive_type =
                primitive_type_for(signature.result);
        result_type = make_shared<stream>(result_size, result_primitive_type);
    }
    else
    {
        builtin_function f;
        f.op = group.op;
        f.signature = signature;

        result_type = constant_for(f, args);

        if (!result_type)
        {
            switch(signature.result)
            {
            case type::boolean:
                result_type = make_shared<boolean>();
                break;
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
template<> type::tag type_tag_for<bool>() { return type::boolean; }

template<typename T>
type_ptr type_for(const T & value);

template<>
type_ptr type_for<bool>(const bool & value)
{
    return make_shared<boolean>(value);
}

template<>
type_ptr type_for<int>(const int & value)
{
    return make_shared<integer_num>(value);
}

template<>
type_ptr type_for<double>(const double & value)
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
    case type::boolean:
        return type->as<boolean>().constant_value();
    default:
        assert(false);
        throw error("Type has no constant value.");
    }
}

template<primitive_op I>
struct primitive_processor;

template<>
struct primitive_processor<primitive_op::add>
{
    static int process(int a, int b) { return a + b; }
    static double process(double a, double b) { return a + b; }
};

template<>
struct primitive_processor<primitive_op::subtract>
{
    static int process(int a, int b) { return a - b; }
    static double process(double a, double b) { return a - b; }
};

template<>
struct primitive_processor<primitive_op::multiply>
{
    static int process(int a, int b) { return a * b; }
    static double process(double a, double b) { return a * b; }
};

template<>
struct primitive_processor<primitive_op::divide>
{
    static double process(double a, double b) { return a / b; }
};

template<>
struct primitive_processor<primitive_op::raise>
{
    static int process(int a, int b) { return std::pow(a, b); }
    static double process(double a, double b) { return std::pow(a, b); }
};

template<>
struct primitive_processor<primitive_op::negate>
{
    static bool process(bool a) { return !a; }
    static int process(int a) { return -a; }
    static double process(double a) { return -a; }
};

template<>
struct primitive_processor<primitive_op::abs>
{
    static int process(int a) { return std::abs(a); }
    static double process(double a) { return std::abs(a); }
};

template<>
struct primitive_processor<primitive_op::max>
{
    static int process(int a, int b) { return std::max(a, b); }
    static double process(double a, double b) { return std::max(a, b); }
};

template<>
struct primitive_processor<primitive_op::min>
{
    static int process(int a, int b) { return std::min(a, b); }
    static double process(double a, double b) { return std::min(a, b); }
};

template<>
struct primitive_processor<primitive_op::compare_eq>
{
    static bool process(int a, int b) { return a == b; }
    static bool process(double a, double b) { return a == b; }
    static bool process(bool a, bool b) { return a == b; }
};

template<>
struct primitive_processor<primitive_op::compare_neq>
{
    static bool process(int a, int b) { return a != b; }
    static bool process(double a, double b) { return a != b; }
    static bool process(bool a, bool b) { return a != b; }
};

template<>
struct primitive_processor<primitive_op::compare_l>
{
    static bool process(int a, int b) { return a < b; }
    static bool process(double a, double b) { return a < b; }
};

template<>
struct primitive_processor<primitive_op::compare_leq>
{
    static bool process(int a, int b) { return a <= b; }
    static bool process(double a, double b) { return a <= b; }
};

template<>
struct primitive_processor<primitive_op::compare_g>
{
    static bool process(int a, int b) { return a > b; }
    static bool process(double a, double b) { return a > b; }
};

template<>
struct primitive_processor<primitive_op::compare_geq>
{
    static bool process(int a, int b) { return a >= b; }
    static bool process(double a, double b) { return a >= b; }
};

template<primitive_op I, typename R, typename ...A>
struct primitive_for
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
        R result = primitive_processor<I>::process(const_val<A>(args)...);
        return type_for<R>(result);
    }
};


template<primitive_op I>
type_ptr const_arithmetic( const function_signature & func,
                           const vector<type_ptr> & args)
{
    type_ptr result;
    bool ok =
            primitive_for<I,int,int,int>::try_compute(result, func, args[0], args[1]) ||
            primitive_for<I,double,double,double>::try_compute(result, func, args[0], args[1]);
    if (ok)
        return result;
    else
        return type_ptr();
}

template<primitive_op I>
type_ptr const_unary_arithmetic( const function_signature & func,
                                 const type_ptr & arg)
{
    type_ptr result;
    primitive_for<I,bool,bool>::try_compute(result, func, arg) ||
            primitive_for<I,int,int>::try_compute(result, func, arg) ||
            primitive_for<I,double,double>::try_compute(result, func, arg);
    return result;
}

template<primitive_op I>
type_ptr const_compare_eq( const function_signature & func,
                           const vector<type_ptr> & args )
{
    type_ptr result;
    primitive_for<I,bool,bool,bool>
            ::try_compute(result, func, args[0], args[1]) ||
            primitive_for<I,bool,int,int>
            ::try_compute(result, func,  args[0], args[1]) ||
            primitive_for<I,bool,double,double>
            ::try_compute(result, func,  args[0], args[1]);
    return result;
}

template<primitive_op I>
type_ptr const_compare_arithmetic( const function_signature & func,
                                   const vector<type_ptr> & args )
{
    type_ptr result;
    primitive_for<I,bool,int,int>::try_compute(result, func, args[0], args[1]) ||
            primitive_for<I,bool,double,double>::try_compute(result, func, args[0], args[1]);
    return result;
}

type_ptr type_checker::constant_for( const builtin_function & func,
                                     const vector<type_ptr> & args )
{
    for (const auto  & arg : args)
    {

        bool is_constant = arg->is_scalar() &&
                arg->as<basic_scalar>().is_constant();

        if (!is_constant)
            return type_ptr();
    }

    type_ptr result;
    switch(func.op)
    {
    case primitive_op::compare_eq:
        return const_compare_eq<primitive_op::compare_eq>(func.signature, args);
    case primitive_op::compare_neq:
        return const_compare_eq<primitive_op::compare_neq>(func.signature, args);
    case primitive_op::compare_g:
        return const_compare_arithmetic<primitive_op::compare_g>(func.signature, args);
    case primitive_op::compare_geq:
        return const_compare_arithmetic<primitive_op::compare_geq>(func.signature, args);
    case primitive_op::compare_l:
        return const_compare_arithmetic<primitive_op::compare_l>(func.signature, args);
    case primitive_op::compare_leq:
        return const_compare_arithmetic<primitive_op::compare_leq>(func.signature, args);
    case primitive_op::logic_and:
        return type_for(const_val<bool>(args[0]) && const_val<bool>(args[1]));
    case primitive_op::logic_or:
        return type_for(const_val<bool>(args[0]) || const_val<bool>(args[1]));
    case primitive_op::add:
        return const_arithmetic<primitive_op::add>(func.signature, args);
    case primitive_op::subtract:
        return const_arithmetic<primitive_op::subtract>(func.signature, args);
    case primitive_op::multiply:
        return const_arithmetic<primitive_op::multiply>(func.signature, args);
    case primitive_op::divide:
        return type_for(const_val<double>(args[0]) / const_val<double>(args[1]));
    case primitive_op::divide_integer:
        if (signature_is<int,int,int>(func.signature))
            return type_for((const_val<int>(args[0]) / const_val<int>(args[1])));
        else
            return type_for((int)(const_val<double>(args[0]) / const_val<double>(args[1])));
    case primitive_op::modulo:
    {
        if (signature_is<int,int,int>(func.signature))
        {
            // FIXME: handle floating point exception when divisor is 0
            int x = const_val<int>(args[0]);
            int y = const_val<int>(args[1]);
            int m = x%y;
            if ((m!=0) && ((m<0) != (y<0))) { m += y; }
            return type_for(m);
        }
        else // real % real -> real
        {
            double l = const_val<double>(args[0]);
            double r = const_val<double>(args[1]);
            double q = std::floor(l/r);
            return type_for(l - r * q);
        }
    }
    case primitive_op::raise:
        return const_arithmetic<primitive_op::raise>(func.signature, args);
    case primitive_op::negate:
        return const_unary_arithmetic<primitive_op::negate>(func.signature, args[0]);
    case primitive_op::log:
        return type_for( (double) std::log(const_val<double>(args[0])) );
    case primitive_op::log2:
        return type_for( (double) std::log2(const_val<double>(args[0])) );
    case primitive_op::log10:
        return type_for( (double) std::log10(const_val<double>(args[0])) );
    case primitive_op::exp:
        return type_for( (double) std::exp(const_val<double>(args[0])) );
    case primitive_op::exp2:
        return type_for( (double) std::exp2(const_val<double>(args[0])) );
    case primitive_op::sqrt:
        return type_for( (double) std::sqrt(const_val<double>(args[0])) );
    case primitive_op::cos:
        return type_for( (double) std::cos(const_val<double>(args[0])) );
    case primitive_op::tan:
        return type_for( (double) std::tan(const_val<double>(args[0])) );
    case primitive_op::asin:
        return type_for( (double) std::asin(const_val<double>(args[0])) );
    case primitive_op::acos:
        return type_for( (double) std::acos(const_val<double>(args[0])) );
    case primitive_op::atan:
        return type_for( (double) std::atan(const_val<double>(args[0])) );
    case primitive_op::ceil:
        return type_for( (int) std::ceil(const_val<double>(args[0])) );
    case primitive_op::floor:
        return type_for( (int) std::floor(const_val<double>(args[0])) );
    case primitive_op::abs:
        return const_unary_arithmetic<primitive_op::abs>(func.signature, args[0]);
    case primitive_op::max:
        return const_arithmetic<primitive_op::max>(func.signature, args);
    case primitive_op::min:
        return const_arithmetic<primitive_op::min>(func.signature, args);
    default:;
    }

    return result;
}

}
}
