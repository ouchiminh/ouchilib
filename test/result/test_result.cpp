#include "../test.hpp"
#include "ouchilib/result/result.hpp"

DEFINE_TEST(test_result_instantiate)
{
    using namespace ouchi::result;

    {
        auto r = []()->result<int> {
            int a = 0;
            return ok(a);
        }();
    }
    {
        int a = 0;
        auto r = [&a]()->result<int&> {
            return ok(a);
        }();
        auto& i = r.unwrap();
        a = 4;
        CHECK_EQUAL(i, 4);
    }
}

DEFINE_TEST(test_result_unwrap)
{
    using namespace ouchi::result;
    auto s = []()->result<int> {return ok(10); }();
    auto e = []()->result<int, bool> {return err(false); }();
    
    CHECK_EQUAL(s.unwrap(), 10);
    CHECK_EQUAL(e.unwrap_err(), false);

    CHECK_THROW(s.unwrap_err());
    CHECK_THROW(e.unwrap());

    CHECK_EQUAL(s.unwrap_or(5), 10);
    CHECK_EQUAL(e.unwrap_or(5), 5);

    CHECK_EQUAL(s.unwrap_or_else([](auto) {return 5; }), 10);
    CHECK_EQUAL(e.unwrap_or_else([](auto b) {return (int)b; }), 0);
}

DEFINE_TEST(test_result_map)
{
    using namespace ouchi::result;
    using namespace std::literals;
    {
        auto s = []()->result<int> {return ok(10); }();
        auto e = []()->result<int, bool> {return err(false); }();

        CHECK_EQUAL(s.map([](int a) {return std::to_string(a); }).unwrap(),
                    "10"s);
        CHECK_EQUAL(e.map([](int) {return 0; }).unwrap_err(), false);
    }
    {
        int a = 10;
        bool b = false;
        auto s = [&a]()->result<int&> {return ok(a); }();
        auto e = [&b]()->result<int, bool&> {return err(b); }();

        CHECK_EQUAL(s.map([](int a) {return std::to_string(a); }).unwrap(),
                    "10"s);
        CHECK_EQUAL(e.map([](int) {return 0; }).unwrap_err(), false);
    }
    {
        constexpr result<int, int> r1 = ok(10);
        constexpr result<double> r2 = ok(4);
        constexpr result<int, int> e1 = err(1);
        CHECK_TRUE(r1 && r2);
        CHECK_TRUE(r1 || e1);
        CHECK_TRUE(!(r1 && e1));

    }
}
