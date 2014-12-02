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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
}
}
