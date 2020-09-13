#pragma once
#include <type_traits>
#include <compare>

namespace ouchi::math {
struct infinity {
    constexpr infinity()
        : sgn_{ 1 }
    {}
    constexpr infinity(int sgn)
        : sgn_{ sgn < 0 ? -1 : 1 }
    {}
    template<class M>
    friend constexpr auto operator<=>(const infinity& a, const M&) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<M>, std::strong_ordering>
    {
        return a.sgn_ < 0 ? std::strong_ordering::less : std::strong_ordering::greater;
    }
    template<class M>
    friend constexpr auto operator<=>(const M&, const infinity& a) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<M>, std::strong_ordering>
    {
        return a.sgn_ < 0 ? std::strong_ordering::greater : std::strong_ordering::less;
    }
    friend constexpr auto operator<=>(infinity a, infinity b) noexcept
    {
        return a.sgn_ == b.sgn_
            ? std::partial_ordering::unordered
            : (std::partial_ordering)(a.sgn_<=>b.sgn_);
    }
    infinity operator-()
    {
        return infinity{ -sgn_ };
    }
private:
    int sgn_;
};
inline constexpr infinity inf;
}

