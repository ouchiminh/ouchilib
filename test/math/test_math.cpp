﻿#include "../test.hpp"
#include "ouchilib/math/gf.hpp"
#include "ouchilib/utl/step.hpp"
#include "ouchilib/math/infinity.hpp"
#include "ouchilib/math/modint.hpp"

DEFINE_TEST(test_gf)
{
    using gf256 = ouchi::math::gf<unsigned char, 0x1b>;
    static constexpr std::uint8_t mul2[] = {
        0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
        0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
        0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
        0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,
        0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
        0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
        0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
        0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
        0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
        0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
        0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
        0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
        0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
        0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
        0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
        0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
    };
    for (auto i : ouchi::step(256)) {
        CHECK_EQUAL(gf256::mul(0x02, (std::uint8_t)i), mul2[i]);
    }
    for (auto i : ouchi::step(256)) {
        CHECK_EQUAL(gf256::mul(0x03, (std::uint8_t)i), i ^ mul2[i]);
    }
}

DEFINE_TEST(test_pow)
{
    using gf256 = ouchi::math::gf<unsigned char, 0x1b>;
    for (auto i : ouchi::step(1, 255)) {
        gf256 a((std::uint8_t)i);
        CHECK_EQUAL((gf256{ gf256::power((std::uint8_t)i, 254) } * a).value, 1);
    }
}

DEFINE_TEST(test_pow2)
{
    using namespace ouchi::math;
    constexpr std::uint8_t a = 0x11;
    auto a2 = gf256<>::mul(a, a);
    CHECK_EQUAL(gf256<>::power(a, 4), gf256<>::mul(a2, a2));
    CHECK_EQUAL(gf256<>::power(a, 5), gf256<>::mul(a, gf256<>::mul(a2, a2)));
    CHECK_EQUAL(gf256<>::power(a, 0), 1);
}

DEFINE_TEST(test_gfinv)
{
    using namespace ouchi::math;
    for (auto i : ouchi::step(1, 256)) {
        CHECK_EQUAL(gf256<>::mul(gf256<>::inv((std::uint8_t)i), (std::uint8_t)i), 1);
    }
    for (auto i : ouchi::step(1, 5)) {
        CHECK_EQUAL(gf2_16<>::mul(gf2_16<>::inv((std::uint8_t)i), (std::uint8_t)i), 1);
    }
    for (auto i : ouchi::step(1, 25)) {
        CHECK_EQUAL(gf2_32<>::mul(gf2_32<>::inv((std::uint8_t)i), (std::uint8_t)i), 1);
    }
}

DEFINE_TEST(test_inf)
{
    using namespace ouchi::math;
    CHECK_TRUE(inf > 1);
    CHECK_TRUE(1 < inf);
}
DEFINE_TEST(test_ex_gcd)
{
    using namespace ouchi::math;
    auto [d,x,y] = ex_gcd(3, 13);
    std::cout << d << '=' << x*3 << '+' << y*13 << '\n';
    std::cout << x % 13 << '\n';
}

DEFINE_TEST(test_modint)
{
    using namespace ouchi::math;
    modint<int, long long> a(1, 13), b(2, 13);
    CHECK_EQUAL(a + b, 3);
    CHECK_EQUAL(a - b, a + (-b));
    CHECK_EQUAL(a / b, 7);
}

DEFINE_TEST(test_modpow)
{
    using namespace ouchi::math;
    modint<int, long long> g(2, 997);
    for (int i = 0; i < 100; i++) {
        modint<int, long long> res(1, g.mod());
        int j = i;
        while (j-->0) res *= g;
        CHECK_EQUAL(res, pow(g, i));
    }
}

DEFINE_TEST(test_modint_cipher)
{
    using namespace ouchi::math;
    modint<int> g(2, 997);
    constexpr int a = 2, b = 5;
    auto alpha = g*g, beta = g*g*g*g*g;
    modint<int> M(8, 997);
    auto K = alpha * beta;
    CHECK_EQUAL(beta * alpha, K);
    CHECK_EQUAL(alpha, pow(g, 2));
    CHECK_EQUAL(beta, pow(g, 5));
}

