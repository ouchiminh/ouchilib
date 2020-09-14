#pragma once
#include <type_traits>
#include <concepts>
#include <tuple>
#include <utility>
#include "step.hpp"

namespace ouchi {

template<additive T>
struct interval_divide {
    using additive_t = std::remove_reference_t<T>;

    interval_divide() = delete;
    template<class U = T, std::enable_if_t<std::is_same_v<T, U> && std::is_move_constructible_v<U>, int> = 0>
    constexpr interval_divide(U&& begin, U&& end)
        : begin_{ std::forward<T>(begin) }
        , end_{ std::forward<T>(end) }
    {}

    template<class ...Arg1, class ...Arg2>
    constexpr interval_divide(std::piecewise_construct_t, std::tuple<Arg1...> begin, std::tuple<Arg2...> end)
        : interval_divide(begin, end,
                          std::make_index_sequence<sizeof...(Arg1)>(),
                          std::make_index_sequence<sizeof...(Arg2)>())
    {}

    template<class U = T, std::enable_if_t<std::is_same_v<T, U> && std::is_copy_constructible_v<U>, int> = 0>
    constexpr interval_divide(const interval_divide<U>& id)
        : begin_{id.begin_}
        , end_{id.end_}
    {}

    template<class U = T, std::enable_if_t<std::is_same_v<T, U> && std::is_move_constructible_v<U>, int> = 0>
    constexpr interval_divide(interval_divide<U>&& id)
        : begin_{std::move(id.begin_)}
        , end_{std::move(id.end_)}
    {}

    constexpr std::pair<additive_t, additive_t> divide(size_t current, size_t total)
    {
        size_t interval_count = end_ - begin_;
        size_t r = interval_count % total;
        size_t b = interval_count / total;
        auto begin = begin_ + current * b + std::min(current, r);
        return { begin, begin + b + (current < r ? 1 : 0) };
    }
    constexpr step<additive_t> divide_into_steps(size_t current, size_t total)
    {
        auto [begin, end] = divide(current, total);
        return step(begin, end);
    }
private:
    template<class ...Arg1, class ...Arg2, size_t ...I1, size_t ...I2>
    constexpr interval_divide(std::tuple<Arg1...>& t1, std::tuple<Arg2...>& t2, std::index_sequence<I1...>, std::index_sequence<I2...>)
        : begin_(std::get<I1>(t1) ...)
        , end_(std::get<I2>(t2) ...)
    {}

    additive_t begin_, end_;
};

template<class T>
interval_divide(T&&, T&&)->interval_divide<std::remove_reference_t<T>>;
template<class T>
interval_divide(const T&, const T&)->interval_divide<std::remove_reference_t<T>>;

}

