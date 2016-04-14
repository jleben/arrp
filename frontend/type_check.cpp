#include "type_check.hpp"
#include "error.hpp"

#include <cassert>

using namespace std;

namespace stream {
namespace functional {

string text(primitive_op op, const vector<primitive_type> & args)
{
    ostringstream text;
    text << op
        << " ( ";
    for (auto & t : args)
        text << t << " ";
    text << ")";
    return text.str();
}

void type_checker::process(const unordered_set<id_ptr> & ids)
{
    for (auto & id : ids)
        process(id);
}

void type_checker::process(id_ptr id)
{
    if (id->type != primitive_type::undefined)
        return;

    id->type = primitive_type::undefined;
    id->type = process(id->expr, id);
    if (id->type == primitive_type::undefined)
        throw source_error("Ambiguous result type.", id->location);
    auto arr = dynamic_pointer_cast<array>(id->expr);
    if ( arr && arr->is_recursive )
    {
        // Must re-check
        id->type = process(id->expr, id);
        assert(id->type != primitive_type::undefined);
    }
}

primitive_type type_checker::process(expr_ptr expr, id_ptr id)
{
    primitive_type type = primitive_type::undefined;

    if (auto i = dynamic_pointer_cast<constant<int>>(expr))
    {
        type = primitive_type::integer;
    }
    else if (auto d = dynamic_pointer_cast<constant<double>>(expr))
    {
        type = primitive_type::real;
    }
    else if (auto b = dynamic_pointer_cast<constant<bool>>(expr))
    {
        type = primitive_type::boolean;
    }
    else if (auto op = dynamic_pointer_cast<primitive>(expr))
    {
        vector<primitive_type> arg_types;
        for (const auto & arg : op->operands)
            arg_types.push_back(process(arg, id));
        try {
            type = result_type(op->kind, arg_types);
        }
        catch (no_type &)
        {
            string msg("Invalid argument types: ");
            msg += text(op->kind, arg_types);
            throw source_error(msg, op->location);
        }
        catch (ambiguous_type &)
        {
            string msg("Ambiguous function resolution:");
            msg += text(op->kind, arg_types);
            throw source_error(msg, op->location);
        }
    }
    else if (auto c = dynamic_pointer_cast<case_expr>(expr))
    {
        type = process(c->cases.front().second, id);

        for (int i=1; i < c->cases.size(); ++i)
        {
            primitive_type type2 = process(c->cases[i].second, id);
            try {
                type = common_type(type, type2);
            } catch (no_type &) {
                throw source_error("Incompatible case types.",
                                   c->location);
            }
        }
    }
    else if (auto arr = dynamic_pointer_cast<array>(expr))
    {
        type = process(arr->expr, id);
    }
    else if (auto app = dynamic_pointer_cast<array_app>(expr))
    {
        type = process(app->object, id);
    }
    else if (auto ref = dynamic_pointer_cast<reference>(expr))
    {
        if (auto id = dynamic_pointer_cast<identifier>(ref->var))
        {
            assert(id);
            process(id);
            type = id->type;
        }
        else if (auto v = dynamic_pointer_cast<array_var>(ref->var))
        {
            type = primitive_type::integer;
        }
        else
        {
            throw error("Unexpected reference type.");
        }
    }
    else if (auto self_ref = dynamic_pointer_cast<array_self_ref>(expr))
    {
        type = id->type;
    }
    else
    {
        throw source_error("Unexpected expression.", expr->location);
    }

    expr->type = type;

    return type;
}

}
}
