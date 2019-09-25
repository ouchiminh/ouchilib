#include <iostream>
#include "../test.hpp"
#include "ouchilib/crypto/algorithm/sha.hpp"

DEFINE_TEST(test_sha512_single_block)
{
    using namespace ouchi::literals;
    auto code = "abc"_sha512;
    CHECK_EQUAL(code[0], 0xDD);
}
DEFINE_TEST(test_sha512_2_block)
{
    using namespace ouchi::literals;
    auto code = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"_sha512;
    CHECK_EQUAL(code[0], 0x8e);
}

DEFINE_TEST(test_sha256_single_block)
{
    using namespace ouchi::literals;
    auto code = "abc"_sha256;
    CHECK_EQUAL(code[0], 0xba);
}
DEFINE_TEST(test_sha256_2_block)
{
    using namespace ouchi::literals;
    auto code = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"_sha256;
    CHECK_EQUAL(code[0], 0x24);
}
