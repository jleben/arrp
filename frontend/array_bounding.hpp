#pragma once

#include "array_to_isl.hpp"
#include "../utility/stacker.hpp"
#include "../common/functional_model.hpp"

#include <isl-cpp/set.hpp>
#include <isl-cpp/printer.hpp>

#include <utility>
#include <vector>

namespace stream {
namespace functional {

using std::pair;
using std::vector;
using ref_ptr = std::shared_ptr<reference>;

struct isl_domain_map
{
    isl_domain_map(): domain(nullptr) {}

    isl::set domain;
    vector<array_var_ptr> vars;
};

class isl_domain_constructor : private visitor<void>
{
public:
    isl_domain_map map(expr_ptr, const isl::context & = isl::context());

private:
    void visit_array(const shared_ptr<array> & arr) override;
    void visit_ref(const shared_ptr<reference> &) override;

    void add_variable(array_var_ptr);

    isl_domain_map m_map;
};

class array_var_refs : private visitor<void>
{
public:
    unordered_set<ref_ptr> find_in(expr_ptr);
private:
    unordered_set<ref_ptr> m_refs;
    void visit_ref(const shared_ptr<reference> &) override;
};

class array_bounding : private visitor<void>
{
public:
    array_bounding();
    void bound(array_ptr);

private:
    struct inferred_dim
    {
        int index;
        isl::set invalid_domain;
    };

    void visit_array(const shared_ptr<array> &) override;
    void visit_cases(const shared_ptr<case_expr> &) override;
    void visit_array_app(const shared_ptr<array_app> &) override;

    isl::set current_domain();
    stacker<isl::set, std::stack<isl::set>> push_domain(const isl::set & d);

    isl::context m_ctx;
    isl::printer m_printer;
    array_var_refs m_var_ref_finder;
    space_map m_space_map;
    vector<inferred_dim> m_inferred;
    std::stack<isl::set> m_domain;
};

}
}
