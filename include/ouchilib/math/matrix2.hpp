#pragma once

#include <stdexcept>
#include <string> // for error message

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
struct fixed_length final : detail::size_base {
    static constexpr size_t row_size = Row;
    static constexpr size_t column_size = Column;
    template<class T>
    using container_type = std::array<T, row_size * column_size>;
};
struct variable_length final : detail::size_base {
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
    static constexpr computability value = computability::impossible;
};
template<class S, class T>
struct add_possibility<S, T, std::enable_if_t<(is_variable_length_v<S> || is_variable_length_v<T>) &&
                                              (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    static constexpr computability value = computability::maybe;
};
template<size_t R, size_t C>
struct add_possibility<fixed_length<R, C>, fixed_length<R, C>, void> {
    static constexpr computability value = computability::possible;
};
template<class S, class T>
constexpr computability add_possibility_v = add_possibility<S, T>::value;

/*************** 乗算可能性 ****************/

template<class S, class T, class = void>
struct mul_possibility {
    static constexpr computability value = computability::impossible;
};
template<class S, class T>
struct mul_possibility<S, T, std::enable_if_t<(is_variable_length_v<S> || is_variable_length_v<T>) &&
                                              (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    static constexpr computability value = computability::maybe;
};
template<size_t R, size_t C, size_t C2>
struct mul_possibility<fixed_length<R, C>, fixed_length<C, C2>, void> {
    static constexpr computability value = computability::possible;
};
template<class S, class T>
constexpr computability mul_possibility_v = mul_possibility<S, T>::value;

} // namespace detail
} // namespace matrix_size_specifier

template<class, class Size, class = void>
class basic_matrix;

template<class T, class Size>
class basic_matrix<T, Size, std::enable_if_t<is_size_spec_v<Size>>> {
    using container_type = typename Size:: template container_type<T>;
    size_t row_size_;
    size_t column_size_;
    container_type values_;
public:
    using size_spec_type = Size;
    using value_type = T;
    // 固定長行列
    template<class S = Size, std::enable_if_t<is_fixed_length_v<S>>* = nullptr>
    constexpr basic_matrix()
        : row_size_{S::row_size}
        , column_size_{S::column_size}
        , values_{}
    {}
    template<class S = Size, std::enable_if_t<is_fixed_length_v<S>>* = nullptr>
    constexpr basic_matrix(const T& fill_value)
        : row_size_{S::row_size}
        , column_size_{S::column_size}
        , values_{}
    {
        values_.fill(fill_value);
    }
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
    basic_matrix(size_t row = 0, size_t column = 0, const T& fill_value = T{})
        : row_size_{row}
        , column_size_{column}
        , values_(row * column, fill_value)
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
    /// <summary>
    /// 行列のi, j成分を返す
    /// </summary>
    constexpr const T& operator()(size_t i, size_t j) const noexcept
    {
        return values_[i * column_size_ + j];
    }
    constexpr T& operator()(size_t i, size_t j) noexcept
    {
        return values_[i * column_size_ + j];
    }
    /// <summary>
    /// 0行0列 (0, 0) から (0, 1), (0, 2)...と数えていったときのi番目の成分
    /// </summary>
    constexpr const T& operator()(size_t i) const noexcept
    {
        return values_[i];
    }
    constexpr T& operator()(size_t i)  noexcept
    {
        return values_[i];
    }

    /******** 算術演算子 ********/

private:
    template<class M1, class M2, class RM>
    static constexpr auto add(const M1& m1, const M2& m2, RM& res) noexcept
        -> std::enable_if_t<(detail::add_possibility_v<typename M1::size_spec_type, typename M2::size_spec_type> > detail::computability::impossible)>
    {
        // privateなので十分な加算可能性が検証された後で呼ばれる前提。型チェックは行わない。
        for (auto i = 0u; i < res.total_size(); ++i) {
            res(i) = static_cast<T>(m1(i)) + static_cast<T>(m2(i));
        }
    }
    template<class M1, class M2, class RM>
    static constexpr auto mul(const M1& m1, const M2& m2, RM& res) noexcept
        ->std::enable_if_t<(detail::add_possibility_v<typename M1::size_spec_type, typename M2::size_spec_type> > detail::computability::impossible)>
    {
        // privateなので十分な乗算可能性が検証された後で呼ばれる前提。型チェックは行わない。
        auto [m1r, m1c] = m1.size();
        auto m2c = m2.size().second;
        
        for (auto i = 0u; i < m1r; ++i) {
            for (auto k = 0u; k < m1c; ++k) {
                for (auto j = 0u; j < m2c; ++j) {
                    res(i, j) += m1(i, k) * m2(k, j);
                }
            }
        }
    }
public:
    template<class S = Size>
    friend constexpr auto operator-(const basic_matrix& a) noexcept
        -> std::enable_if_t<is_fixed_length_v<S>, basic_matrix>
    {
        basic_matrix res;
        for (auto i = 0u; i < res.total_size(); ++i)
            res(i) = -a(i);
        return res;
    }
    template<class S = Size>
    friend auto operator-(const basic_matrix& a)
        -> std::enable_if_t<is_variable_length_v<S>, basic_matrix>
    {
        basic_matrix res(a.size().first, a.size().second);
        for (auto i = 0u; i < res.total_size(); ++i)
            res(i) = -a(i);
        return std::move(res);
    }

    /******** 足し算 ********/

    template<class U, class S>
    friend auto operator+(const basic_matrix& a, const basic_matrix<U, S>& b)
        -> std::enable_if_t<(detail::add_possibility_v<Size, S> == detail::computability::maybe), basic_matrix<std::common_type_t<U, T>, variable_length>>
    {
        if (a.size() != b.size())
            throw std::domain_error("two matrixes that have different size cannot be added each other.");
        using res_t = std::common_type_t<U, T>;
        basic_matrix<res_t, variable_length> res(a.size().first, a.size().second);
        basic_matrix::add(a, b, res);
        return std::move(res);
    }
    template<class U, class S>
    friend constexpr auto operator+(const basic_matrix& a, const basic_matrix<U, S>& b) noexcept
        -> std::enable_if_t<detail::add_possibility_v<Size, S> == detail::computability::possible, basic_matrix<std::common_type_t<T, U>, Size>>
    {
        using res_t = std::common_type_t<T, U>;
        basic_matrix<res_t, Size> res;
        basic_matrix::add(a, b, res);
        return res;
    }
    template<class U, class S>
    friend constexpr auto operator-(const basic_matrix& a, const basic_matrix<U, S>& b)
    {
        return a + (-b);
    }

    /******** 掛け算 ********/

    template<class U, class S>
    friend auto operator*(const basic_matrix& a, const basic_matrix<U, S>& b)
        -> std::enable_if_t<(detail::mul_possibility_v<Size, S> == detail::computability::maybe), basic_matrix<std::common_type_t<U, T>, variable_length>>
    {
        if (a.size().second != b.size().first)
            throw std::domain_error("multiplication can be applied only if size of lhs.column == that of rhs.row");
        using res_t = std::common_type_t<U, T>;
        basic_matrix<res_t, variable_length> res(a.size().first, b.size().second);
        basic_matrix::mul(a, b, res);
        return std::move(res);
    }
    template<class U, class S>
    friend constexpr auto operator*(const basic_matrix& a, const basic_matrix<U, S>& b) noexcept
        -> std::enable_if_t<detail::mul_possibility_v<Size, S> == detail::computability::possible, basic_matrix<std::common_type_t<T, U>, Size>>
    {
        using res_t = std::common_type_t<T, U>;
        basic_matrix<res_t, fixed_length<basic_matrix::size_spec_type::row_size, S::column_size>> res;
        basic_matrix::mul(a, b, res);
        return res;
    }
};

// 固定長の行列(constexpr)
template<class T, size_t R, size_t C>
using fl_matrix = basic_matrix<T, fixed_length<R, C>>;
// 可変長の行列(c++17では実行時計算しかできない)
template<class T>
using vl_matrix = basic_matrix<T, variable_length>;

}
