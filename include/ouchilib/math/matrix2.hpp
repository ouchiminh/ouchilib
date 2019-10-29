#pragma once

#include <type_traits>
#include <array>
#include <vector>
#include <cstddef>

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

} // namespace matrix_size_specifier

template<class T, class Size, std::enable_if_t<is_size_spec_v<Size>>* = nullptr>
class basic_matrix {
    using container_type = typename Size:: template container_type<T>;
    size_t row_size_;
    size_t column_size_;
    container_type values_;
    // 固定長行列
    template<class S, std::enable_if_t<is_fixed_length_v<S>>* = nullptr>
    constexpr basic_matrix(S)
        : row_size_{S::row_size}
        , column_size_{S::column_size}
        , values_{}
    {}
    // 可変長行列
    template<class S, std::enable_if_t<is_variable_length_v<S>>* = nullptr>
    basic_matrix(S)
        : row_size_{0}
        , column_size_{0}
        , values_{}
    {}
public:
    constexpr basic_matrix()
        : basic_matrix(Size{})
    {}

    constexpr std::pair<size_t, size_t> size() const noexcept {
        return std::make_pair(row_size_, column_size_);
    }

};

template<class T, size_t R, size_t C>
using fl_matrix = basic_matrix<T, fixed_length<R, C>>;
template<class T>
using vl_matrix = basic_matrix<T, variable_length>;

}
