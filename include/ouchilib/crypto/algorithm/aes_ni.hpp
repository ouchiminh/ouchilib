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
        using vp = std::decay_t<std::add_volatile_t<decltype(key_.data)>>;
        using vpm = volatile __m128i*;
        std::fill(const_cast<vp>(key_.data), const_cast<vp>(key_.data + KeyLength), 0);

        for (auto [w,d] : ouchi::multiitr(w128, dw128)) {
            ouchi::crypto::secure_memset((vpm)&w, 0);
            ouchi::crypto::secure_memset((vpm)&d, 0);
        }
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
        //for (auto r = 1u; r < nr; ++r) {
        //    state = _mm_aesenc_si128(state, w128[r]);
        //}
        state = enc_round(state, std::make_index_sequence<nr-1>{});
        state = _mm_aesenclast_si128(state, w128[nr]);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(dest), state);
    }
    void decrypt(block_t src, void* dest) const noexcept
    {
        __m128i state = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src.data));
        state = _mm_xor_si128(state, dw128[nr]);
        //for (auto r = nr - 1; r > 0; --r) {
        //    state = _mm_aesdec_si128(state, dw128[r]);
        //}
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
        constexpr int Rcon[10] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36 };
        std::uint32_t temp;
        std::uint32_t w[60];

        std::memcpy(w, key_.data, KeyLength);
        for (size_t i = KeyLength / 4; i<nb*(nr + 1); i++) {
            temp = w[i - 1];
            if ((i % (KeyLength / 4)) == 0)
                temp = aes::subword(rotword(temp)) ^ Rcon[i / (KeyLength / 4) - 1];
            else if ((KeyLength / 4) > 6 && (i % (KeyLength / 4)) == 4)
                temp = aes::subword(temp);
            w[i] = w[i - (KeyLength / 4)] ^ temp;
        }
        // copy software-expanded key
        for (auto r : ouchi::step(nr + 1)) {
            w128[r] = _mm_loadu_si128(reinterpret_cast<__m128i*>(w + r * 4));
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
