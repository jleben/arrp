#ifndef STREAM_LANG_POLYHEDRAL_MODEL_GENERATION
#define STREAM_LANG_POLYHEDRAL_MODEL_GENERATION

#include "../common/functional_model.hpp"
#include "../common/ph_model.hpp"
#include "../common/func_model_visitor.hpp"

#include <isl-cpp/set.hpp>
#include <isl-cpp/map.hpp>
#include <isl-cpp/context.hpp>
#include <isl-cpp/printer.hpp>

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace stream {
namespace functional {

using std::string;
using std::unordered_map;
using std::unordered_set;

class polyhedral_gen : public rewriter_base
{
public:
    polyhedral_gen();
    polyhedral::model process(const unordered_set<id_ptr> & ids);
    void add_output(polyhedral::model &,
                    const string & name, id_ptr id);

private:
    struct space_map
    {
        space_map(isl::space & s, isl::local_space & ls,
                  const vector<array_var_ptr> & v):
            space(s), local_space(ls), vars(v) {}
        isl::space & space;
        isl::local_space & local_space;
        const vector<array_var_ptr> & vars;

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
    };

    polyhedral::array_ptr make_array(id_ptr id);

    void make_statements(id_ptr id, polyhedral::model &);

    polyhedral::stmt_ptr make_stmt(const vector<array_var_ptr> &,
                                   const string & name,
                                   expr_ptr subdomain_expr,
                                   expr_ptr expr);

    isl::set to_affine_set(expr_ptr, const space_map &);
    isl::expression to_affine_expr(expr_ptr, const space_map &);

    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_array_app(const shared_ptr<array_app> & app) override;

    isl::context m_isl_ctx;
    isl::printer m_isl_printer;
    unordered_map<id_ptr, polyhedral::array_ptr> m_arrays;
    id_ptr m_current_id;
    polyhedral::stmt_ptr m_current_stmt;
    space_map * m_space_map = nullptr;
};

}
}


#endif // STREAM_LANG_POLYHEDRAL_MODEL_GENERATION
