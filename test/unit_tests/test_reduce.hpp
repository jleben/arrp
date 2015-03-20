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

#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {

namespace reduce {

result stream1_add()
{
    test t;

    std::istringstream code("f(x) = reduce(a,b in x) a + b");

    t.expect_type(real_type());

    {
        using namespace polyhedral;

        statement *x = new statement;
        x->domain = {9};
        x->expr = new input_access(polyhedral::real, 0);

        // initializer
        stmt_access *x0 = new stmt_access(x);
        x0->pattern = mapping(1,1);
        statement *initializer = new statement;
        initializer->domain = {1};
        initializer->expr = x0;

        // reductor
        statement *reductor = new statement;
        reductor->domain = {8};

        reduction_access *a = new reduction_access(polyhedral::real);
        a->initializer = initializer;
        a->reductor = reductor;

        stmt_access *b = new stmt_access(x);
        b->pattern = mapping::identity(1,1);
        b->pattern.constant(0) = 1;

        reductor->expr = new primitive_expr(polyhedral::real, primitive_op::add,{a,b});

        // result
        stmt_access *out_expr = new stmt_access(reductor);
        out_expr->pattern = mapping(1,1);
        out_expr->pattern.constant(0) = 7;
        statement *out = new statement;
        out->domain = {1};
        out->expr = out_expr;

        t.expect_polyhedral_model({x,initializer,reductor,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result map_range_by_stream1_reduction()
{
    test t;

    std::istringstream code("f(x) = for each(in 1..100) reduce(a,b in x) a + b");

    t.expect_type(stream_type(100));

    {
        using namespace polyhedral;

        statement *x = new statement;
        x->domain = {9};
        x->expr = new input_access(polyhedral::real, 0);

        // initializer
        stmt_access *x0 = new stmt_access(x);
        x0->pattern = mapping(1,1);
        statement *initializer = new statement;
        initializer->domain = {100};
        initializer->expr = x0;

        // reductor
        statement *reductor = new statement;
        reductor->domain = {100,8};

        reduction_access *a = new reduction_access(polyhedral::real);
        a->initializer = initializer;
        a->reductor = reductor;

        stmt_access *b = new stmt_access(x);
        b->pattern = mapping(2,1);
        b->pattern.coefficient(1,0) = 1;
        b->pattern.constant(0) = 1;

        reductor->expr = new primitive_expr(polyhedral::real, primitive_op::add,{a,b});

        // result
        stmt_access *out_expr = new stmt_access(reductor);
        out_expr->pattern = mapping(1,2);
        out_expr->pattern.coefficient(0,0) = 1;
        out_expr->pattern.constant(1) = 7;
        statement *out = new statement;
        out->domain = {100};
        out->expr = out_expr;

        t.expect_polyhedral_model({x,initializer,reductor,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result map_stream2_by_substream_reduction()
{
    test t;

    std::istringstream code("f(x) = for each(y in x) reduce(a,b in y) a + b");

    t.expect_type(stream_type(9));

    {
        using namespace polyhedral;

        statement *x = new statement;
        x->domain = {9,13};
        x->expr = new input_access(polyhedral::real, 0);

        // initializer
        stmt_access *x0 = new stmt_access(x);
        x0->pattern = mapping::identity(1,2);

        statement *initializer = new statement;
        initializer->domain = {9};
        initializer->expr = x0;

        // reductor
        statement *reductor = new statement;
        reductor->domain = {9,12};

        reduction_access *a = new reduction_access(polyhedral::real);
        a->initializer = initializer;
        a->reductor = reductor;

        stmt_access *b = new stmt_access(x);
        b->pattern = mapping::identity(2,2);
        b->pattern.constant(1) = 1;

        reductor->expr = new primitive_expr(polyhedral::real, primitive_op::add,{a,b});

        // result
        stmt_access *out_expr = new stmt_access(reductor);
        out_expr->pattern = mapping(1,2);
        out_expr->pattern.coefficient(0,0) = 1;
        out_expr->pattern.constant(1) = 11;
        statement *out = new statement;
        out->domain = {9};
        out->expr = out_expr;

        t.expect_polyhedral_model({x,initializer,reductor,out});
    }

    return run(t, code, "f", { stream_type(9,13) });
}

}

}
}
