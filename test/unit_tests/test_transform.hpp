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

namespace slice {

result stream1_by_int ()
{
    test t;

    std::istringstream code("f(x) = x[3]");

    t.expect_type(real_type());

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(1,1);
        x->pattern.constant(0) = 2;

        statement *out = new statement;
        out->domain = {1};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream1_by_range ()
{
    test t;

    std::istringstream code("f(x) = x[3..6]");

    t.expect_type(stream_type(4));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(1,1);
        x->pattern.coefficient(0,0) = 1;
        x->pattern.constant(0) = 2;

        statement *out = new statement;
        out->domain = {4};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9) });
}

result stream3_by_int ()
{
    test t;

    std::istringstream code("f(x) = x[3]");

    t.expect_type(stream_type(11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(2,3);
        x->pattern.constant(0) = 2;
        x->pattern.coefficient(0,1) = 1;
        x->pattern.coefficient(1,2) = 1;

        statement *out = new statement;
        out->domain = {11,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_range ()
{
    test t;

    std::istringstream code("f(x) = x[3..6]");

    t.expect_type(stream_type(4,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);
        x->pattern.constant(0) = 2;

        statement *out = new statement;
        out->domain = {4,11,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_int2 ()
{
    test t;

    std::istringstream code("f(x) = x[3,5]");

    t.expect_type(stream_type(13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(1,3);
        x->pattern.constant(0) = 2;
        x->pattern.constant(1) = 4;
        x->pattern.coefficient(0,2) = 1;

        statement *out = new statement;
        out->domain = {13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_range2 ()
{
    test t;

    std::istringstream code("f(x) = x[3..6, 4..8]");

    t.expect_type(stream_type(4,5,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);
        x->pattern.constant(0) = 2;
        x->pattern.constant(1) = 3;

        statement *out = new statement;
        out->domain = {4,5,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_int3 ()
{
    test t;

    std::istringstream code("f(x) = x[3,5,7]");

    t.expect_type(real_type());

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(1,3);
        x->pattern.constant(0) = 2;
        x->pattern.constant(1) = 4;
        x->pattern.constant(2) = 6;

        statement *out = new statement;
        out->domain = {1};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_range3 ()
{
    test t;

    std::istringstream code("f(x) = x[3..6, 4..8, 7..9]");

    t.expect_type(stream_type(4,5,3));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping::identity(3,3);
        x->pattern.constant(0) = 2;
        x->pattern.constant(1) = 3;
        x->pattern.constant(2) = 6;

        statement *out = new statement;
        out->domain = {4,5,3};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream3_by_range_int_range ()
{
    test t;

    std::istringstream code("f(x) = x[3..6, 5, 7..9]");

    t.expect_type(stream_type(4,3));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(2,3);

        x->pattern.coefficient(0,0) = 1;
        x->pattern.constant(0) = 2;

        x->pattern.constant(1) = 4;

        x->pattern.coefficient(1,2) = 1;
        x->pattern.constant(2) = 6;

        statement *out = new statement;
        out->domain = {4,3};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

}

namespace transpose {

result stream3_to_dim2()
{
    test t;

    std::istringstream code("f(x) = x{2}");

    t.expect_type(stream_type(11,9,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(3,3);
        x->pattern.coefficient(0,1) = 1;
        x->pattern.coefficient(1,0) = 1;
        x->pattern.coefficient(2,2) = 1;

        statement *out = new statement;
        out->domain = {11,9,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13) });
}

result stream4_to_dim2_dim3()
{
    test t;

    std::istringstream code("f(x) = x{2,3}");

    t.expect_type(stream_type(11,13,9,15));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13,15};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(4,4);
        x->pattern.coefficient(0,1) = 1;
        x->pattern.coefficient(1,2) = 1;
        x->pattern.coefficient(2,0) = 1;
        x->pattern.coefficient(3,3) = 1;

        statement *out = new statement;
        out->domain = {11,13,9,15};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13,15) });
}

result stream4_to_dim4_dim2()
{
    test t;

    std::istringstream code("f(x) = x{4,2}");

    t.expect_type(stream_type(15,11,9,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13,15};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(4,4);
        x->pattern.coefficient(0,3) = 1;
        x->pattern.coefficient(1,1) = 1;
        x->pattern.coefficient(2,0) = 1;
        x->pattern.coefficient(3,2) = 1;

        statement *out = new statement;
        out->domain = {15,11,9,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13,15) });
}

}

namespace transform
{

result slice_by_scalar_and_transpose()
{
    test t;

    std::istringstream code("f(x) = x[5]{3}");

    t.expect_type(stream_type(15,11,13));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13,15};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(3,4);
        x->pattern.constant(0) = 4;
        x->pattern.coefficient(0,3) = 1;
        x->pattern.coefficient(1,1) = 1;
        x->pattern.coefficient(2,2) = 1;

        statement *out = new statement;
        out->domain = {15,11,13};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13,15) });
}

result transpose_and_slice_by_scalar()
{
    test t;

    std::istringstream code("f(x) = x{3}[5]");

    t.expect_type(stream_type(9,11,15));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {9,11,13,15};
        in->expr = new input_access(primitive_type::real, 0);

        stmt_access *x = new stmt_access(in);
        x->pattern = mapping(3,4);
        x->pattern.coefficient(0,0) = 1;
        x->pattern.coefficient(1,1) = 1;
        x->pattern.constant(2) = 4;
        x->pattern.coefficient(2,3) = 1;

        statement *out = new statement;
        out->domain = {9,11,15};
        out->expr = x;

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "f", { stream_type(9,11,13,15) });
}

result stream2_map_and_slice()
{
    std::istringstream code("f(x) = for each (y in x) y[3]");

    test t;
    t.set_target("f", { stream_type(9,11) });
    t.expect_type(stream_type(9));

    {
        using namespace polyhedral;

        statement *x = new statement;
        x->domain = {9,11};
        x->expr = new input_access(primitive_type::real, 0);

        stmt_access *xx = new stmt_access(x);
        xx->pattern = mapping(1,2);
        xx->pattern.coefficient(0,0) = 1;
        xx->pattern.constant(1) = 2;

        statement *out = new statement;
        out->domain = {9};
        out->expr = xx;

        t.expect_polyhedral_model({x,out});
    }

    return t.try_run(code);
}

}

}
}
