#include "unit_test.hpp"

#include <sstream>

namespace stream {
namespace unit_testing {
namespace binop {

result ii()
{
    std::istringstream code("result = 1 + 2");
    test t;
    t.expect_type(int_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<int>(1), new constant<int>(2)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result ir()
{
    std::istringstream code("result = 1 + 2.3");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<int>(1), new constant<double>(2.3)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result ri()
{
    std::istringstream code("result = 2.3 + 1");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<double>(2.3), new constant<int>(1)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result rr()
{
    std::istringstream code("result = 2.3 + 3.4");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<double>(2.3), new constant<double>(3.4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result iR()
{
    std::istringstream code("result = 1 + 3..10");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<int>(1), new iterator_access(0,3)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result rR()
{
    std::istringstream code("result = 2.7 + 3..10");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new constant<double>(2.7), new iterator_access(0,3)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result Ri()
{
    std::istringstream code("result = 3..10 + 222");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new constant<int>(222)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result Rr()
{
    std::istringstream code("result = 3..10 + 2.34");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new constant<double>(2.34)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result RR()
{
    std::istringstream code("result = 3..10 + 4..11");
    test t;
    t.expect_type(stream_type(8));
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {8};
        stmt->expr = new intrinsic(intrinsic::add,
        {new iterator_access(0,3), new iterator_access(0,4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result iS()
{
    test t;
    std::istringstream code("result(x) = 2 + x");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {new constant<int>(2), x});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result Si()
{
    test t;
    std::istringstream code("result(x) = x + 2");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {x, new constant<int>(2)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}


result rS()
{
    test t;
    std::istringstream code("result(x) = 3.4 + x");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {new constant<double>(3.4), x});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

result Sr()
{
    test t;
    std::istringstream code("result(x) = x + 3.4");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::add, {x, new constant<double>(3.4)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}


result raise_int_int()
{
    std::istringstream code("result = 2 ^ 4");
    test t;
    t.expect_type(int_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::raise,
        {new constant<int>(2), new constant<int>(4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result raise_real_real()
{
    std::istringstream code("result = 2.2 ^ 4.4");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::raise,
        {new constant<double>(2.2), new constant<double>(4.4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result raise_int_real()
{
    std::istringstream code("result = 2 ^ 4.4");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::raise,
        {new constant<int>(2), new constant<double>(4.4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result raise_real_int()
{
    std::istringstream code("result = 2.2 ^ 4");
    test t;
    t.expect_type(real_type());
    {
        using namespace polyhedral;
        statement *stmt = new statement;
        stmt->domain = {1};
        stmt->expr = new intrinsic(intrinsic::raise,
        {new constant<double>(2.2), new constant<int>(4)});
        t.expect_polyhedral_model({stmt});
    }
    return run(t, code, "result");
}

result raise_stream_int()
{
    test t;
    std::istringstream code("result(x) = x ^ 3");

    t.expect_type(stream_type(4,5,6));

    {
        using namespace polyhedral;

        statement *in = new statement;
        in->domain = {4,5,6};
        in->expr = new input_access(0);

        stream_access *x = new stream_access;
        x->target = in;
        x->pattern = mapping::identity(3,3);

        statement *out = new statement;
        out->domain = {4,5,6};
        out->expr = new intrinsic(intrinsic::raise, {x, new constant<int>(3)});

        t.expect_polyhedral_model({in,out});
    }

    return run(t, code, "result", { stream_type(4,5,6) });
}

}
}
}
