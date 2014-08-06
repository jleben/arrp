#include "types.hpp"
#include "ast.hpp"

namespace stream {
namespace semantic {

ast::node_ptr function::expression() const
{
    return statement->as_list()->elements[2];
}

type_ptr function::result_type() const
{
    return expression()->semantic_type;
}


void function::print_on( ostream & s ) const
{
    s << name;
    s << " (";
    int p = 0;
    for (const string & param : parameters)
    {
        ++p;
        s << param;
        if (p < parameters.size())
            s << ", ";
    }
    s << ") -> ";
    type_ptr t = result_type();
    if (t)
        s << *t;
    else
        s << "?";
}

}
}
