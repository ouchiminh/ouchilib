#include "../test.hpp"
#include "ouchilib/crypto/algorithm/secret_sharing.hpp"

#ifdef _DEBUG

DEFINE_TEST(test_secret_sharing_solve)
{
    using namespace ouchi::crypto;
    using ouchi::math::gf256;
    secret_sharing<> ss([i = (unsigned char)1]() mutable { return gf256<>{ i++ }; }, 3);
    auto y1 = ss.f(gf256<>{ 3 }, gf256<>{2});
    auto y2 = ss.f(gf256<>{ 2 }, gf256<>{2});
    auto y3 = ss.f(gf256<>{ 12 }, gf256<>{2});
    auto res = ss.solve({ gf256<>{3}, gf256<>{2}, gf256<>{12} },
                        { y1, y2, y3 });
    CHECK_EQUAL(res.value, 2);
}


#endif
