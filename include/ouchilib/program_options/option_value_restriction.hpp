#pragma once
#include <string>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <variant>
#include "ouchilib/utl/meta-helper.hpp"

namespace ouchi::program_options {

// value restriction

/************ option value type ************/

namespace detail {
struct type_restriction {};
struct flag_value : type_restriction {
    using value_type = void;
};

template<class T>
struct single_value : type_restriction {
    using value_type = std::remove_reference_t<std::remove_cv_t<T>>;
};

template<class T>
struct multi_value : type_restriction {
    using value_type = std::vector<typename single_value<T>::value_type>;
};
} // namespace detail

inline const detail::flag_value flag;
template<class T>
inline const detail::multi_value<T> multi;
template<class T>
inline const detail::single_value<T> single;

using default_option_value_type = detail::flag_value;

/************ default option value ************/

namespace detail {

template<class T>
struct default_value : default_value<void> {
    using value_type = std::remove_reference_t<std::remove_cv_t<T>>;
    value_type value;
    constexpr default_value(value_type&& v)
        : value(std::move(v))
    {}
    constexpr default_value(const value_type& v)
        : value(v)
    {}
};
template<>
struct default_value<void> {
    template<class U>
    constexpr default_value<U> operator=(U&& value) const noexcept
    {
        return default_value<U>{std::forward<U>(value)};
    }
};
} // namespace detail

constexpr detail::default_value<void> default_value;

// value restriction policy

namespace detail {

template<class B, class Head, class ...D>
inline std::variant<variant_compatible_t<Head>, variant_compatible_t<D>...>
find_derived_value_impl(Head&& h, D&& ...d) 
{
    if constexpr (std::is_base_of_v<B, Head>) return h;
    return find_default_value_impl(std::forward<D>(d)...);
}

template<class B>
inline std::variant<std::monostate> find_derived_value_impl() { return ::std::monostate{}; }

template<class B, class ...D>
inline auto find_derived_value(D&& ...derived) -> ::ouchi::variant_compatible_t<::ouchi::find_derived_t<B, D...>>
{
    return std::get<::ouchi::variant_compatible_t<::ouchi::find_derived_t<B, D...>>>(find_default_value_impl(std::forward<D>(derived)...));
}

template<class ...D>
inline auto find_default_value(D&& ...values) -> ::ouchi::variant_compatible_t<::ouchi::find_derived_t<default_value<void>, D...>>
{
    return find_derived_value<default_value<void>>(std::forward<D>(values)...);
}

template<class ...Args>
struct value_type_policy {
    using type = std::conditional_t<
        std::is_same_v<::ouchi::find_derived_t<type_restriction, Args...>, void>,
        flag_value,
        ::ouchi::find_derived_t<type_restriction, Args...>
    >;
};

template<class ...Args>
using value_type_policy_t = typename value_type_policy<Args...>::type;

static_assert(std::is_same_v<detail::single_value<int>,
              value_type_policy_t<int, detail::single_value<int>>>);
}

}
