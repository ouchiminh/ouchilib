#pragma once
#include <wmmintrin.h>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include "ouchilib/utl/step.hpp"
#include "aes.hpp"
#include "../common.hpp"

namespace ouchi::crypto {

template<size_t KeyLength>
struct aes_ni {
    static_assert(KeyLength == 16 || KeyLength == 24 || KeyLength == 32);
private:
    static constexpr std::uint_fast32_t nb = 4;
    static constexpr std::uint_fast32_t nk = KeyLength / 4;
    static constexpr std::uint_fast32_t nr = nk + 6;
public:
    static constexpr size_t block_size = nb * 4;
    using block_t = memory_view<block_size>;
    using key_t = memory_entity<KeyLength>;
    using key_view = memory_view<KeyLength>;

    aes_ni(key_view key)
    {
        set_key(key);
    }
    aes_ni() = default;
    ~aes_ni()
    {
        ouchi::crypto::secure_memset(w128, 0);
        ouchi::crypto::secure_memset(dw128, 0);
        ouchi::crypto::secure_memset(key_.data, 0);
    }
    void set_key(key_view key) noexcept
    {
        std::memcpy(key_.data, key.data, KeyLength);
        expand_key();
    }
    void encrypt(block_t src, void* dest) const noexcept
    {
        __m128i state = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src.data));
        state = _mm_xor_si128(state, w128[0]);
        state = enc_round(state, std::make_index_sequence<nr-1>{});
        state = _mm_aesenclast_si128(state, w128[nr]);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(dest), state);
    }
    void decrypt(block_t src, void* dest) const noexcept
    {
        __m128i state = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src.data));
        state = _mm_xor_si128(state, dw128[nr]);
        state = dec_round(state, std::make_index_sequence<nr-1>{});
        state = _mm_aesdeclast_si128(state, dw128[0]);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(dest), state);
    }
private:
    __m128i w128[nr + 1];
    __m128i dw128[nr + 1];
    key_t key_;
    template<size_t ...S>
    inline [[nodiscard]] __m128i enc_round(__m128i state, std::index_sequence<S...>) const noexcept
    {
        ((state = _mm_aesenc_si128(state, w128[S + 1])), ...);
        return state;
    }
    template<size_t ...S>
    inline [[nodiscard]] __m128i dec_round(__m128i state, std::index_sequence<S...>) const noexcept
    {
        ((state = _mm_aesdec_si128(state, dw128[nr - S - 1])), ...);
        return state;
    }

    void expand_key()
    {
        using aes = aes<KeyLength>;
        std::uint32_t w[60];
        constexpr std::uint32_t Rcon[10] = {
            0x0100'0000,0x0200'0000,0x0400'0000,0x0800'0000,0x100'00000,
            0x2000'0000,0x4000'0000,0x8000'0000,0x1b00'0000,0x3600'0000 };
        constexpr auto nk = KeyLength / 4;
        size_t i;

        for (i = 0u; i < nk; ++i) {
            w[i] = detail::pack<std::uint32_t>(key_.data + i * 4);
        }

        for (i = nk; i<nb*(nr + 1); ++i) {
            std::uint32_t temp = w[i - 1];
            if ((i % nk) == 0)
                temp = aes::subword(rotword(temp)) ^ Rcon[i / nk - 1];
            else if (nk > 6 && (i % nk) == 4)
                temp = aes::subword(temp);
            w[i] = w[i - nk] ^ temp;
        }
        // copy expanded key
        for (auto r : ouchi::step(nr + 1)) {
            alignas(16) std::uint8_t tmp[16];
            detail::unpack(w[r*4], tmp);
            detail::unpack(w[r*4+1], tmp+4);
            detail::unpack(w[r*4+2], tmp+8);
            detail::unpack(w[r*4+3], tmp+12);
            w128[r] = _mm_load_si128((__m128i*)tmp);
        }
        dw128[0] = w128[0];
        for (auto i : ouchi::step(1u, nr)) {
            dw128[i] = _mm_aesimc_si128(w128[i]);
        }
        dw128[nr] = w128[nr];
    }

};

using aes128_ni = aes_ni<16>;
using aes192_ni = aes_ni<24>;
using aes256_ni = aes_ni<32>;
}
