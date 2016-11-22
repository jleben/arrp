#include "utility.hpp"
#include "../common/error.hpp"
#include <isl-cpp/printer.hpp>

using namespace std;

namespace arrp {

void find_rays(const isl::basic_set & set, vector<ivector> & rays)
{
    isl::printer p(set.ctx());

    auto space = set.local_space();

    if (space.dimension(isl::space::parameter) != 0)
    {
        throw stream::error("Parameteric sets not supported.");
    }
    if (space.dimension(isl::space::div) != 0)
    {
        throw stream::error("Sets with local variables not allowed.");
    }

    isl::basic_set dual = isl_basic_set_coefficients(set.copy());
    //cout << "dual: "; p.print(dual); cout << endl;

    int n_dual_vars = dual.get_space().dimension(isl::space::variable);

    isl::matrix dual_eq =
            isl_basic_set_equalities_matrix
            (dual.get(),
             isl_dim_set,
             isl_dim_cst,
             isl_dim_param,
             isl_dim_div
             );

    isl::matrix dual_ineq =
            isl_basic_set_inequalities_matrix
            (dual.get(),
             isl_dim_set,
             isl_dim_cst,
             isl_dim_param,
             isl_dim_div
             );

    for (int c = 0; c < dual_ineq.row_count(); ++c)
    {
        auto d = dual_ineq(c,0).value().integer();
        if (d != 0)
            continue;

        ivector vec(n_dual_vars-1);
        for (int i = 0; i < n_dual_vars-1; ++i)
        {
            auto val = dual_ineq(c,i+1).value();
            if (!val.is_integer())
                throw stream::error("Ray coordinate not an integer.");
            vec[i] = dual_ineq(c,i+1).value().integer();
        }

        rays.push_back(vec);
    }

    for (int c = 0; c < dual_eq.row_count(); ++c)
    {
        auto d = dual_eq(c,0).value().integer();
        if (d != 0)
            continue;

        ivector pvec(n_dual_vars-1);
        ivector nvec(n_dual_vars-1);
        for (int i = 0; i < n_dual_vars-1; ++i)
        {
            auto val = dual_eq(c,i+1).value();
            if (!val.is_integer())
                throw stream::error("Ray coordinate not an integer.");
            pvec[i] = dual_eq(c,i+1).value().integer();
            nvec[i] = -pvec[i];
        }

        rays.push_back(pvec);
        rays.push_back(nvec);
    }
}

}
