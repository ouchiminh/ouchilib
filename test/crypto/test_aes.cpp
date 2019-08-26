#include <string>
#include "../test.hpp"
#include "ouchilib/crypto/algorithm/aes.hpp"
#include "ouchilib/crypto/algorithm/aes_ni.hpp"
#include "ouchilib/utl/multiitr.hpp"

DEFINE_TEST(test_aes)
{
    char plain[16] = "123456789abcdef";
    char key[16] = "!!!!!!!!!!!!!!!";
    ouchi::crypto::aes<16> encoder(ouchi::crypto::memory_view<16>((void*)key));

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
    ouchi::crypto::aes<16> encoder(ouchi::crypto::memory_view<16>((void*)key));

    encoder.encrypt(buf, buf);
    encoder.decrypt(buf, buf);

    using namespace std::string_literals;
    CHECK_EQUAL("123456789abcdef"s, buf);
}

DEFINE_TEST(test_aes_ni)
{
    constexpr char buf[16] = "123456789abcdef";
    char code[2][16];
    char p[16];
    char key[16] = "!!!!!!!!!!!!!!!";
    ouchi::crypto::aes128_ni encoder(ouchi::crypto::memory_view<16>((void*)key));
    ouchi::crypto::aes128 software_encoder(ouchi::crypto::memory_view<16>((void*)key));
    encoder.encrypt(buf, code[0]);
    software_encoder.encrypt(buf, code[1]);
    encoder.decrypt(code[0], p);

    using namespace std::string_literals;
    CHECK_EQUAL("123456789abcdef"s, p);
    for (auto [a, b] : ouchi::multiitr{ code[0], code[1] })
        CHECK_EQUAL(a, b);
}
