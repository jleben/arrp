#include "collect_names.hpp"

using namespace std;

namespace stream {
namespace cpp_gen {

bool collect_names(const expression_ptr & e, std::unordered_set<string> & names)
{
    // FIXME: Handle other expressions
    if (auto id = dynamic_pointer_cast<id_expression>(e))
    {
        names.insert(id->name);
        return false;
    }
    else if (auto unop = dynamic_pointer_cast<un_op_expression>(e))
    {
        collect_names(unop->rhs, names);
        return true;
    }
    else if (auto binop = dynamic_pointer_cast<bin_op_expression>(e))
    {
        collect_names(binop->lhs, names);
        collect_names(binop->rhs, names);
        return true;
    }
    else if (auto call = dynamic_pointer_cast<call_expression>(e))
    {
        for (auto & arg : call->args)
            collect_names(arg, names);
        return true;
    }

    return false;
}

}
}

