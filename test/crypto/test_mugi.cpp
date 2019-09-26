#include "../test.hpp"
#include "ouchilib/crypto/algorithm/mugi.hpp"

DEFINE_TEST(test_mugi)
{
    using namespace ouchi::crypto;
    {
        memory_entity<16> me{};
        mugi rnd{ me, me };
        CHECK_EQUAL(rnd(), 0xc76e14e70836e6b6);
        CHECK_EQUAL(rnd(), 0xcb0e9c5a0bf03e1e);
        CHECK_EQUAL(rnd(), 0x0acf9af49ebe6d67);
        CHECK_EQUAL(rnd(), 0xd5726e374b1397ac);
    }
    {
        std::uint8_t key[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
        std::uint8_t iv[] = {0xf0,0xe0,0xd0,0xc0,0xb0,0xa0,0x90,0x80,0x70,0x60,0x50,0x40,0x30,0x20,0x10,0x00};
        mugi rnd{ key, iv };
        CHECK_EQUAL(rnd(), 0xbc62430614b79b71);
        CHECK_EQUAL(rnd(), 0x71a66681c35542de);
        CHECK_EQUAL(rnd(), 0x7aba5b4fb80e82d7);
        CHECK_EQUAL(rnd(), 0x0b96982890b6e143);
    }
}
