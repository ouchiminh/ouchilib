#include <iostream>
#include "../test.hpp"
#include "ouchilib/crypto/algorithm/sha.hpp"

DEFINE_TEST(test_sha512_single_block)
{
    using namespace ouchi::literals;
    auto code = "abc"_sha512;
    for (auto i : code.data) {
        std::cout << std::hex << (int)i << ' ';
    }
    std::cout << std::endl;
}
DEFINE_TEST(test_sha512_2_block)
{
    using namespace ouchi::literals;
    auto code = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu"_sha512;
    for (auto i : code.data) {
        std::cout << std::hex << (int)i << ' ';
    }
    std::cout << std::endl;
}
