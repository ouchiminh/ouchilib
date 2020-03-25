#pragma once
#include <utility>
#include <type_traits>
#include <concepts>
#include <string_view>

namespace ouchi::units {

namespace detail {

template<class, class>
class add_sequence;

template<class Int, Int ...I, Int ...J>
struct add_sequence<
    std::integer_sequence<Int, I...>,
    std::integer_sequence<Int, J...>>
{
    using result = std::enable_if_t<
        sizeof...(I) == sizeof...(J),
        std::integer_sequence<Int, (I + J)...>>;
};


template<class, class>
class sub_sequence;

template<class Int, Int ...I, Int ...J>
struct sub_sequence<
    std::integer_sequence<Int, I...>,
    std::integer_sequence<Int, J...>>
{
    using result = std::enable_if_t<
        sizeof...(I) == sizeof...(J),
        std::integer_sequence<Int, (I - J)...>>;
};

template<class T>
concept arithmetic = std::is_arithmetic_v<T>;
}

template<class SystemOfUnits, int ...Exponents>
struct system_of_units_base{
    template<int ...Es>
    auto mul(system_of_units_base<SystemOfUnits, Es...>)
        ->typename detail::add_sequence<std::integer_sequence<int, Exponents...>, std::integer_sequence<int, Es...>>;
    template<int ...Es>
    auto div(system_of_units_base<SystemOfUnits, Es...>)
        ->typename detail::sub_sequence<std::integer_sequence<int, Exponents...>, std::integer_sequence<int, Es...>>;
};

template<detail::arithmetic T, class SystemOfUnits>
struct quantity{};

}

