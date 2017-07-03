#pragma once

#include "../common/functional_model.hpp"
#include "../common/func_model_visitor.hpp"
#include "../frontend/error.hpp"
#include "type_graph.hpp"

#include <stack>
#include <unordered_set>
#include <unordered_map>

namespace arrp {

using namespace stream;
using namespace stream::functional;

using std::stack;
using std::unordered_set;
using std::unordered_map;

class type_checker;

class type_constraint_setup : public visitor<type>
{
public:
    type_constraint_setup(type_graph & graph);

    void process(const unordered_set<id_ptr> & ids);

private:
    void process(const id_ptr &);
    type type_for(const id_ptr &);
    type visit(const expr_ptr & e) override;
    type visit_ref(const shared_ptr<reference> & e) override;
    type visit_int(const shared_ptr<int_const> &) override;
    type visit_real(const shared_ptr<real_const> &) override;
    type visit_complex(const shared_ptr<complex_const> &) override;
    type visit_bool(const shared_ptr<bool_const> &) override;
    type visit_infinity(const shared_ptr<infinity> &) override;
    type visit_array(const shared_ptr<array> & arr) override;
    type visit_array_app(const shared_ptr<array_app> & app) override;
    type visit_array_size(const shared_ptr<array_size> & as) override;
    type visit_func(const shared_ptr<function> & func) override;
    type visit_func_app(const shared_ptr<func_app> & app) override;


    type_graph & m_graph;
};

}
