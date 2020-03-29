#pragma once
#include <utility>
#include <type_traits>
#include <concepts>
#include <string_view>
#include <ratio>
#include <compare>

#include "dimension.hpp"
#include "detail.hpp"

namespace ouchi::units {

template<class T, class Unit>
class quantity;

template<class ...>
struct basic_units;

/// <summary>
/// 単位を表す型です。
/// </summary>
/// <remarks>
/// 単位系の各次元はその次元の指数及びSI接頭辞に相当する倍数の情報を保持します。
/// ある次元の量を二乗するとSI接頭辞に相当する倍数の情報も二乗されます。
/// 例えば km + km = km^2ですが、km^2の単位はこの型ではbasic_units<basic_dimension<length, 2, std::ratio<1'000'000>>...>と表されます。
/// </remarks>
template<class ...Tags, int ...Exs, class ...Ratio>
struct basic_units<basic_dimension<Tags, Exs, Ratio>...> {
    using unit_type = basic_units;

    template<class Unit>
    friend constexpr auto operator*(basic_units, Unit) noexcept
        -> typename detail::mul_unit<basic_units, Unit>::type
    { return {}; }

    template<class Unit>
    friend constexpr auto operator/(basic_units, Unit) noexcept
        -> typename detail::div_unit<basic_units, Unit>::type
    { return {}; }
    template<class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    friend constexpr auto operator|(T&& val, basic_units)
        -> quantity<T, basic_units>
    {
        return quantity<T, basic_units>{val};
    }
    template<class Unit, std::enable_if_t<detail::is_convertible_unit<Unit, basic_units>::value, int> = 0>
    static constexpr auto ratio(Unit = Unit{}) noexcept
    {
        using diff_t = typename detail::div_unit<Unit, basic_units>::type;
        constexpr auto num = (detail::find_dim_from_unit<Tags, diff_t>::type::ratio::num * ... * 1);
        constexpr auto den = (detail::find_dim_from_unit<Tags, diff_t>::type::ratio::den * ... * 1);
        return typename std::ratio<num, den>::type{};
    }
    template<class Unit, std::enable_if_t<detail::is_convertible_unit<Unit, basic_units>::value, int> = 0>
    friend constexpr bool operator<(basic_units, Unit) noexcept
    {
        using r = decltype(ratio<Unit>());
        return r::num > r::den;
    }
    template<class Unit, std::enable_if_t<detail::is_convertible_unit<Unit, basic_units>::value, int> = 0>
    friend constexpr bool operator>(basic_units r, Unit l) noexcept { return l < r; }

    template<class Unit, std::enable_if_t<detail::is_convertible_unit<Unit, basic_units>::value, int> = 0>
    inline static constexpr bool lt = decltype(ratio<Unit>())::num > decltype(ratio<Unit>())::den;
};

/// <summary>
/// 量を表す型です。
/// </summary>
template<class T, class ...Tags, int ...Exs, class ...Ratio>
class quantity<T, basic_units<basic_dimension<Tags, Exs, Ratio>...>> {
private:
    template<class U>
    inline static constexpr bool is_operator_noexcept =
        noexcept(std::declval<T>() * std::declval<U>()) &&
        noexcept(std::declval<T>() / std::declval<U>()) &&
        noexcept(std::declval<T>() - std::declval<U>()) &&
        noexcept(std::declval<T>() + std::declval<U>()) &&
        std::is_nothrow_copy_constructible_v<std::common_type_t<T, U>>;

public:
    using unit_type = basic_units<basic_dimension<Tags, Exs, Ratio>...>;
    using value_type = std::remove_cvref_t<T>;

public:
    constexpr quantity() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
    constexpr quantity(const quantity&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
    constexpr quantity(quantity&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
    explicit constexpr quantity(T val) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_{ val }
    {}
    /// <summary>
    /// 同じ次元の量を<see cref="unit_type"/>に変換して初期化します。
    /// </summary>
    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit_v<unit_type, Unit>, int> = 0>
    constexpr quantity(quantity<U, Unit> q) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_{ q.convert<unit_type, T>().get_value() }
    {}

    /// <summary>
    /// 同じ次元の量を<see cref="unit_type"/>に変換して初期化します。
    /// </summary>
    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit_v<unit_type, Unit>, int> = 0>
    quantity& operator=(const quantity<U, Unit>& q)
    {
        value_ = q.convert<unit_type, T>().get_value();
        return *this;
    }
    /// <summary>
    /// 量の数を返します。
    /// </summary>
    [[nodiscard]]
    const T& get_value() const noexcept { return value_; }

    /// <summary>
    /// qの符号を反転します。
    /// </summary>
    [[nodiscard]]
    friend constexpr quantity operator-(const quantity& q) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        return quantity{ -q.get_value() };
    }

    /// <summary>
    /// 同じ次元の指定された量に変換します。この変換関数では、例えば kg から g への変換など次元が同じ量にしか適用できません。
    /// </summary>
    template<class Unit, class U = T, std::enable_if_t<detail::is_convertible_unit_v<unit_type, Unit>, int> = 0>
    [[nodiscard]]
    constexpr auto convert(Unit = Unit{}) const
        -> quantity<std::common_type_t<U, T>, Unit>
    {
        using r = typename decltype(unit_type::template ratio<Unit>())::type;
        using ct = std::common_type_t<U, T>;
        return quantity<std::common_type_t<U, T>, Unit>{ static_cast<ct>(value_) * r::den / r::num };
    }
    template<class U, class Unit>
    [[nodiscard]]
    friend constexpr auto operator*(const quantity& lhs,
                                    const quantity<U, Unit>& rhs)
        noexcept(is_operator_noexcept<U>)
        -> quantity<std::common_type_t<T, U>, typename detail::mul_unit<unit_type, Unit>::type>
    {
        using ct = std::common_type_t<T, U>;
        return quantity<std::common_type_t<T, U>, typename detail::mul_unit<unit_type, Unit>::type>
        {static_cast<ct>(lhs.get_value()) * static_cast<ct>(rhs.get_value())};
    }
    template<class U, class Unit>
    [[nodiscard]]
    friend constexpr auto operator/(const quantity& lhs,
                                    const quantity<U, Unit>& rhs)
        noexcept(is_operator_noexcept<U>)
        -> quantity<std::common_type_t<T, U>, typename detail::div_unit<unit_type, Unit>::type>
    {
        using ct = std::common_type_t<T, U>;
        return quantity<std::common_type_t<T, U>, typename detail::div_unit<unit_type, Unit>::type>
        {static_cast<ct>(lhs.get_value()) / static_cast<ct>(rhs.get_value())};
    }
    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit<unit_type, Unit>::value, int> = 0>
    [[nodiscard]]
    friend constexpr auto operator+(const quantity& lhs,
                                    const quantity<U, Unit>& rhs)
        noexcept(is_operator_noexcept<U>)
        ->quantity<std::common_type_t<T, U>, std::conditional_t<unit_type:: template lt<Unit>, unit_type, Unit>>
    {
        using ut = std::conditional_t<unit_type:: template lt<Unit>, unit_type, Unit>;
        using ct = std::common_type_t<T, U>;

        return quantity<ct, ut>{lhs.convert<ut, ct>().get_value() + rhs.convert<ut, ct>().get_value()};
    }
    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit<unit_type, Unit>::value, int> = 0>
    [[nodiscard]]
    friend constexpr auto operator-(const quantity& lhs,
                                    const quantity<U, Unit>& rhs)
        noexcept(is_operator_noexcept<U>)
        ->quantity<std::common_type_t<T, U>, std::conditional_t<unit_type:: template lt<Unit>, unit_type, Unit>>
    {
        return lhs + (-rhs);
    }

    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit<unit_type, Unit>::value, int> = 0>
    constexpr auto operator<=>(const quantity<U, Unit>& rhs)
    {
        using ut = std::conditional_t<(unit_type{} < Unit{}), unit_type, Unit>;
        using ct = std::common_type_t<T, U>;
        return this->convert<ut, ct>().get_value() <=> rhs.convert<ut, ct>().get_value();
    }
    template<class U, class Unit, std::enable_if_t<detail::is_convertible_unit<unit_type, Unit>::value, int> = 0>
    constexpr bool operator==(const quantity<U, Unit>& rhs)
    {
        return std::strong_ordering::equal == ((*this) <=> rhs);
    }
private:
    value_type value_;
};

/// <summary>
/// 単位系の各次元を表す任意の型から単位系を作ります。
/// </summary>
/// <example>
/// ユーザー定義の単位系を作る例を示します。SIは7次元ですが、ここでは3次元の単位系を作ります。
/// 単位系を作るための最初のステップとして単位系の各次元を表す型を作ります。これはタグとして利用され、任意の型がタグになることができます。
/// <code>
/// struct l{}; // 長さ
/// struct m{}; // 質量
/// struct t{}; // 時間
/// using system_t = system_of_units<l, m, t>; // 単位系
/// </code>
/// </example>
template<class ...DimensionTags>
struct system_of_units {
    /// <summary>
    /// 指定された次元, 指数, プレフィクスをもつ単位の型です。
    /// </summary>
    template<class D, int E = 1, class Ratio = std::ratio<1, 1>>
    using unit_t = std::enable_if_t<
        detail::belongs_v<D, std::tuple<DimensionTags...>>,
        typename detail::power<
        basic_units<basic_dimension<
            DimensionTags,
            std::is_same_v<D, DimensionTags> ? 1 : 0,
            std::conditional_t<std::is_same_v<D, DimensionTags>, Ratio, std::ratio<1, 1>>
        >...>, E>::type
    >;
    /// <summary>
    /// <see cref="unit_t"/>の値です。
    /// </summary>
    template<class D, int E = 1, class Ratio = std::ratio<1, 1>>
    static constexpr auto make_unit() noexcept
        -> unit_t<D, E, Ratio>
    { return {}; }
    /// <summary>
    /// この単位系における無次元の単位です。
    /// </summary>
    using dimensionless_t = basic_units<basic_dimension<DimensionTags, 0, std::ratio<1, 1>>...>;
    /// <summary>
    /// <see cref="dimensionless_t"/>の値です。
    /// </summary>
    static constexpr auto dimensionless() noexcept
        -> dimensionless_t
    { return {}; }
};

}

