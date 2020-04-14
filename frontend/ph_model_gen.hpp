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
    struct options
    {
        bool ordered_io = true;
        bool atomic_io = false;
    };

    polyhedral_gen(const options &);

    polyhedral::model process(const vector<id_ptr> & ids)
    {
        return process(unordered_set<id_ptr>(ids.begin(), ids.end()));
    }

    polyhedral::model process(const unordered_set<id_ptr> & ids);

private:
    struct space_map
    {
        // Requires a set space
        space_map(isl::space & s,
                  const vector<array_var_ptr> & v):
            space(s), local_space(s), vars(v) {}
        isl::space & space;
        isl::local_space local_space;
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

    void make_time_array();
    void add_time_array(polyhedral::model &);

    void make_input(polyhedral::model &, id_ptr id, bool atomic, bool ordered);
    void make_output(polyhedral::model &, id_ptr id, bool atomic, bool ordered);

    polyhedral::array_ptr make_array(id_ptr id);

    void make_statements(id_ptr id, polyhedral::model &);

    polyhedral::stmt_ptr make_stmt(const vector<array_var_ptr> &,
                                   const string & name,
                                   const expr_slot & subdomain_expr,
                                   const expr_slot & expr);

    isl::set to_affine_set(expr_ptr, const space_map &);
    isl::expression to_affine_expr(expr_ptr, const space_map &);

    shared_ptr<polyhedral::array_access> access
    (const polyhedral::stmt_ptr &, const polyhedral::array_ptr &, const vector<expr_ptr> index,
     bool read, bool write);

    expr_ptr visit(const expr_ptr &expr) override;
    expr_ptr visit_ref(const shared_ptr<reference> & e) override;
    expr_ptr visit_array_app(const shared_ptr<array_app> & app) override;
    expr_ptr visit_func_app(const shared_ptr<func_app> &app) override;

    const options m_options;

    isl::context m_isl_ctx;
    isl::printer m_isl_printer;
    unordered_map<id_ptr, polyhedral::array_ptr> m_arrays;
    id_ptr m_current_id;
    polyhedral::stmt_ptr m_current_stmt;
    space_map * m_space_map = nullptr;

    unordered_set<expr_ptr> m_nonaffine_array_args;
    bool m_in_affine_array_arg = false;

    polyhedral::array_ptr m_time_array;
    vector<polyhedral::stmt_ptr> m_time_stmts;
    bool m_time_array_needed = false;
};


struct piecewise_affine_expr_builder
{
    piecewise_affine_expr_builder(isl::space domain, const vector<array_var_ptr> & vars):
        m_space(domain), m_vars(vars)
    {}

    isl::piecewise_expression build(expr_ptr);

    isl::space space() const { return m_space; }
    const vector<array_var_ptr> & vars() { return m_vars; }

    const unordered_set<expr_ptr> & nnonaffine() { return m_nonaffine; }

    int index_of(array_var_ptr var) const
    {
        int i;
        for(i=0; i<m_vars.size(); ++i)
        {
            if (m_vars[i] == var)
                return i;
        }
        return -1;
    }

private:
    isl::space m_space;
    const vector<array_var_ptr> & m_vars;

    unordered_set<expr_ptr> m_nonaffine;
};

void add_io_clock(polyhedral::model &);

}
}


#endif // STREAM_LANG_POLYHEDRAL_MODEL_GENERATION
