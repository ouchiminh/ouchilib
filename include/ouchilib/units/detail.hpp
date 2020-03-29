#pragma once
#include <utility>
#include <type_traits>

#include "dimension.hpp"

namespace ouchi::units::detail {

template<class E, class S>
struct belongs;

template<class E, template<class...>class S, class ...Es>
struct belongs<E, S<Es...>>
    : std::bool_constant<std::disjunction_v<std::is_same<E, Es>...>>
{};

template<class E, class S>
inline constexpr bool belongs_v = belongs<E, S>::value;

template<class Subset, class Superset>
struct includes;

template<template<class ...>class S, class ...T, class ...U>
struct includes<S<T...>, S<U...>>
    : std::bool_constant<std::conjunction_v<belongs<T, S<U...>>...>>
{};

template<class S1, class S2>
inline constexpr bool includes_v = includes<S1, S2>::value;

template<class, class>
struct set_equal;

template<template<class ...> class S, class ...T, class ...U>
struct set_equal<S<T...>, S<U...>>
    : std::bool_constant<std::conjunction_v<includes<S<T...>, S<U...>>, includes<S<U...>, S<T...>>>>
{};

template<class S1, class S2>
inline constexpr bool set_equal_v = set_equal<S1, S2>::value;

template<class Tag, class ...Dimensions>
struct find_dim{
    using type = void;
};

template<class Tag, class Dimension, class ...Dimensions>
struct find_dim<Tag, Dimension, Dimensions...> {
    using type = std::conditional_t<
        std::is_same_v<Tag, typename Dimension::dimension>,
        Dimension, typename find_dim<Tag, Dimensions...>::type
    >;
};

template<class Tag, class Unit>
struct find_dim_from_unit;
template<class Tag, class ...Dimensions, template<class ...> class U>
struct find_dim_from_unit<Tag, U<Dimensions...>> {
    using type = typename find_dim<Tag, Dimensions...>::type;
};

template<class Tag, class ...D>
using find_dim_t = typename find_dim<Tag, D...>::type;

template<class S1, class S2, class = void>
struct mul_unit;

template<template<class ...> class S,
    class ...T1, class ...T2,
    int ...E1, int ...E2,
    class ...R1, class ...R2
>
struct mul_unit<
    S<basic_dimension<T1, E1, R1>...>, S<basic_dimension<T2, E2, R2>...>,
    std::enable_if_t<set_equal_v<std::tuple<T1...>, std::tuple<T2...>>>
>
{
    using type = S<
        basic_dimension<
            T1,
            E1 + find_dim_t<T1, basic_dimension<T2, E2, R2>...>::exponent,
            typename std::ratio<R1::num * find_dim_t<T1, basic_dimension<T2, E2, R2>...>::ratio::num, R1::den * basic_dimension<T2, E2, R2>::ratio::den>::type
        >...
    >;
};

template<class S1, class S2, class = void>
struct div_unit;

template<template<class ...> class S,
    class ...T1, class ...T2,
    int ...E1, int ...E2,
    class ...R1, class ...R2
>
struct div_unit<
    S<basic_dimension<T1, E1, R1>...>, S<basic_dimension<T2, E2, R2>...>,
    std::enable_if_t<set_equal_v<std::tuple<T1...>, std::tuple<T2...>>>
>
{
    using type = S<
        basic_dimension<
            T1,
            E1 - find_dim_t<T1, basic_dimension<T2, E2, R2>...>::exponent,
            typename std::ratio<R1::num * find_dim_t<T1, basic_dimension<T2, E2, R2>...>::ratio::den, R1::den * basic_dimension<T2, E2, R2>::ratio::num>::type
        >...
    >;
};

template<bool ...V>
struct logical_and : std::true_type{};

template<bool V1, bool ...V>
struct logical_and<V1, V...> : std::bool_constant<V1 && logical_and<V...>::value> {};

template<class S1, class S2, class = void>
struct is_convertible_unit : std::false_type {};

template<template<class ...> class S,
    class ...T1, class ...T2,
    int ...E1, int ...E2,
    class ...R1, class ...R2
>
struct is_convertible_unit<S<basic_dimension<T1, E1, R1>...>, S<basic_dimension<T2, E2, R2>...>, std::enable_if_t<set_equal_v<std::tuple<T1...>, std::tuple<T2...>>>>
{
    inline static constexpr bool value = std::conjunction_v<
        std::bool_constant<(E1 == find_dim_t<T1, basic_dimension<T2, E2, R2>...>::exponent)>...>;
};
template<class S1, class S2>
inline static constexpr bool is_convertible_unit_v = is_convertible_unit<S1, S2>::value;

template<class T, T Val>
struct abs {
    inline static constexpr T value = Val < 0 ? -Val : Val;
};

template<class Unit, int N, class = void>
struct power;

template<class ...Tags, int ...E, class ...R, template<class ...> class U>
struct power<U<basic_dimension<Tags, E, R>...>, 0> {
    using type = U<basic_dimension<Tags, 0>...>;
};

template<class ...Tags, int ...E, class ...R, template<class ...> class U, int N>
struct power<U<basic_dimension<Tags, E, R>...>, N, std::enable_if_t<(N > 0)>> {
    using type = typename mul_unit<typename power<U<basic_dimension<Tags, E, R>...>, N - 1>::type, U<basic_dimension<Tags, E, R>...>>::type;
};

template<class ...Tags, int ...E, class ...R, template<class ...> class U, int N>
struct power<U<basic_dimension<Tags, E, R>...>, N, std::enable_if_t<(N < 0)>> {
    using type = typename div_unit<U<basic_dimension<Tags, 0>...>, typename power<U<basic_dimension<Tags, E, R>...>, abs<int, N>::value>::type>::type;
};
}

