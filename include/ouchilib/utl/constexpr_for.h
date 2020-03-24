#pragma once
#include <concepts>
#include <type_traits>
#include <cstddef>

namespace ouchi {

namespace detail {

template<std::invocable<size_t> F, size_t ...Ints>
constexpr auto constexpr_for_impl(F&& f, std::index_sequence<Ints...>)
{
    return (f(Ints), ...);
}
template<std::invocable<> F, size_t ...Ints>
constexpr auto constexpr_for_impl(F&& f, std::index_sequence<Ints...>)
{
    return ((Ints, f()), ...);
}

}

template<size_t N, class F>
constexpr auto constexpr_for(F&& f)
{
    return detail::constexpr_for_impl(std::forward<F>(f), std::make_index_sequence<N>());
}

}

