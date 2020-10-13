#pragma once
#include <algorithm>
#include <type_traits>

namespace ouchi {

template<class T>
concept additive = requires(T t)
{
    t++;
    ++t;
    --t;
    t--;
    t + std::declval<size_t>();
    std::declval<size_t>() + t;
};


template<additive T>
class step {
    T begin_;
    T end_;
    size_t width_;
public:
    constexpr step() = default;
    constexpr step(T begin, T end, size_t width = 1)
        : begin_{ begin }
        , end_{ end }
        , width_ { width }
    {}
    template<class U = T, std::enable_if_t<std::is_same_v<T, U> && std::is_integral_v<U>, int> = 0>
    constexpr step(T end)
        : step(static_cast<T>(0),
               end,
               1)
    {}

    class iterator{
        T cur_;
        T end_;
        size_t width_;
    public:
        using difference_type = T;
        using value_type = T;
        using iterator_category = std::input_iterator_tag;

        constexpr iterator(T cur, T end, size_t width)
            : cur_{ cur }
            , end_{ end }
            , width_{ width }
        {}

        constexpr iterator& operator++() {
            cur_ = std::min(static_cast<T>(cur_ + width_), end_);
            return *this;
        }
        constexpr iterator operator++(int) { auto cp = *this; ++(*this); return cp; }
        constexpr T operator*() const noexcept { return cur_; }
        constexpr friend bool operator==(const iterator& a, const iterator& b) {
            return a.cur_ == b.cur_ && a.end_ == b.end_;
        }
        constexpr friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }
    };

    constexpr iterator begin() const noexcept { return iterator{begin_, end_, width_}; }
    constexpr iterator end() const noexcept { return iterator{end_, end_, width_}; }
    constexpr size_t size() const noexcept { return (end_ - begin_) / width_; }
};

}

