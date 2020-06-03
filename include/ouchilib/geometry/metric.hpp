#pragma once
#include <cstdint>
#include <map>
#include <type_traits>
#include <compare>
#include <cassert>
namespace ouchi::geometry {
template<class Int, class = void>
class metric;

template<class Int>
class metric<Int, std::enable_if_t<std::is_integral_v<Int>>> {
public:
    metric() = default;
    metric(Int value, std::uint64_t i)
        : polynomial_{std::make_pair(i, (Int)1), std::make_pair(0, value)}
    {
        assert(i != 0);
    }
    metric(Int value)
        : polynomial_{std::make_pair(0, value)}
    {}
    metric(const metric&) = default;
    metric(metric&&) = default;
    metric& operator=(const metric&) = default;
    metric& operator=(metric&&) = default;
    metric& operator=(const Int& value)
    {
        polynomial_.clear();
        polynomial_.insert_or_assign(0, value);
    }

    void assign(Int v, std::uint64_t i)
    {
        *this = metric{ v, i };
    }

    metric operator-() const
    {
        auto cp = *this;
        for (auto& [key, value] : cp.polynomial_) {
            value = -value;
        }
        return std::move(cp);
    }

    friend metric operator+(const metric& a,
                            const metric& b)
    {
        auto cp = a;
        return cp += b;
    }
    friend metric operator-(const metric& a,
                            const metric& b)
    {
        return a + (-b);
    }
    friend metric operator*(const metric& a,
                            const metric& b)
    {
        auto cp = a;
        return cp *= b;
    }
    metric& operator+=(const metric& a)
    {
        for (auto [ex, co] : a.polynomial_) polynomial_[ex] += co;
        return *this;
    }
    metric& operator-=(const metric& a)
    {
        return *this += (-a);
    }
    metric& operator*=(const metric& a)
    {
        std::map<std::uint64_t, Int> poly;
        for (auto [ex1, co1] : polynomial_) {
            for (auto [ex2, co2] : a.polynomial_) {
                poly[ex1+ex2] = co1*co2;
            }
        }
        polynomial_ = std::move(poly);
        return *this;
    }
    friend std::strong_ordering operator<=>(const metric& a,
                                            const metric& b)
    {
        auto s = a - b;
        for (auto [ex, co] : s.polynomial_) {
            if (co != 0) return co <=> (Int)0;
        }
        return std::strong_ordering::equal;
    }
    friend bool operator== (const metric& a,
                            const metric& b)
    {
        return (a <=> b) == std::strong_ordering::equal;
    }
    explicit operator Int() const
    {
        return polynomial_.count(0) ? polynomial_.at(0) : 0;
    }
    void reduce() noexcept
    {
        std::erase_if(polynomial_, [](const auto& kvpair) { return kvpair.second == 0; });
    }
private:
    //       指数            係数
    std::map<std::uint64_t, Int> polynomial_;
};

}

