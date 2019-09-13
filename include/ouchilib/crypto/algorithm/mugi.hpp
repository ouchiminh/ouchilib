#pragma once
#include <type_traits>
#include "ouchilib/math/gf.hpp"
#include "ouchilib/math/matrix.hpp"
#include "../common.hpp"
#include "aes.hpp"

namespace ouchi::crypto {

struct mugi {
    static constexpr unsigned vec_size = 16;
    using result_type = std::uint64_t;
    mugi() = default;
    mugi(memory_view<vec_size> key, memory_view<vec_size> iv)
        : a_{ detail::pack<std::uint64_t>(key.data),
             detail::pack<std::uint64_t>(key.data+8),
             (rotl(detail::pack<std::uint64_t>(key.data), 7)) ^ (rotr(detail::pack<std::uint64_t>(key.data+8), 7)) ^ c[0] }
        , b_{}
    {
        constexpr std::uint64_t zero[16] = {};
        for (auto i = 0u; i < 16; ++i) {
            rho(zero);
            b_[15-i] = a_[0];
        }
        a_[0] ^= detail::pack<std::uint64_t>(iv.data);
        a_[1] ^= detail::pack<std::uint64_t>(iv.data + 8);
        a_[2] ^= rotl(detail::pack<std::uint64_t>(iv.data), 7) ^
            rotr(detail::pack<std::uint64_t>(iv.data + 8), 7) ^ c[0];
        for (auto i = 0u; i < 16; ++i) rho(zero);
        for (auto i = 0u; i < 16; ++i) update();
    }

    result_type operator()() noexcept
    {
        auto cp = a_[2];
        update();
        return cp;
    }

private:
    std::uint64_t a_[3];
    std::uint64_t b_[16];
    static constexpr std::uint64_t c[3] = {
        0x6A09E667F3BCC908,0xBB67AE8584CAA73B,0x3C6EF372FE94F82B
    };
    mutable ouchi::math::matrix<ouchi::math::gf256, 4, 1> xm_;
    void update()
    {
        std::uint64_t at[3] = { a_[0], a_[1], a_[2] };
        rho(b_); lambda(at);
    }
    void rho(const std::uint64_t (&b)[16])
    {
        std::uint64_t a0 = a_[0], a1 = a_[1];
        a_[0] = a_[1];
        a_[1] = a_[2] ^ F(a_[1], b[4]) ^ c[1];
        a_[2] = a0 ^ F(a1, rotl(b[10], 17)) ^ c[2];
    }
    void lambda(const std::uint64_t (&a)[3])
    {
        std::uint64_t bt[16];
        std::memcpy(bt, b_, sizeof(bt));
        for (auto i :{ 1,2,3,5,6,7,8,9,11,12,13,14,15 }) {
            b_[i] = bt[i - 1];
        }
        b_[0] = bt[15] ^ a[0];
        b_[4] = bt[3] ^ bt[7];
        b_[10] = bt[9] ^ rotl(bt[13], 32);
    }
    std::uint64_t F(std::uint64_t x, std::uint64_t b) const
    {
        std::uint8_t o[8] = {};
        detail::unpack(x ^ b, o);
        for (auto& i : o) { i = aes128::subchar(i); }
        std::uint32_t ph = detail::pack<std::uint32_t>(o);
        std::uint32_t pl = detail::pack<std::uint32_t>(o + 4);
        auto qh = M(ph), ql = M(pl);
        return 
            (((ql >> 24) & 0xFFull) << 56) |
            (((ql >> 16) & 0xFFull) << 48) |
            (((qh >> 8) & 0xFFull) << 40) |
            (((qh >> 0) & 0xFFull) << 32) |
            (((qh >> 24) & 0xFFull) << 24) |
            (((qh >> 16) & 0xFFull) << 16) |
            (((ql >> 8) & 0xFFull) << 8) |
            ((ql >> 0) & 0xFFull);
    }
    std::uint32_t M(std::uint32_t x) const
    {
        using namespace ouchi::math;
        static const matrix<gf256, 4, 4> m{
            gf256{0x02},gf256{0x03},gf256{0x01},gf256{0x01},
            gf256{0x01},gf256{0x02},gf256{0x03},gf256{0x01},
            gf256{0x01},gf256{0x01},gf256{0x02},gf256{0x03},
            gf256{0x03},gf256{0x01},gf256{0x01},gf256{0x02}
        };
        xm_.assign({
            gf256{(x >> 24)&0xff},
            gf256{(x >> 16)&0xff},
            gf256{(x >> 8)&0xff},
            gf256{(x >> 0)&0xff}
        });
        
        auto r = m * xm_;
        return detail::pack<std::uint32_t>(r.data());
    }
};

}
