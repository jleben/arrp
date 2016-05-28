#include "ph_model.hpp"

namespace stream {
namespace polyhedral {

enum bound {
    upper_bound,
    lower_bound
};

isl::matrix to_isl_matrix(const affine_matrix & src, const isl::context & ctx, bound b)
{
    int sign = b == upper_bound ? 1 : -1;

    isl::matrix dst(ctx, src.output_dimension(),
                    src.input_dimension() + src.output_dimension() + 1,
                    0);
    for(int out = 0; out < src.output_dimension(); ++out)
    {
        int out_col = src.input_dimension() + out;
        dst(out, out_col) = -sign;

        for (int in = 0; in < src.input_dimension(); ++in)
        {
            int in_col = in;
            dst(out, in_col) = src.coefficient(in, out) * sign;
        }

        int const_col = src.input_dimension() + src.output_dimension();
        dst(out, const_col) = src.constant(out) * sign;
    }
    return dst;
}

isl::map to_isl_map(const stmt_ptr & stmt, const array_relation & relation)
{
    auto space = isl::space::from(stmt->domain.get_space(),
                                  relation.array->domain.get_space());
#if 0
    auto to_expr_on_map = [](const isl::expression & e, const isl::space & s)
    {
        auto ls = isl::local_space(s).wrapped();
        int n_in = s.dimension(isl::space::input);
        //int n_out = s.dimension(isl::space::output);

        // FIXME: other dimension types?
        auto m = isl_aff_zero_on_domain(ls.copy());
        for (int i = 0; i < n_in; ++i)
        {
            auto v = e.coefficient(isl::space::variable, i);
            m = isl_aff_set_coefficient_val
                    (m, isl_dim_set, i, v.copy());
        }
        m = isl_aff_set_constant_val(m, e.constant().copy());
        return isl::expression(m);
    };
#endif
    if (relation.size.empty())
    {
        auto map = isl::basic_map::universe(space);
        int out_idx = 0;
        for (int d = 0; d < relation.expr.size(); ++d)
        {
            auto e = relation.expr.at(d);
            auto o = space.out(out_idx);
            map.add_constraint(o == e);
            ++out_idx;
        }
        return map;
    }
    else
    {
        auto map = isl::basic_map::universe(space);
        int out_idx = 0;
        for (int d = 0; d < relation.expr.size(); ++d)
        {
            auto e = relation.expr.at(d);
            auto o = space(isl::space::output, out_idx);

            // lower bound
            map.add_constraint(o >= e);

            // upper bound
            assert(relation.size[out_idx] > 0);
            e = e + relation.size[out_idx];
            map.add_constraint(o < e);

            ++out_idx;
        }
        return map;
    }
}

}
}
