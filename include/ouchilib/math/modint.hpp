#pragma once
#include <type_traits>
#include <numeric>
#include <limits>
#include <concepts>
#include <cassert>
#include "gf.hpp"

namespace ouchi::math {

template<class Int, class Internal = Int>
class modint {
public:
    static constexpr bool nothrow_copy = std::is_nothrow_assignable_v<Int, Int> && std::is_nothrow_assignable_v<Int, Internal> && std::is_nothrow_copy_assignable_v<Int> && std::is_nothrow_copy_constructible_v<Int>;
    static constexpr bool nothrow_mod = noexcept(std::declval<Int>() % std::declval<Int>());
    static constexpr bool nothrow_add = nothrow_copy && nothrow_mod && noexcept(std::declval<Int>() + std::declval<Int>());
    static constexpr bool nothrow_mul = nothrow_copy && nothrow_mod && noexcept(std::declval<Internal>() * std::declval<Int>());
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
    constexpr const Int& mod() const noexcept { return mod_; }
    constexpr Int& mod() noexcept { return mod_; }

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
    auto operator<=>(const modint& i) const
    {
        return this->value_ <=> i;
    }
    bool operator==(const modint& i) const
    {
        return this->value_ == i;
    }
private:
    Int value_;
    Int mod_;
};

template<class T, class Int>
inline modint<T> pow(const modint<T>& a, const Int e)
{
    using std::abs;
    if (e == 0) return modint<T>(1, a.mod());
    auto c = modint<T>(a, a.mod());
    auto ec = abs(e);
    while (ec) {
        if (ec & 1) {
            c *= a;
        } else {
            c *= c;
            ec /= 2;
        }
        --ec;
    }
    return e < 0 ? modint<T>(1, a.mod()) / c : c;
}

}

