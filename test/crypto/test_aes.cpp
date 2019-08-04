#include <string>
#include "../test.hpp"
#include "ouchilib/crypto/algorithm/aes.hpp"
#include "ouchilib/utl/multiitr.hpp"

DEFINE_TEST(test_aes)
{
    char plain[16] = "123456789abcdef";
    char key[16] = "!!!!!!!!!!!!!!!";
    ouchi::crypto::algorithm::aes<16> encoder(ouchi::crypto::memory_view<16>((void*)key));

    char crypto[16];
    char decrypt[16];
    encoder.encrypt(plain, crypto);
    encoder.decrypt(crypto, decrypt);

    for (auto [p, d] : ouchi::multiitr{ plain, decrypt }) {
        CHECK_EQUAL(p, d);
    }
}
DEFINE_TEST(test_aes_memory)
{
    char buf[16] = "123456789abcdef";
    char key[16] = "!!!!!!!!!!!!!!!";
    ouchi::crypto::algorithm::aes<16> encoder(ouchi::crypto::memory_view<16>((void*)key));

    encoder.encrypt(buf, buf);
    encoder.decrypt(buf, buf);

    using namespace std::string_literals;
    CHECK_EQUAL("123456789abcdef"s, buf);
}
