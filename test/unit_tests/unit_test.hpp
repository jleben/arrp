#ifndef STREAM_LANG_UNIT_TEST_INCLUDED
#define STREAM_LANG_UNIT_TEST_INCLUDED

#include "../../frontend/types.hpp"
#include "../../frontend/ast.hpp"
#include "../../frontend/environment.hpp"
#include "../../polyhedral/model.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

namespace stream {
namespace unit_testing {

using std::vector;
using std::istream;
using std::string;

class success {};

class failure
{
public:
    failure() {}
    failure( const string & reason ): reason(reason) {}
    string reason;
};

enum result
{
    succeeded = 0,
    failed
};

using semantic::type_ptr;

inline
type_ptr int_type() { return std::make_shared<semantic::integer_num>(); }
inline
type_ptr int_type(int c) { return std::make_shared<semantic::integer_num>(c); }
inline
type_ptr real_type() { return std::make_shared<semantic::real_num>(); }
inline
type_ptr real_type(double c) { return std::make_shared<semantic::real_num>(c); }
inline
type_ptr range_type() { return std::make_shared<semantic::range>(); }

inline
type_ptr stream_type( int size)
{ return std::make_shared<semantic::stream>(size); }

template <typename ...T> inline
type_ptr stream_type(const T & ...size)
{ return std::make_shared<semantic::stream>( vector<int>({size...}) ); }


class test
{
public:
    typedef vector<semantic::type_ptr> arg_list;

    test(): do_test_type(false), do_test_polyhedral_model(false) {}

    void run(istream & code, const string & symbol, const arg_list & args = arg_list());

    void expect_type( const semantic::type_ptr & t )
    {
        do_test_type = true;
        m_expected_type = t;
    }

    void expect_polyhedral_model( const vector<polyhedral::statement*> & m )
    {
        do_test_polyhedral_model = true;
        m_expected_polyhedral_model = m;
    }

private:
    bool do_test_type;
    bool do_test_polyhedral_model;

    semantic::type_ptr m_expected_type;
    vector<polyhedral::statement*> m_expected_polyhedral_model;

    void test_type(const semantic::type_ptr & actual);
    void test_polyhedral_model(const vector<polyhedral::statement*> & actual);
};

result run( test & t, istream & code,
            const string & symbol, const test::arg_list & args = test::arg_list() );

}
}

#endif // STREAM_LANG_UNIT_TEST_INCLUDED
