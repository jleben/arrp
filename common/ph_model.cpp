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

}
}
