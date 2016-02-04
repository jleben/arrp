#include "ph_model.hpp"

namespace stream {
namespace polyhedral {

isl::matrix to_isl_equalities_matrix(const affine_matrix & src, const isl::context & ctx)
{
    isl::matrix dst(ctx, src.output_dimension(),
                    src.input_dimension() + src.output_dimension() + 1,
                    0);
    for(int out = 0; out < src.output_dimension(); ++out)
    {
        int out_col = src.input_dimension() + out;
        dst(out, out_col) = -1;

        for (int in = 0; in < src.input_dimension(); ++in)
        {
            int in_col = in;
            dst(out, in_col) = src.coefficient(in, out);
        }

        int const_col = src.input_dimension() + src.output_dimension();
        dst(out, const_col) = src.constant(out);
    }
    return dst;
}

isl::map to_isl_map(const affine_matrix & affine, const isl::space & space)
{
    auto ctx = space.get_context();
    auto eq = to_isl_equalities_matrix(affine, ctx);
    auto ineq = isl::matrix(ctx, eq.row_count(), eq.column_count(), 0);
    auto map = isl::basic_map(space, eq, ineq);
    return map;
}

}
}
