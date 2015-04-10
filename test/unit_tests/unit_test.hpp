/*
Compiler for language for stream processing

Copyright (C) 2014  Jakob Leben <jakob.leben@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef STREAM_LANG_UNIT_TEST_INCLUDED
#define STREAM_LANG_UNIT_TEST_INCLUDED

#include "../../common/types.hpp"
#include "../../common/ast.hpp"
#include "../../common/environment.hpp"
#include "../../common/polyhedral_model.hpp"

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
type_ptr stream_type(int size)
{ return std::make_shared<semantic::stream>(size, primitive_type::real); }

template <typename ...T> inline
type_ptr stream_type(const T & ...size)
{ return std::make_shared<semantic::stream>( vector<int>({size...}), primitive_type::real ); }

inline
type_ptr stream_type(primitive_type t, int size)
{ return std::make_shared<semantic::stream>(size, t); }

template <typename ...T> inline
type_ptr stream_type(primitive_type t, const T & ...size)
{ return std::make_shared<semantic::stream>( vector<int>({size...}), t); }


class test
{
public:
    typedef vector<semantic::type_ptr> arg_list;

    test(): do_test_type(false), do_test_polyhedral_model(false) {}

    result try_run(istream & code);

    void run(istream & code);

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

    void set_target( const string & symbol, const test::arg_list & args = test::arg_list() )
    {
        m_symbol = symbol;
        m_args = args;
    }

private:
    string m_symbol;
    arg_list m_args;

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
