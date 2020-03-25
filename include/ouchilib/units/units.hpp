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

template<detail::arithmetic T, class SystemOfUnits>
class quantity;

template<class SystemOfUnits, int ...Exponents>
struct basic_system_of_units{
    template<int ...Is>
    static auto seq_to(std::integer_sequence<int, Is...>)
        ->basic_system_of_units<SystemOfUnits, Is...>;
    template<int ...Es>
    using mul_t = decltype(seq_to(std::declval<typename detail::add_sequence<std::integer_sequence<int, Exponents...>, std::integer_sequence<int, Es...>>::result>()));
    template<int ...Es>
    using div_t = decltype(seq_to(std::declval<typename detail::sub_sequence<std::integer_sequence<int, Exponents...>, std::integer_sequence<int, Es...>>::result>()));
    using inv_t = basic_system_of_units<SystemOfUnits, (-Exponents)...>;

    template<int ...Es>
    friend auto operator*(basic_system_of_units, basic_system_of_units<SystemOfUnits, Es...>)
        -> mul_t<Es...>
    { return {}; }
    template<int ...Es>
    friend auto operator/(basic_system_of_units, basic_system_of_units<SystemOfUnits, Es...>)
        -> div_t<Es...>
    { return {}; }
    template<detail::arithmetic T>
    friend quantity<T, basic_system_of_units> operator*(T value, basic_system_of_units) noexcept
    {
        return quantity<T, basic_system_of_units>{ value };
    }
};

template<detail::arithmetic T, class SystemOfUnits, int ...Exponents>
class quantity<T, basic_system_of_units<SystemOfUnits, Exponents...>> {
private:
    T value_;
public:
    using units = basic_system_of_units<SystemOfUnits, Exponents...>;

    quantity() noexcept
        : value_{}
    {}
    explicit quantity(T value) noexcept
        :value_{ value }
    {}
    T get_value() const noexcept { return value_; }

    template<detail::arithmetic U>
    friend auto operator+(const quantity& rhs, const quantity<U, units>& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, units>
    {
        using result_t = std::common_type_t<T, U>;
        return quantity<result_t, units>{ static_cast<result_t>(rhs.get_value()) + static_cast<result_t>(lhs.get_value()) };
    }
    template<detail::arithmetic U>
    friend auto operator-(const quantity& rhs, const quantity<U, units>& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, units>
    {
        using result_t = std::common_type_t<T, U>;
        return quantity<result_t, units>{ static_cast<result_t>(rhs.get_value()) - static_cast<result_t>(lhs.get_value()) };
    }

    template<detail::arithmetic U, int ...Es>
    friend auto operator*(const quantity& rhs,
                          const quantity<U, basic_system_of_units<SystemOfUnits, Es...>>& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, typename units:: template mul_t<Es...>>
    {
        using nt = std::common_type_t<T, U>;
        using qt = quantity<std::common_type_t<T, U>, typename units:: template mul_t<Es...>>;
        return qt{ static_cast<nt>(rhs.get_value()) * static_cast<nt>(lhs.get_value()) };
    }
    template<detail::arithmetic U, int ...Es>
    friend auto operator/(const quantity& rhs,
                          const quantity<U, basic_system_of_units<SystemOfUnits, Es...>>& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, typename units:: template div_t<Es...>>
    {
        using nt = std::common_type_t<T, U>;
        using qt = quantity<std::common_type_t<T, U>, typename units:: template div_t<Es...>>;
        return qt{ static_cast<nt>(rhs.get_value()) / static_cast<nt>(lhs.get_value()) };
    }

    template<detail::arithmetic U>
    friend auto operator*(U&& rhs, const quantity& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, units>
    {
        using result_t = std::common_type_t<T, U>;
        return quantity<result_t, units>{ static_cast<result_t>(rhs) * static_cast<result_t>(lhs.get_value()) };
    }
    template<detail::arithmetic U>
    friend auto operator*(const quantity& rhs, U&& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, units>
    {
        return lhs * rhs;
    }
    template<detail::arithmetic U>
    friend auto operator/(U&& rhs, const quantity& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, typename units::inv_t>
    {
        using result_t = std::common_type_t<T, U>;
        return quantity<result_t, typename units::inv_t>{ static_cast<result_t>(rhs) / static_cast<result_t>(lhs.get_value()) };
    }
    template<detail::arithmetic U>
    friend auto operator/(const quantity& rhs, U&& lhs) noexcept
        -> quantity<std::common_type_t<T, U>, units>
    {
        using nt = std::common_type_t<T, U>;
        return quantity<nt, units>{static_cast<nt>(rhs.get_value()) / static_cast<nt>(lhs)};
    }
};

}

