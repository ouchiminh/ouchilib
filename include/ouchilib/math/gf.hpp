#pragma once
#include <type_traits>
#include <cstdint>
#include <limits>
#include <tuple>

namespace ouchi::math {

/// <summary>
/// 拡張ユークリッド互除法 
/// </summary>
/// <returns>d = ax + byとなるような{d, x, y}</returns>
template<class Int>
inline constexpr auto ex_gcd(Int a, Int b)
-> std::tuple<Int, Int, Int>
{
    if (b == 0) return std::make_tuple((Int)a, (Int)1, (Int)0);
    auto [d, x, y] = ex_gcd(b, a % b);
    y -= a / b * x;
    return std::make_tuple(d, x, y);
}

/// <summary>
/// F : Intで表現される拡大体の既約多項式のビット表現
/// </summary>
template<class Int, Int F, std::enable_if_t<std::is_integral_v<Int> && std::is_unsigned_v<Int>, int> = 0>
struct gf {
    Int value;
    gf() = default;
    constexpr gf(const gf&) = default;
    constexpr gf(gf&&) = default;
    explicit constexpr gf(Int v) : value{ v } {}

    gf& operator=(const gf&) = default;
    gf& operator=(gf&&) = default;

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
        constexpr Int flow = (Int)0x80 << ((sizeof(Int) - 1) * 8);
        Int acm = mask[!!(flow & v)];
        v <<= 1;
        return v ^ acm;
    }
    static constexpr Int power(Int v, size_t c) noexcept
    {
        return c == 0 ? 1
            : c & 1 ? mul(v, power(v, c - 1))
            : power(mul(v, v), c >> 1);
    }
    friend constexpr gf pow(gf v, size_t c) noexcept { return gf{ gf::power((Int)v, c) }; }
    static constexpr Int inv(Int v) noexcept
    {
        constexpr auto r = std::numeric_limits<Int>::max() - 1;
        return power(v, r);
    }
    constexpr gf inv() const noexcept { return gf::inv(value); }
    friend constexpr bool operator<(gf lhs, gf rhs) noexcept { return lhs.value < rhs.value; }
    friend constexpr bool operator>(gf lhs, gf rhs) noexcept { return rhs < lhs; }
    friend constexpr bool operator==(gf lhs, gf rhs) noexcept { return lhs.value == rhs.value; }
    friend constexpr bool operator==(gf lhs, Int rhs) noexcept { return lhs.value == rhs; }
    friend constexpr bool operator==(Int lhs, gf rhs) noexcept { return lhs == rhs.value; }
    friend constexpr bool operator!=(gf lhs, gf rhs) noexcept { return !(lhs == rhs); }
    friend constexpr bool operator!=(gf lhs, Int rhs) noexcept { return !(lhs == rhs); }
    friend constexpr bool operator!=(Int lhs, gf rhs) noexcept { return !(lhs == rhs); }
    friend constexpr bool operator<=(gf lhs, gf rhs) noexcept { return (lhs < rhs) || (lhs == rhs); }
    friend constexpr bool operator>=(gf lhs, gf rhs) noexcept { return (lhs > rhs) || (lhs == rhs); }

    friend constexpr gf operator+(gf lhs, gf rhs) noexcept { return gf{ gf::add((Int)lhs, (Int)rhs) }; }
    friend constexpr gf operator-(gf lhs, gf rhs) noexcept { return gf{ gf::add((Int)lhs, (Int)rhs) }; }
    friend constexpr gf operator*(gf lhs, gf rhs) noexcept { return gf{ gf::mul((Int)lhs, (Int)rhs) }; }
    friend constexpr gf operator/(gf lhs, gf rhs) noexcept { return gf{ gf::mul((Int)lhs, gf::inv((Int)rhs)) }; }
    constexpr gf& operator+=(gf rhs) noexcept { return *this = *this + rhs; }
    constexpr gf& operator-=(gf rhs) noexcept { return *this = *this - rhs; }
    constexpr gf& operator*=(gf rhs) noexcept { return *this = *this * rhs; }
    constexpr gf& operator/=(gf rhs) noexcept { return *this = *this / rhs; }
};

template<unsigned char F = 0b0110'0011>
using gf256 = gf<std::uint8_t, F>;
template<unsigned char F = 0b0110'0011>
using gf2_8 = gf<std::uint8_t, F>;

template<std::uint16_t F = 0b0001'0000'1000'0101>
using gf65536 = gf<std::uint16_t, F>;
template<std::uint16_t F = 0b0001'0000'1000'0101>
using gf2_16 = gf<std::uint16_t, F>;

template<std::uint32_t F = 0b0000'0000'0100'0000'0000'0000'0000'0111>
using gf4294967296 = gf<std::uint32_t, F>;
template<std::uint32_t F = 0b0000'0000'0100'0000'0000'0000'0000'0111>
using gf2_32 = gf<std::uint32_t, F>;
}
