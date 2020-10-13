#include <type_traits>
#include <numeric>
#include <limits>
#include <concepts>
#include <cassert>
#include "gf.hpp"

namespace ouchi::math {

namespace detail {
template<class T>
concept integral = std::is_integral_v<T>;
}

template<detail::integral Int, detail::integral Internal = Int>
class modint {
public:
    static constexpr bool nothrow_mod = noexcept(std::declval<Int>() % std::declval<Int>());
    static constexpr bool nothrow_add = nothrow_mod && noexcept(std::declval<Int>() + std::declval<Int>());
    static constexpr bool nothrow_mul = nothrow_mod && noexcept(std::declval<Internal>() * std::declval<Int>());
    constexpr modint(const Int& value, const Int& mod)
    {
        if (value % mod < 0) value_ = value % mod + mod;
        else value_ = value % mod;
        mod_ = mod;
    }
    constexpr modint()
        : value_{}
        , mod_{std::numeric_limits<Int>::max()}
    {}

    constexpr operator const Int&() const noexcept(noexcept(std::is_nothrow_copy_constructible_v<Int>)) { return value_; }

    constexpr modint operator-() const noexcept(std::is_copy_constructible_v<Int>)
    {
        return modint(0, mod_) - *this;
    }
    modint& operator+=(const Int& i) & noexcept(nothrow_add)
    {
        value_ = (value_ + i) % mod_;
        return *this;
    }
    modint& operator-=(const Int& i) & noexcept(nothrow_add)
    {
        value_ = (value_ - i) % mod_;
        if (value_ < 0) value_ += mod_;
        return *this;
    }
    modint& operator*=(const Int& i) & noexcept(nothrow_mul)
    {
        const Internal buf = value_;
        value_ = (const Int&)(buf * i % mod_);
        return *this;
    }
    modint& operator/=(const Int& i) & noexcept(nothrow_mul)
    {
        auto [d, x, y] = ex_gcd(i, mod_);
        if (x < 0) x += mod_;
        return *this *= x;
    }
    friend modint operator+(const modint& a, const modint& b)
    {
        assert(a.mod_ == b.mod_);
        auto ret = a;
        return ret += b;
    }
    friend modint operator-(const modint& a, const modint& b)
    {
        assert(a.mod_ == b.mod_);
        auto ret = a;
        return ret -= b;
    }
    friend modint operator*(const modint& a, const modint& b)
    {
        assert(a.mod_ == b.mod_);
        auto ret = a;
        return ret *= b;
    }
    friend modint operator/(const modint& a, const modint& b)
    {
        assert(a.mod_ == b.mod_);
        auto ret = a;
        return ret /= b;
    }
private:
    Int value_;
    Int mod_;
};

}

