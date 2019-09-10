#include <string>
#include "../test.hpp"
#include "ouchilib/crypto/algorithm/aes.hpp"
#include "ouchilib/crypto/algorithm/aes_ni.hpp"
#include "ouchilib/utl/multiitr.hpp"
#include "ouchilib/utl/time-measure.hpp"

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
    ouchi::crypto::memory_entity<16> plain{ "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff" };
    ouchi::crypto::memory_entity<16> buf{ plain };
    constexpr ouchi::crypto::memory_entity<16> key{ "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f" };
    ouchi::crypto::aes128_ni encoder(key);

    encoder.encrypt(buf, buf.data);
    CHECK_EQUAL(buf.data[0], 0x69);
    CHECK_EQUAL(buf.data[1], 0xc4);
    encoder.decrypt(buf, buf.data);

    using namespace std::string_literals;
    CHECK_EQUAL(plain, buf);
}

DEFINE_TEST(test_aes_ni)
{
    constexpr char buf[16] = "123456789abcdef";
    char code[2][16];
    char p[16];
    constexpr char key[16] = "!!!!!!!!!!!!!!!";
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

DEFINE_TEST(test_aes_ni_speed)
{
    static char buffer[8192] = { 1, 2, 3 };
    constexpr char key[32] = "!!!!!!!!!!!!!!!";
    ouchi::crypto::aes256_ni c(key);
    auto t = ouchi::measure([&c]() {
        for (auto i = 0u; i < sizeof buffer; i += 16) {
            c.encrypt(buffer + i, buffer + i);
        }
    });
    std::printf("%f kbps\n", (sizeof(buffer) / 1024) / (t.count() / (double)decltype(t)::period::den));
}
