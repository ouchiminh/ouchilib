﻿#include "../test.hpp"
#include "ouchilib/crypto/algorithm/secret_sharing.hpp"
#include <random>
#include <string_view>

#ifndef NDEBUG

DEFINE_TEST(test_secret_sharing_solve)
{
    using namespace ouchi::crypto;
    using ouchi::math::gf256;
    secret_sharing<> ss([i = (unsigned char)1]() mutable { return gf256<>{ i++ }; }, 3);
    auto y1 = ss.f(gf256<>{ 3 }, gf256<>{2});
    auto y2 = ss.f(gf256<>{ 2 }, gf256<>{2});
    auto y3 = ss.f(gf256<>{ 12 }, gf256<>{2});
    auto res = ss.solve({ gf256<>{3}, gf256<>{2}, gf256<>{12} },
                        { y1, y2, y3 }).unwrap();
    CHECK_EQUAL(res.value, 2);
}

#endif

DEFINE_TEST(test_secret_sharing_share_creation_and_recover)
{
    using namespace ouchi::crypto;
    using ouchi::math::gf256;
    using namespace std::string_view_literals;
    char ans[256] = {};
    secret_sharing<> ss([r = std::mt19937{ std::random_device{}() }]() mutable {return gf256<>{r() & 0xff}; },
                        2);
    ss.push("hogehoge");
    ss.push("fugafuga");
    auto share = { ss.get_share(1),ss.get_share(2),ss.get_share(3) };
    CHECK_EQUAL(ss.recover_secret(ans, sizeof(ans), share).unwrap(), 16);
    CHECK_EQUAL(ans, "hogehogefugafuga"sv);
}

DEFINE_TEST(test_secret_sharing_error_handling)
{
    using namespace ouchi::crypto;
    using ouchi::math::gf256;
    char ans[256] = {};
    secret_sharing<> ss([r = std::mt19937{ std::random_device{}() }]() mutable {return gf256<>{r() & 0xff}; },
                        2);
    ss.push("hogehoge");
    ss.push("fugafuga");
    CHECK_TRUE(!ss.recover_secret(ans, sizeof(ans), { ss.get_share(1) }));
    CHECK_TRUE(!ss.recover_secret(ans, sizeof(ans), { ss.get_share(1), ss.get_share(1) }));
    CHECK_TRUE(!ss.recover_secret(ans, 0, { ss.get_share(1), ss.get_share(2) }));
}
