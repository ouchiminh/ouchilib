﻿#pragma once

#include <type_traits>
#include <array>
#include <vector>
#include <cstddef>
#include <cassert>

namespace ouchi::math {

inline namespace matrix_size_specifier {
namespace detail {
struct size_base {};

} // namespace detail
template<size_t Row, size_t Column>
struct fixed_length : detail::size_base {
    static constexpr size_t row_size = Row;
    static constexpr size_t column_size = Column;
    template<class T>
    using container_type = std::array<T, row_size * column_size>;
};
struct variable_length : detail::size_base {
    template<class T>
    using container_type = std::vector<T>;
};

template<class T, class = void>
struct is_size_spec : std::false_type {};
template<class T>
struct is_size_spec<T, std::void_t<std::enable_if_t<std::is_base_of_v<detail::size_base, T>>>> : std::true_type {};
template<class T>
constexpr bool is_size_spec_v = is_size_spec<T>::value;

template<class T>
constexpr bool is_variable_length_v = std::is_same_v<variable_length, T>;
template<class T>
constexpr bool is_fixed_length_v = is_size_spec_v<T> && !is_variable_length_v<T>;

namespace detail {

enum class computability {
    impossible, possible, maybe
};

/*************** 加算可能性 ****************/

template<class S, class T, class = void>
struct add_possibility {
    computability value = computability::impossible;
};
template<class S, class T>
struct add_possibility<S, T, std::enable_if_t<is_variable_length_v<S> || is_variable_length_v<T> &&
                                             (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    computability value = computability::maybe;
};
template<size_t R, size_t C>
struct add_possibility<fixed_length<R, C>, fixed_length<R, C>, void> {
    computability value = computability::possible;
};
template<class S, class T>
constexpr computability add_possibility_v = add_possibility<S, T>::value;

/*************** 乗算可能性 ****************/

template<class S, class T, class = void>
struct mul_possibility {
    computability value = computability::impossible;
};
template<class S, class T>
struct mul_possibility<S, T, std::enable_if_t<is_variable_length_v<S> || is_variable_length_v<T> &&
                                             (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    computability value = computability::maybe;
};
template<size_t R, size_t C, size_t C2>
struct mul_possibility<fixed_length<R, C>, fixed_length<C, C2>, void> {
    computability value = computability::possible;
};
template<class S, class T>
constexpr computability mul_possibility_v = mul_possibility<S, T>::value;

} // namespace detail
} // namespace matrix_size_specifier

template<class T, class Size, std::enable_if_t<is_size_spec_v<Size>>* = nullptr>
class basic_matrix {
    using container_type = typename Size:: template container_type<T>;
    size_t row_size_;
    size_t column_size_;
    container_type values_;
public:
    // 固定長行列
    template<class S = Size, std::enable_if_t<is_fixed_length_v<S>>* = nullptr>
    constexpr basic_matrix()
        : row_size_{S::row_size}
        , column_size_{S::column_size}
        , values_{}
    {}
    template<class S = Size, std::enable_if_t<is_fixed_length_v<S>>* = nullptr>
    constexpr basic_matrix(std::initializer_list<T> il)
        : basic_matrix()
    {
        auto ilbeg = il.begin();
        auto vbeg = values_.begin();
        for (auto i = 0ul; i < total_size(); ++i) {
            *vbeg++ = *ilbeg++;
        }
    }
    // 可変長行列
    template<class S = Size, std::enable_if_t<is_variable_length_v<S>>* = nullptr>
    basic_matrix(size_t row = 0, size_t column = 0)
        : row_size_{row}
        , column_size_{column}
        , values_(row * column)
    {}
    template<class S = Size, std::enable_if_t<is_variable_length_v<S>>* = nullptr>
    basic_matrix(std::initializer_list<T> il, size_t row, size_t column)
        : row_size_{row}
        , column_size_{column}
        , values_{il}
    {
        assert(il.size() == row_size_ * column_size_);
    }

    constexpr std::pair<size_t, size_t> size() const noexcept
    {
        return std::make_pair(row_size_, column_size_);
    }
    constexpr size_t total_size() const noexcept
    {
        return row_size_ * column_size_;
    }

};

// 固定長の行列(constexpr)
template<class T, size_t R, size_t C>
using fl_matrix = basic_matrix<T, fixed_length<R, C>>;
// 可変長の行列(c++17では実行時計算しかできない)
template<class T>
using vl_matrix = basic_matrix<T, variable_length>;

}
