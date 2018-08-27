#pragma once

#include "types.hpp"
#include "built_in_types.hpp"
#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../common/func_model_printer.hpp"
#include "../utility/context.hpp"
#include "../utility/stacker.hpp"

#include <deque>
#include <unordered_map>

namespace arrp  {

using std::deque;
using std::unordered_map;

using namespace stream::functional;

class type_checker : stream::functional::visitor<type_ptr>
{
public:
    using var_ptr = stream::functional::var_ptr;

    type_checker(built_in_types * builtin);

    void process(const scope &);

private:
    built_in_types * m_builtin;
    stream::functional::printer m_printer;
};

}
