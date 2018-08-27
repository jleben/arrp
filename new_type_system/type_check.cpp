#include "type_check.hpp"

#include <iostream>

using namespace stream::functional;
using namespace std;

namespace arrp {

type_checker::type_checker(built_in_types * builtin):
    m_builtin(builtin)
{}

void type_checker::process(const arrp::scope & scope)
{

}

}
