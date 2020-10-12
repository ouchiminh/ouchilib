#include "ouchilib/utl/bytes.hpp"
#include "../test.hpp"
#include "ouchilib/utl/time-measure.hpp"

DEFINE_TEST(test_bytes_addition)
{
    ouchi::bytes<4> a{ 1,2,3,4 }, b{ 4,3,2,1 };
    ouchi::bytes<4> c = a|b;
    CHECK_EQUAL(c[0], (std::byte)(1|4));
    CHECK_EQUAL(c[1], (std::byte)(2|3));
    CHECK_EQUAL(c[2], (std::byte)(3|2));
    CHECK_EQUAL(c[3], (std::byte)(4|1));
}

DEFINE_TEST(test_bytes_operation)
{
    ouchi::bytes<4> a{ 1,2,3,4 }, b{ 4,3,2,1 };
    ouchi::bytes<4> c = (a ^ b);
    CHECK_EQUAL(c[0], (std::byte)(1^4));
    CHECK_EQUAL(c[1], (std::byte)(2^3));
    CHECK_EQUAL(c[2], (std::byte)(3^2));
    CHECK_EQUAL(c[3], (std::byte)(4^1));
}

template<size_t I>
ouchi::bytes<I> operator+(const ouchi::bytes<I>& r, const ouchi::bytes<I>& l)
{
    ouchi::bytes<I> res;
    for (unsigned i = 0; i < I; ++i) {
        res.data[i] = r.data[i]|l.data[i];
    }
    return res;
}

DEFINE_TEST(test_bytes_speed)
{
    ouchi::bytes<1024*1024> a{ 1,2,3,4 }, b{ 4,3,2,1 };
    ouchi::bytes<1024*1024> c;
    CHECK_TRUE(ouchi::measure([&c, a, b]() {
        for(int i = 0; i < 1; ++i)
        c = a+b+a+b+a+b+a+b+a+b+a+b+a; }).count() < 
    ouchi::measure([&c, a, b]() {
        for(int i = 0; i < 1; ++i)
        c = a|b|a|b|a|b|a|b|a|b|a|b|a; }).count());
}

