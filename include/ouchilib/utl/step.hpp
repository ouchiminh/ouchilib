#pragma once
#include <algorithm>
#include <type_traits>

namespace ouchi {

template<class Int, std::enable_if_t<std::is_integral_v<Int>>* = nullptr>
class step {
    Int begin_;
    Int end_;
    Int width_;
public:
    step() = default;
    step(Int begin, Int end, Int width = static_cast<Int>(1))
        : begin_{ begin }
        , end_{ end }
        , width_ { width }
    {}
    step(Int end)
        : step(static_cast<Int>(0),
               end,
               static_cast<Int>(1))
    {}

    class iterator{
        Int cur_;
        Int end_;
        Int width_;
    public:
        iterator(Int cur, Int end, Int width)
            : cur_{ cur }
            , end_{ end }
            , width_{ width }
        {}

        iterator& operator++() {
            cur_ = std::min(cur_ + width_, end_);
            return *this;
        }
        iterator operator++(int) { auto cp = *this; ++(*this); return cp; }
        Int operator*() const noexcept { return cur_; }
        friend bool operator==(const iterator& a, const iterator& b) {
            return a.cur_ == b.cur_ && a.end_ == b.end_;
        }
        friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }
    };

    iterator begin() const noexcept { return iterator{begin_, end_, width_}; }
    iterator end() const noexcept { return iterator{end_, end_, width_}; }
};

}

