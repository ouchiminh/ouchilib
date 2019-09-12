#pragma once
#include <type_traits>
#include <cstdint>

namespace ouchi::math {

/// <summary>
/// F : Intで表現される拡大体の既約多項式のビット表現
/// </summary>
template<class Int, Int F, std::enable_if_t<std::is_integral_v<Int> && std::is_unsigned_v<Int>>* = nullptr>
struct gf {
    Int value;
    gf() = default;
    explicit constexpr gf(Int v) : value{ v } {}

    explicit constexpr operator Int() const noexcept { return value; }

    static constexpr Int add(Int lhs, Int rhs) noexcept
    {
        return lhs ^ rhs;
    }
    static constexpr Int mul(Int lhs, Int rhs) noexcept
    {
        Int res{};
        while (lhs) {
            res ^= (lhs & 1 ? rhs : 0);
            rhs = xmul(rhs);
            lhs >>= 1;
        }
        return res;
    }
    static constexpr Int xmul(Int v) noexcept
    {
        constexpr Int mask[] = { (Int)0, F };
        constexpr Int flow = (Int)0x80 << (sizeof(Int) - 1);
        Int acm = mask[!!(flow & v)];
        v <<= 1;
        return v ^ acm;
    }
    static constexpr Int power(Int v, unsigned c) noexcept
    {
        Int res{1};
        while (c-->0) {
            res = mul(res, v);
        }
        return res;
    }

    friend constexpr gf operator+(gf lhs, gf rhs) noexcept { return gf{ add(lhs, rhs) }; }
    friend constexpr gf operator-(gf lhs, gf rhs) noexcept { return gf{ add(lhs, rhs) }; }
    friend constexpr gf operator*(gf lhs, gf rhs) noexcept { return gf{ mul(lhs, rhs) }; }
};

using gf256 = gf<unsigned char, 0x1b>;

}
