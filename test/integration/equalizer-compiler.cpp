#include "equalizer.hpp"
#include "model-construction.hpp"
#include "../../compiler/compiler.hpp"

#include <memory>

using namespace stream;
using namespace stream::polyhedral;
using namespace stream::testing;
using namespace std;

static
statement* biquad(statement *in, const vector<double> & coeff,
                  const string & id, vector<statement*> & stmts)
{
    const double a1 = coeff[0];
    const double a2 = coeff[1];
    const double b0 = coeff[2];
    const double b1 = coeff[3];
    const double b2 = coeff[4];

    stmt_access * x = new stmt_access(in);
    x->pattern = mapping::identity(1,1);

    statement *W = new statement(string("W") + id);
    {
        W->domain = {-1};

        stmt_access *w1 = new stmt_access(W, primitive_type::real);
        w1->pattern = mapping::identity(1,1);
        w1->pattern.constant(0) = -1;

        stmt_access *w2 = new stmt_access(W, primitive_type::real);
        w2->pattern = mapping::identity(1,1);
        w2->pattern.constant(0) = -2;

        W->expr = op(x) - (op(w1) * const_op(a1)) - (op(w2) * const_op(a2));
    }

    statement *Y = new statement(string("Y") + id);
    {
        Y->domain = {-1};

        stmt_access *w0 = new stmt_access(W);
        w0->pattern = mapping::identity(1,1);

        stmt_access *w1 = new stmt_access(W);
        w1->pattern = mapping::identity(1,1);
        w1->pattern.constant(0) = -1;

        stmt_access *w2 = new stmt_access(W);
        w2->pattern = mapping::identity(1,1);
        w2->pattern.constant(0) = -2;

        Y->expr = op(w0) * const_op(b0) + op(w1) * const_op(b1) + op(w2) * const_op(b2);
    }

    stmts.push_back(W);
    stmts.push_back(Y);

    return Y;
}

static
statement* biquad_n(statement *in, int N, const vector<double> & coeff,
                    const string & id, vector<statement*> & stmts)
{
    const double a1 = coeff[0];
    const double a2 = coeff[1];
    const double b0 = coeff[2];
    const double b1 = coeff[3];
    const double b2 = coeff[4];

    stmt_access * x = new stmt_access(in);
    x->pattern = mapping::identity(2,2);

    statement *W = new statement(string("W") + id);
    {
        W->domain = {-1,N};

        stmt_access *w1 = new stmt_access(W, primitive_type::real);
        w1->pattern = mapping::identity(2,2);
        w1->pattern.constant(0) = -1;

        stmt_access *w2 = new stmt_access(W, primitive_type::real);
        w2->pattern = mapping::identity(2,2);
        w2->pattern.constant(0) = -2;

        W->expr = op(x) - (op(w1) * const_op(a1)) - (op(w2) * const_op(a2));
    }

    statement *Y = new statement(string("Y") + id);
    {
        Y->domain = {-1,N};

        stmt_access *w0 = new stmt_access(W);
        w0->pattern = mapping::identity(2,2);

        stmt_access *w1 = new stmt_access(W);
        w1->pattern = mapping::identity(2,2);
        w1->pattern.constant(0) = -1;

        stmt_access *w2 = new stmt_access(W);
        w2->pattern = mapping::identity(2,2);
        w2->pattern.constant(0) = -2;

        Y->expr = op(w0) * const_op(b0) + op(w1) * const_op(b1) + op(w2) * const_op(b2);
    }

    stmts.push_back(W);
    stmts.push_back(Y);

    return Y;
}

void create_model(vector<polyhedral::statement*> & model,
                  vector<semantic::type_ptr> & inputs)
{
    // Create polyhedral model

    statement *in = new statement("In");
    in->domain = {-1};
    in->expr = new input_access(primitive_type::real, 0);
    model.push_back(in);

    statement *y;
    y = biquad(in, equalizer::low_pass_coef, "1", model);
    y = biquad(y, equalizer::high_pass_coef, "2", model);
    y = biquad(y, equalizer::band_stop_coef, "3", model);

    // Define inputs

    inputs.push_back( std::make_shared<semantic::stream>(semantic::stream::infinite,
                                                         primitive_type::real) );
}

int main()
{
    vector<polyhedral::statement*> model;
    vector<semantic::type_ptr> inputs;

    create_model(model, inputs);

    compiler::arguments args(0, nullptr);
    args.output_filename = "equalizer-kernel.ll";
    args.cpp_output_filename = "equalizer-kernel.h";
    args.target.name = "equalizer";
    args.target.args = inputs;
    args.print[compiler::arguments::polyhedral_model_output] = true;
    args.print[compiler::arguments::target_ast_output] = true;
    args.print[compiler::arguments::buffer_size_output] = true;
    args.debug_topics.push_back("polyhedral.ast");
    args.debug_topics.push_back("dataflow");

    for(const auto & topic : args.debug_topics)
        debug::set_status_for_id(topic, debug::enabled);
    for(const auto & topic : args.no_debug_topics)
        debug::set_status_for_id(topic, debug::disabled);

    compiler::compile_polyhedral_model(model, args);
}
