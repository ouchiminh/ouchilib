#include "../test.hpp"
#include "ouchilib/crypto/encoder.hpp"
#include "ouchilib/crypto/algorithm/aes.hpp"

DEFINE_TEST(test_encoder_ecb) {
    using namespace ouchi::crypto;
    using namespace ouchi::crypto::algorithm;
    char plain[16] = "123456789abcdef";
    char key[16] = "!!!!!!!!!!!!!!!";
    encoder<ecb, aes128> cryptographer(key);
    char crypto[32];
    char decrypt[32];

    CHECK_EQUAL(cryptographer.encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    CHECK_EQUAL(cryptographer.decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);

    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
}
DEFINE_TEST(test_encoder_cbc) {
    using namespace ouchi::crypto;
    using namespace ouchi::crypto::algorithm;
    char plain[16] =    "123456789abcdef";
    char key[16] =      "!!!!!!!!!!!!!!!";
    char iv[16] =       "hogehogehogehog";
    encoder<cbc, aes128> cryptographer(iv, key);
    encoder<cbc, aes128> decoder(iv, key);

    char crypto[32];
    char decrypt[32];

    REQUIRE_EQUAL(cryptographer.encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    REQUIRE_EQUAL(decoder.decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);

    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
}
DEFINE_TEST(test_encoder_ctr) {
    using namespace ouchi::crypto;
    using namespace ouchi::crypto::algorithm;
    char plain[16] = "123456789abcdef";
    char key[16] =   "!!!!!!!!!!!!!!!";
    char nonce[16] = "hogehogehogehog";
    size_t ictr = 0;
    encoder<ctr, aes128> cryptographer(nonce, ictr, key);
    encoder<ctr, aes128> decoder(nonce, ictr, key);
    char crypto[32];
    char decrypt[32];

    REQUIRE_EQUAL(cryptographer.encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    REQUIRE_EQUAL(decoder.decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);

    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
}
