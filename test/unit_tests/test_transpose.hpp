#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
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

/*
result stream3_to_2()
{
    string func("(S) = S{2}");
    return is_func_type(func, {stream_type(3,5,7)}, stream_type(5,3,7));
}

result stream4_to_3_2()
{
    string func("(S) = S{3,2}");
    return is_func_type(func, {stream_type(3,5,7,9)}, stream_type(7,5,3,9));
}
*/

}
}
}
