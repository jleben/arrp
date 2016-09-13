#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"

#include <isl-cpp/context.hpp>
#include <isl-cpp/space.hpp>
#include <isl-cpp/set.hpp>
#include <isl-cpp/expression.hpp>

namespace stream {
namespace functional {

struct space_map
{
    space_map(): space(nullptr) {}

    space_map(const isl::space & s,
              const vector<array_var_ptr> & v):
        space(s), vars(v) {}

    isl::space space;
    vector<array_var_ptr> vars;

    int index_of(array_var_ptr var) const
    {
        int i;
        for(i=0; i<vars.size(); ++i)
        {
            if (vars[i] == var)
                return i;
        }
        return -1;
    }

    isl::expression var(array_var_ptr var) const
    {
        int i = index_of(var);
        if (i >= 0)
            return space.var(i);
        else
            return isl::expression(nullptr);
    }
};

isl::set to_affine_set(expr_ptr, const space_map &);
isl::expression to_affine_expr(expr_ptr, const space_map &);

class isl_model_visitor : public visitor<void>
{
public:
    isl_model_visitor();
private:
    isl::context m_ctx;
    space_map m_space_map;
};

}
}
