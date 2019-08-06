﻿#include <sstream>
#include "../test.hpp"
#include "ouchilib/crypto/encoder.hpp"
#include "ouchilib/crypto/algorithm/aes.hpp"

DEFINE_TEST(test_encoder_ecb) {
    using namespace ouchi::crypto;
    char plain[16] = "123456789abcdef";
    char key[16] = "!!!!!!!!!!!!!!!";
    block_encoder<ecb, aes128> cryptographer(key);
    char crypto[32];
    char decrypt[32];

    CHECK_EQUAL(cryptographer.encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    CHECK_EQUAL(cryptographer.decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);

    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
    std::stringstream plains(plain);
    std::stringstream cr;
    std::stringstream dc;

    cryptographer.encrypt(plains, cr);
    cryptographer.decrypt(cr, dc);

    CHECK_EQUAL(plains.str(), dc.str());
}

DEFINE_TEST(test_encoder_cbc) {
    using namespace ouchi::crypto;
    const char plain[16] = "123456789abcdef";
    const char key[16] =   "!!!!!!!!!!!!!!!";
    const char iv[16] =    "hogehogehogehog";

    block_encoder<cbc, aes128> cg[2] = { {iv, key}, {iv, key} };

    char crypto[32];
    char decrypt[32];

    REQUIRE_EQUAL(cg[0].encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    REQUIRE_EQUAL(cg[1].decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);
    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
}
DEFINE_TEST(test_encoder_ctr) {
    using namespace ouchi::crypto;
    char plain[16] = "123456789abcdef";
    char key[16] =   "!!!!!!!!!!!!!!!";
    char nonce[16] = "hogehogehogehog";
    size_t ictr = 0;
    block_encoder<ctr, aes128> cg[2] = { {nonce, ictr, key}, {nonce, ictr, key} };
    char crypto[32];
    char decrypt[32];

    REQUIRE_EQUAL(cg[0].encrypt(plain, sizeof(plain), crypto, sizeof(crypto)), 32);
    REQUIRE_EQUAL(cg[1].decrypt(crypto, sizeof(crypto), decrypt, sizeof(decrypt)), 16);

    for (auto i : ouchi::step(16)) {
        CHECK_EQUAL(plain[i], decrypt[i]);
    }
}