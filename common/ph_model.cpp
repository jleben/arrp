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
    auto ctx = space.get_context();

    if (relation.size.empty())
    {
        auto eq = to_isl_matrix(relation.matrix, ctx, upper_bound);
        auto ineq = isl::matrix(ctx, eq.row_count(), eq.column_count(), 0);
        return isl::basic_map(space, eq, ineq);
    }
    else
    {
        assert(relation.size.size() == relation.matrix.output_dimension());
        auto lb = to_isl_matrix(relation.matrix, ctx, lower_bound);
        auto ub_matrix = relation.matrix;
        for (int dim = 0; dim < ub_matrix.output_dimension(); ++dim)
        {
            assert(relation.size[dim] > 0);
            ub_matrix.constant(dim) += relation.size[dim] - 1;
        }
        auto ub = to_isl_matrix(ub_matrix, ctx, upper_bound);
        auto ineq = isl::matrix::concatenate_vertically(lb, ub);
        auto eq = isl::matrix(ctx, ineq.row_count(), ineq.column_count(), 0);
        return isl::basic_map(space, eq, ineq);
    }
}

}
}
