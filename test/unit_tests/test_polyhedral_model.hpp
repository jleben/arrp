#include "unit_test.hpp"
#include "../../polyhedral/model.hpp"
#include "../../polyhedral/printer.hpp"

#include <sstream>
#include <fstream>
#include <memory>
#include <algorithm>

using namespace stream;
using namespace stream::polyhedral;
using stream::unit_test::result;
using namespace std;

// Printing

template <typename T>
class printable
{
public:
    printable(const T & v, polyhedral::printer & p):
        value(v), printer(p)
    {}

    const T & value;
    polyhedral::printer & printer;
};

template <typename T>
ostream & operator<<( ostream &stream, const printable<T> & object )
{
    object.printer.print(object.value, stream);
    return stream;
}

ostream & operator<<( ostream &stream, const vector<int> & v )
{
    stream << "[";
    for (int i = 0; i < v.size(); ++i)
    {
        if (i > 0)
            stream << ",";
        stream << v[i];
    }
    stream << "]";
    return stream;
}

// Exceptions

class test_error
{
public:
    test_error() {}
    test_error( const string & msg ): msg(msg) {}
    string msg;
};

class success {};

class mismatch_error : public test_error
{
public:
    template <typename T>
    mismatch_error(const string & what, const T & found, const T & expected)
    {
        ostringstream text;
        text << "Wrong " << what << ": " << endl;
        text << "-- found:" << endl << found << endl;
        text << "-- expected:" << endl << expected << endl;

        msg = text.str();
    }
};

template<typename T>
void compare_value( const string & what, const T & found, const T & expected )
{
    if (!(found == expected))
    {
        throw mismatch_error(what, found, expected);
    }
}

class comparator
{
public:
    vector<statement*> given_stmts;
    vector<statement*> expected_stmts;
    statement * current_stmt = nullptr;
    polyhedral::printer printer;

    template <typename T>
    printable<T> print( const T & v )
    {
        return printable<T>(v, printer);
    }

    result compare()
    {
        try {
            compare_value("statement count", given_stmts.size(), expected_stmts.size());

            for (int i = 0; i < given_stmts.size(); ++i)
            {
                current_stmt = given_stmts[i];
                try {
                    compare(given_stmts[i], expected_stmts[i]);
                }
                catch (test_error & e)
                {
                    ostringstream msg;
                    msg << "* In statement " << current_stmt->name << endl;
                    msg << e.msg;
                    e.msg = msg.str();
                    throw e;
                }
            }
        }
        catch (test_error &e)
        {
            cerr << "FAILURE:" << endl;
            cerr << e.msg << endl;
            cerr << "Input:" << endl;
            for (statement *s : given_stmts)
                printer.print(s, cerr);

            return unit_test::failure;
        }

        return unit_test::success;
    }

    void compare(const statement *found, const statement * expected)
    {
        compare_value("statement domain", found->domain, expected->domain);
        try {
            compare(found->expr, expected->expr);
        }
        catch (test_error & e)
        {
            ostringstream msg;
            msg << "* In expression:" << endl;
            printer.indent();
            printer.print(found->expr, msg);
            printer.unindent();
            msg << endl;
            msg << e.msg;
            e.msg = msg.str();
            throw e;
        }
    }

    void compare(const expression *found, const expression * expected)
    {
        try
        {
            compare_expr_type<constant<int>>(found, expected);
            compare_expr_type<constant<double>>(found, expected);
            compare_expr_type<intrinsic>(found, expected);
            compare_expr_type<iterator_access>(found, expected);
            compare_expr_type<stream_access>(found, expected);
            compare_expr_type<reduction_access>(found, expected);
            compare_expr_type<input_access>(found, expected);
        }
        catch (success &)
        {

        }
    }

    template <typename T>
    void compare_expr_type(const expression *found, const expression * expected)
    {
        if (const T* expected_type = dynamic_cast<const T*>(expected))
        {
            const T* found_type = dynamic_cast<const T*>(found);
            if (!found_type)
                throw mismatch_error( "expression type",
                                      print(found),
                                      print(expected) );

            compare_expr(found_type, expected_type);

            throw success();
        }
    }

    template <typename T>
    void compare_expr(const constant<T> *found, const constant<T> *expected)
    {
        compare_value("constant int value",
                      found->value, expected->value);
    }

    void compare_expr(const intrinsic *found, const intrinsic * expected)
    {
        compare_value("intrinsic type",
                      found->kind, expected->kind);

        compare_value("intrinsic operand count",
                      found->operands.size(), expected->operands.size());

        for (int i = 0; i < found->operands.size(); ++i)
        {
            compare(found->operands[i], expected->operands[i]);
        }
    }

    void compare_expr(const iterator_access *found, const iterator_access * expected)
    {
        compare_value("iterator dimension", found->dimension, expected->dimension);
        compare_value("iterator ratio", found->ratio, expected->ratio);
        compare_value("iterator offset", found->offset, expected->offset);
    }

    void compare_expr(const stream_access *found, const stream_access * expected)
    {
        compare_stmt_reference("stream access target",
                               found->target, expected->target);

        ostringstream msg;
        msg << "stream access pattern for " << found->target->name;
        compare_value(msg.str(), found->pattern, expected->pattern);
    }

    void compare_expr(const reduction_access *found, const reduction_access * expected)
    {
        compare_stmt_reference("reduction initializer",
                               found->initializer, expected->initializer);

        compare_stmt_reference("reduction reductor",
                               found->reductor, expected->reductor);
    }

    void compare_expr(const input_access *found, const input_access * expected)
    {
        compare_value("input index", found->index, expected->index);
    }

    void compare_stmt_reference
    (const string & what,
     const statement *given, const statement *expected)
    {
        auto expected_loc = std::find(expected_stmts.begin(),
                                      expected_stmts.end(),
                                      expected);
        int expected_idx = std::distance(expected_stmts.begin(), expected_loc);

        auto given_loc = std::find(given_stmts.begin(),
                                   given_stmts.end(),
                                   given);
        int given_idx = std::distance(given_stmts.begin(), given_loc);

        if (given_idx != expected_idx)
        {
            statement *expected_given = given_stmts[expected_idx];
            throw mismatch_error(what, given->name, expected_given->name);
        }
    }

private:

};

semantic::type_ptr make_stream_type( const vector<int> & size )
{
    return make_shared<semantic::stream>(size);
}

namespace poly {
namespace slice {

result dim_0_of_3()
{
    istringstream code("f(x) = x[5]");

    comparator c;

    c.given_stmts = unit_test::polyhedral_model(code, "f", { make_stream_type({10,15,20}) });

    {
        statement *in = new statement;
        in->domain = {10,15,20};
        in->expr = new input_access(0);

        stream_access *slicer = new stream_access;
        slicer->target = in;
        slicer->pattern = mapping(2,3);
        slicer->pattern.constant(0) = 4;
        slicer->pattern.coefficient(0,1) = 1;
        slicer->pattern.coefficient(1,2) = 1;

        statement *out = new statement;
        out->expr = slicer;
        out->domain = {15,20};

        c.expected_stmts = {in,out};
    }

    return c.compare();
}

result dummy ()
{
    ifstream file("slice1.in");
    if (!file.is_open())
        return unit_test::failure_msg("Could not open input file.");

    comparator c;

    {
        statement *in = new statement;
        in->domain = {infinite};
        in->expr = new input_access(0);

        stream_access *in_access = new stream_access;
        in_access->target = in;
        in_access->pattern = mapping(1,1);
        in_access->pattern.coefficient(0,0) = 5;
        in_access->pattern.constant(0) = 1;

        statement *compute = new statement;
        compute->expr = new intrinsic(intrinsic::add, {new constant<int>(3), in_access});
        compute->domain = {infinite};

        c.expected_stmts = {in, compute};
    }


    c.given_stmts = unit_test::polyhedral_model(file, "f", { make_stream_type({infinite}) });


    return c.compare();
}

} // namespace slice
} // namespace polyhedral
