#include "unit_test.hpp"

#include "../../polyhedral/model.hpp"
#include "../../polyhedral/printer.hpp"

#include <sstream>
#include <memory>
#include <algorithm>

namespace stream {
namespace unit_testing {

using namespace stream;
using namespace stream::polyhedral;
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

class mismatch_error : public failure
{
public:
    template <typename T>
    mismatch_error(const string & what, const T & found, const T & expected)
    {
        ostringstream text;
        text << "Wrong " << what << ": " << endl;
        text << "-- found:" << endl << found << endl;
        text << "-- expected:" << endl << expected << endl;

        reason = text.str();
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
    polyhedral::printer printer;
    statement * current_stmt = nullptr;

public:
    vector<statement*> given_stmts;
    vector<statement*> expected_stmts;

    template <typename T>
    printable<T> print( const T & v )
    {
        return printable<T>(v, printer);
    }

    void run()
    {
        compare_value("statement count", given_stmts.size(), expected_stmts.size());

        for (int i = 0; i < given_stmts.size(); ++i)
        {
            current_stmt = given_stmts[i];
            try {
                compare(given_stmts[i], expected_stmts[i]);
            }
            catch (failure & e)
            {
                ostringstream msg;

                msg << "* In statement " << current_stmt->name << endl;

                msg << e.reason << endl;

                msg << "Input:" << endl;
                for (statement *s : given_stmts)
                    printer.print(s, msg);

                e.reason = msg.str();

                throw e;
            }
        }
    }

    void compare(const statement *found, const statement * expected)
    {
        compare_value("statement domain", found->domain, expected->domain);
        try {
            compare(found->expr, expected->expr);
        }
        catch (failure & e)
        {
            ostringstream msg;
            msg << "* In expression:" << endl;
            printer.indent();
            printer.print(found->expr, msg);
            printer.unindent();
            msg << endl;
            msg << e.reason;
            e.reason = msg.str();
            throw e;
        }
    }

    void compare(const expression *found, const expression * expected)
    {
        if (found->type != expected->type)
        {
            throw mismatch_error( "numerical type",
                                  print(found->type),
                                  print(expected->type) );
        }

        try
        {
            compare_expr_type<constant<int>>(found, expected);
            compare_expr_type<constant<double>>(found, expected);
            compare_expr_type<intrinsic>(found, expected);
            compare_expr_type<iterator_access>(found, expected);
            compare_expr_type<stmt_access>(found, expected);
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

    void compare_expr(const stmt_access *found, const stmt_access * expected)
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

void test::test_polyhedral_model(const vector<polyhedral::statement*> & actual)
{
    if (!do_test_polyhedral_model)
        return;

    comparator c;
    c.given_stmts = actual;
    c.expected_stmts = m_expected_polyhedral_model;
    c.run();
}

} // namespace unit_testing
} // namespace stream
