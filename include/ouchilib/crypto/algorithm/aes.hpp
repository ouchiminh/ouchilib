#pragma once
#include <cstdlib>
#include <cstdint>
#include <utility>
#include "ouchilib/utl/step.hpp"
#include "../common.h"

namespace ouchi::crypto {

inline std::uint32_t rotword(std::uint32_t in) {
	uint32_t inw = in, inw2 = 0;
	unsigned char *cin = reinterpret_cast<unsigned char*>(&inw);
	unsigned char *cin2 = reinterpret_cast<unsigned char*>(&inw2);
	cin2[0] = cin[1];
	cin2[1] = cin[2];
	cin2[2] = cin[3];
	cin2[3] = cin[0];
	return(inw2);
}

inline std::uint32_t mul(std::uint32_t dt, std::uint32_t n)
{
	int x = 0;
	for (int i = 8; i>0; i >>= 1) {
		x <<= 1;
		if (x & 0x100)
			x = (x ^ 0x1b) & 0xff;
		if ((n & i))
			x ^= dt;
	}
	return(x);
}

inline std::uint32_t dataget(void * data, std::uint32_t n) {
	return (reinterpret_cast<std::uint8_t*>(data)[n]);
}

template<size_t KeyLength>  // size in byte : 16,24,32
struct aes {
    static_assert(KeyLength == 16 || KeyLength == 24 || KeyLength == 32);
    static constexpr size_t block_size = 16;
    static constexpr size_t nb = 4;
    using block_t = memory_view<block_size>;
    using key_t = memory_entity<KeyLength>;
    using key_view = memory_view<KeyLength>;
private:
    static constexpr size_t nr = KeyLength / 4 + 6;
    static constexpr std::uint8_t sbox[256] = {
        0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
        0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
        0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
        0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
        0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
        0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
        0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
        0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
        0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
        0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
        0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
        0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
        0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
        0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
        0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
        0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
    };
    static constexpr std::uint8_t inv_sbox[256] = {
        0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
        0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
        0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
        0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
        0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
        0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
        0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
        0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
        0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
        0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
        0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
        0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
        0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
        0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
        0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
        0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
    };

public:
    aes(key_view key) {
        set_key(key);
    }
    aes() = default;
    ~aes()
    {
        using vp = std::decay_t<std::add_volatile_t<decltype(key_.data)>>;
        std::fill(const_cast<vp>(key_.data), const_cast<vp>(key_.data + KeyLength), 0);
        using wvp = std::decay_t<std::add_volatile_t<decltype(w_)>>;
        std::fill(const_cast<wvp>(w_), const_cast<wvp>(w_ + 60), 0);
    }

    void set_key(key_view key) noexcept
    {
        std::memcpy(key_.data, key.data, KeyLength);
        expand_key();
    }

    void encrypt(block_t src, void* dest) const noexcept
    {
        std::memmove(dest, src.data, block_size);
        add_roundkey(dest, 0);
        for (auto i : ouchi::step(1u, (std::uint32_t)nr)) {
            sub_bytes(dest);
            shift_rows(dest);
            mix_columns(dest);
            add_roundkey(dest, i);
        }
        sub_bytes(dest);
        shift_rows(dest);
        add_roundkey(dest, nr);
    }
    void decrypt(block_t src, void* dest) const noexcept
    {
        std::memmove(dest, src.data, block_size);
		add_roundkey(dest, (unsigned)nr);
        unsigned i;
		for (i = nr - 1; i > 0; i--) {
			inv_shift_rows(dest);
			inv_sub_bytes(dest);
			add_roundkey(dest, (int)i);
			inv_mix_columns(dest);
		}

		inv_shift_rows(dest);
		inv_sub_bytes(dest);
		add_roundkey(dest, 0);
    }

private:
    std::uint32_t subword(std::uint32_t in) const noexcept
    {
        uint32_t inw = in;
        unsigned char* cin = (unsigned char*)&inw;
        cin[0] = sbox[cin[0]];
        cin[1] = sbox[cin[1]];
        cin[2] = sbox[cin[2]];
        cin[3] = sbox[cin[3]];
        return (inw);
    }
    void expand_key() noexcept
    {
    /* FIPS 197  P.27 Appendix A.1 Rcon[i/Nk] */ //又は mulを使用する
        constexpr int Rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };
        std::uint32_t temp;

        std::memcpy(w_, key_.data, KeyLength);
        for (size_t i = KeyLength / 4; i<nb*(nr + 1); i++) {
            temp = w_[i - 1];
            if ((i % (KeyLength / 4)) == 0)
                temp = subword(rotword(temp)) ^ Rcon[i / (KeyLength / 4) - 1];
            else if ((KeyLength / 4) > 6 && (i % (KeyLength / 4)) == 4)
                temp = subword(temp);
            w_[i] = w_[i - (KeyLength / 4)] ^ temp;
        }
    }
    void add_roundkey(void* data, unsigned round) const noexcept
    {
        auto ptr = reinterpret_cast<std::uint32_t*>(data);
        for (auto i : ouchi::step(4)) {
            ptr[i] ^= w_[i + round * 4];
        }
    }

    void sub_bytes(void* data) const noexcept
    {
        unsigned char* cb = reinterpret_cast<unsigned char*>(data);
        for (auto i : ouchi ::step(16)) {
            cb[i] = sbox[cb[i]];
        }
    }
    void inv_sub_bytes(void* data) const noexcept
    {
        unsigned char* cb = reinterpret_cast<unsigned char*>(data);
        for (auto i : ouchi ::step(16)) {
            cb[i] = inv_sbox[cb[i]];
        }
    }

    void shift_rows(void* data) const noexcept
    {
        auto ptr{ (uint8_t*)data };
        memory_entity<block_size> cw(data);
        for (int i = 0; i<nb; i += 4) {
            int i4 = i * 4;
            for (int j = 1; j<4; j++) {
                cw.data[i4 + j + 0 * 4] = ptr[i4 + j + ((j + 0) & 3) * 4];
                cw.data[i4 + j + 1 * 4] = ptr[i4 + j + ((j + 1) & 3) * 4];
                cw.data[i4 + j + 2 * 4] = ptr[i4 + j + ((j + 2) & 3) * 4];
                cw.data[i4 + j + 3 * 4] = ptr[i4 + j + ((j + 3) & 3) * 4];
            }
        }
        std::memcpy(data, cw.data, block_size);
    }
    void inv_shift_rows(void* data) const noexcept
    {
        auto ptr{ (uint8_t*)data };
        memory_entity<block_size> cw(data);
        for (int i = 0; i<nb; i += 4) {
            int i4 = i * 4;
            for (int j = 1; j<4; j++) {
                cw.data[i4 + j + ((j + 0) & 3) * 4] = ptr[i4 + j + 0 * 4];
                cw.data[i4 + j + ((j + 1) & 3) * 4] = ptr[i4 + j + 1 * 4];
                cw.data[i4 + j + ((j + 2) & 3) * 4] = ptr[i4 + j + 2 * 4];
                cw.data[i4 + j + ((j + 3) & 3) * 4] = ptr[i4 + j + 3 * 4];
            }
        }
        std::memcpy(data, cw.data, block_size);
    }

    void mix_columns(void* data) const noexcept
    {
        std::uint32_t i4, x;
        auto adata = static_cast<std::uint8_t*>(data);
        for (auto i : ouchi::step((std::uint32_t)nb)) {
            i4 = i * 4;
            x = mul(dataget(adata, i4 + 0), 2) ^
                mul(dataget(adata, i4 + 1), 3) ^
                mul(dataget(adata, i4 + 2), 1) ^
                mul(dataget(adata, i4 + 3), 1);
            x |= (mul(dataget(adata, i4 + 1), 2) ^
                  mul(dataget(adata, i4 + 2), 3) ^
                  mul(dataget(adata, i4 + 3), 1) ^
                  mul(dataget(adata, i4 + 0), 1)) << 8;
            x |= (mul(dataget(adata, i4 + 2), 2) ^
                  mul(dataget(adata, i4 + 3), 3) ^
                  mul(dataget(adata, i4 + 0), 1) ^
                  mul(dataget(adata, i4 + 1), 1)) << 16;
            x |= (mul(dataget(adata, i4 + 3), 2) ^
                  mul(dataget(adata, i4 + 0), 3) ^
                  mul(dataget(adata, i4 + 1), 1) ^
                  mul(dataget(adata, i4 + 2), 1)) << 24;
            std::memcpy(adata + i * 4, &x, sizeof(x));
        }
    }
    void inv_mix_columns(void* data) const noexcept
    {
        std::uint32_t i4, x;
        auto adata = static_cast<std::uint8_t*>(data);
        for (auto i : ouchi::step((std::uint32_t)nb)) {
            i4 = i * 4;
            x = mul(dataget(adata, i4 + 0), 14) ^
                mul(dataget(adata, i4 + 1), 11) ^
                mul(dataget(adata, i4 + 2), 13) ^
                mul(dataget(adata, i4 + 3), 9);
            x |= (mul(dataget(adata, i4 + 1), 14) ^
                  mul(dataget(adata, i4 + 2), 11) ^
                  mul(dataget(adata, i4 + 3), 13) ^
                  mul(dataget(adata, i4 + 0), 9)) << 8;
            x |= (mul(dataget(adata, i4 + 2), 14) ^
                  mul(dataget(adata, i4 + 3), 11) ^
                  mul(dataget(adata, i4 + 0), 13) ^
                  mul(dataget(adata, i4 + 1), 9)) << 16;
            x |= (mul(dataget(adata, i4 + 3), 14) ^
                  mul(dataget(adata, i4 + 0), 11) ^
                  mul(dataget(adata, i4 + 1), 13) ^
                  mul(dataget(adata, i4 + 2), 9)) << 24;
            std::memcpy(adata + i * 4, &x, sizeof(x));
        }
    }
    key_t key_;
    std::uint32_t w_[60];    // expanded key
};

using aes128 = aes<16>;
using aes192 = aes<24>;
using aes256 = aes<32>;

}
