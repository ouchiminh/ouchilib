﻿#pragma once

#include <utility>
#include <stdexcept>
#include <type_traits>
#include <array>
#include <vector>
#include <cstddef>
#include <cassert>

#include "ouchilib/result/result.hpp"

namespace ouchi::math {

template<class T>
inline constexpr T abs(T value) { return value < T{ 0 } ? -value : value; }

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
struct is_variable_length : std::bool_constant<std::is_same_v<variable_length, T>> {};
template<class T>
constexpr bool is_variable_length_v = is_variable_length<T>::value;
template<class T>
struct is_fixed_length : std::false_type {};
template<size_t R, size_t C>
struct is_fixed_length<fixed_length<R, C>> : std::true_type {};
template<class T>
constexpr bool is_fixed_length_v = is_fixed_length<T>::value;

template<class T>
struct mat_size {
    static constexpr size_t row = 0;
    static constexpr size_t column = 0;
};
template<size_t R, size_t C>
struct mat_size<fixed_length<R, C>> {
    static constexpr size_t row = R;
    static constexpr size_t column = C;
};
template<class T>
constexpr size_t mat_size_r = mat_size<T>::row;
template<class T>
constexpr size_t mat_size_c = mat_size<T>::column;

namespace detail {

enum class condvalue {
    no, yes, maybe
};

/*************** 加算可能性 ****************/
template<class S, class T, class = void>
struct add_possibility {
    static constexpr condvalue value = condvalue::no;
};
template<class S, class T>
struct add_possibility<S, T, std::enable_if_t<(is_variable_length_v<S> || is_variable_length_v<T>) &&
                                              (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    static constexpr condvalue value = condvalue::maybe;
    using result_type = variable_length;
};
template<size_t R, size_t C>
struct add_possibility<fixed_length<R, C>, fixed_length<R, C>, void> {
    static constexpr condvalue value = condvalue::yes;
    using result_type = fixed_length<R, C>;
};
template<class S, class T>
constexpr condvalue add_possibility_v = add_possibility<S, T>::value;
template<class S, class T>
using add_possibility_t = typename add_possibility<S, T>::result_type;

/*************** 乗算可能性 ****************/
template<class S, class T, class = void>
struct mul_possibility {
    static constexpr condvalue value = condvalue::no;
};
template<class S, class T>
struct mul_possibility<S, T, std::enable_if_t<(is_variable_length_v<S> || is_variable_length_v<T>) &&
                                              (is_size_spec_v<S> && is_size_spec_v<T>)>>
{
    static constexpr condvalue value = condvalue::maybe;
    using result_type = variable_length;
};
template<size_t R, size_t C, size_t C2>
struct mul_possibility<fixed_length<R, C>, fixed_length<C, C2>, void> {
    static constexpr condvalue value = condvalue::yes;
    using result_type = fixed_length<R, C2>;
};
template<class S, class T>
constexpr condvalue mul_possibility_v = mul_possibility<S, T>::value;
template<class S, class T>
using mul_possibility_t = typename mul_possibility<S, T>::result_type;

/*************** 正方 ****************/
template<class S>
struct is_square {
    static constexpr condvalue value = condvalue::no;
};
template<>
struct is_square<variable_length> {
    static constexpr condvalue value = condvalue::maybe;
};
template<size_t Size>
struct is_square<fixed_length<Size, Size>> {
    static constexpr condvalue value = condvalue::yes;
};
template<class S>
constexpr condvalue is_square_v = is_square<S>::value;

/*************** N次以上の正方行列か ****************/
template<class S, size_t N = 1, class = void>
struct is_n_by_n_or_larger {
    static constexpr condvalue value = condvalue::no;
};
template<class S, size_t N>
struct is_n_by_n_or_larger<S, N, std::enable_if_t<is_variable_length_v<S>>> {
    static constexpr condvalue value = condvalue::maybe;
};
template<size_t R, size_t N>
struct is_n_by_n_or_larger<fixed_length<R, R>, N, std::enable_if_t<(R >= N)>> {
    static constexpr condvalue value = condvalue::yes;
};
template<class T, size_t N = 1>
constexpr condvalue is_n_by_n_or_larger_v = is_n_by_n_or_larger<T, N>::value;

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
    explicit constexpr basic_matrix(const T& fill_value)
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
    template<class S = Size>
    static constexpr auto identity() noexcept(is_fixed_length_v<S>)
        -> std::enable_if_t<(detail::is_square_v<S> == detail::condvalue::yes), basic_matrix<T, S>>
    {
        basic_matrix<T, S> E{};
        for (auto i = 0u; i < E.size().first; ++i) E(i, i) = T{ 1 };
        return E;
    }
    static basic_matrix<T, variable_length> identity(size_t n)
    {
        basic_matrix<T, variable_length> E(n, n, T{ 0 });
        for (auto i = 0u; i < E.size().first; ++i) E(i, i) = T{ 1 };
        return E;
    }

    constexpr void assign(std::initializer_list<T> il) noexcept
    {
        if constexpr (is_variable_length_v<Size>) {
            assert(il.size() == total_size());
        } else {
            static_assert(il.size() == total_size());
        }
        auto vbeg = values_.begin();
        auto ibeg = il.begin();
        for (auto i = 0ul; i < total_size(); ++i) {
            *vbeg++ = *ibeg++;
        }
    }
    template<class S = Size, std::enable_if_t<is_variable_length_v<S>>* = nullptr>
    void assign(std::initializer_list<T> il, size_t row, size_t column)
    {
        resize(row, column);
        assign(il);
    }
    T* data() noexcept
    {
        return values_.data();
    }
    const T* data() const noexcept
    {
        return values_.data();
    }
    auto begin() noexcept { return values_.begin(); }
    auto begin() const noexcept { return values_.begin(); }
    auto end() noexcept { return values_.end(); }
    auto end() const noexcept { return values_.end(); }

    [[nodiscard]]
    constexpr std::pair<size_t, size_t> size() const noexcept
    {
        return std::make_pair(row_size_, column_size_);
    }
    [[nodiscard]]
    constexpr size_t total_size() const noexcept
    {
        return row_size_ * column_size_;
    }
    /// <summary>
    /// 行列のi, j成分を返す
    /// </summary>
    [[nodiscard]]
    constexpr const T& operator()(size_t i, size_t j) const noexcept
    {
        return values_[i * column_size_ + j];
    }
    [[nodiscard]]
    constexpr T& operator()(size_t i, size_t j) noexcept
    {
        return values_[i * column_size_ + j];
    }
    /// <summary>
    /// 0行0列 (0, 0) から (0, 1), (0, 2)...と数えていったときのi番目の成分
    /// </summary>
    [[nodiscard]]
    constexpr const T& operator()(size_t i) const noexcept
    {
        return values_[i];
    }
    [[nodiscard]]
    constexpr T& operator()(size_t i)  noexcept
    {
        return values_[i];
    }

    template<class S = Size>
    auto resize(size_t row, size_t column, const T& v = T{})
        -> std::enable_if_t<is_variable_length_v<S>>
    {
        row_size_ = row;
        column_size_ = column;
        values_.clear();
        values_.resize(total_size(), v);
    }

    template<class S = Size>
    constexpr auto row(size_t i) const noexcept(is_fixed_length_v<S>)
        -> basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<1, mat_size_c<S>>, variable_length>>
    {
        using ret_t = basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<1, mat_size_c<S>>, variable_length>>;
        ret_t ret{};
        if constexpr (is_variable_length_v<S>) {
            ret.resize(1, size().second);
        }
        for (auto j = 0ul; j < size().second; ++j) {
            ret(j) = (*this)(i, j);
        }
        return std::move(ret);
    }
    template<class S = Size>
    constexpr auto column(size_t i) const noexcept(is_fixed_length_v<S>)
        -> basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_r<S>, 1>, variable_length>>
    {
        using ret_t = basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_r<S>, 1>, variable_length>>;
        ret_t ret{};
        if constexpr (is_variable_length_v<S>) {
            ret.resize(size().first, 1);
        }
        for (auto j = 0ul; j < size().first; ++j) {
            ret(j) = (*this)(j, i);
        }
        return std::move(ret);
    }
    void swap_row(size_t i, size_t j) noexcept
    {
        using std::swap;
        for (int k = 0; k < size().second; ++k) swap((*this)(i, k), (*this)(j, k));
    }
    void swap_column(size_t i, size_t j) noexcept
    {
        using std::swap;
        for (int k = 0; k < size().first; ++k) swap((*this)(k, i), (*this)(k, j));
    }

    // 転置
    template<class S = Size>
    [[nodiscard]]
    constexpr auto transpose() const noexcept(is_fixed_length_v<S>)
        -> basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_c<Size>, mat_size_r<Size>>, variable_length>>
    {
        using ret_t = basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_c<Size>, mat_size_r<Size>>, variable_length>>;
        ret_t ret{};
        if constexpr (is_variable_length_v<S>) {
            ret.resize(size().second, size().first);
        }
        for (auto i = 0ul; i < size().first; ++i) {
            for (auto j = 0ul; j < size().second; ++j) {
                ret(j, i) = (*this)(i, j);
            }
        }
        return std::move(ret);
    }

    // 小行列
    template<class S = Size>
    [[nodiscard]]
    constexpr auto minor(size_t i, size_t j) const noexcept(is_fixed_length_v<S>)
        -> std::enable_if_t<(detail::is_n_by_n_or_larger_v<S, 2> > detail::condvalue::no),
                            basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_r<Size> - 1, mat_size_c<S> - 1>, variable_length>>>
    {
        using mat_t = basic_matrix<T, std::conditional_t<is_fixed_length_v<S>, fixed_length<mat_size_r<S> - 1, mat_size_c<S> - 1>, variable_length>>;
        mat_t ret;
        if constexpr (is_variable_length_v<S>) {
            if (size().first != size().second) throw std::domain_error("non-square matrix have no minor determinant");
            size_t n = size().first - 1;
            ret.resize(n, n);
        }
        auto p = 0ul;
        auto q = 0ul;
        for (auto l = 0ul; l < size().first; ++l) {
            if (l == i) continue;
            q = 0;
            for (auto m = 0ul; m < size().second; ++m) {
                if (m == j) continue;
                ret(p, q++) = (*this)(l, m);
            }
            ++p;
        }
        return std::move(ret);
    }
    // 余因子行列
    template<class S = Size>
    [[nodiscard]]
    constexpr auto cofactor_matrix() const noexcept(is_fixed_length_v<S>)
        -> std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), basic_matrix>
    {
        auto sgn = [](auto i, auto j) { return (i + j) & 1 ? -1 : 1; };
        basic_matrix ret;
        if constexpr (is_variable_length_v<S>) {
            if (size().first != size().second) throw std::domain_error("non-square matrix have no cofactor matrix");
            size_t n = size().first;
            ret.resize(n, n);
        }
        for (auto i = 0ul; i<ret.size().first; ++i) {
            for (auto j = 0ul; j<ret.size().second; ++j) {
                ret(i, j) = sgn(i, j) * det(minor(j, i));
            }
        }
        return std::move(ret);
    }
    [[nodiscard]]
    constexpr T cofactor(size_t i, size_t j) const noexcept
    {
        return ((i + j & 1) ? -1 : 1) * det(minor(i, j));
    }
    template<class S = Size>
    [[nodiscard]]
    auto pivot_lu() const noexcept(is_fixed_length_v<S>)
        ->std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), std::pair<basic_matrix, basic_matrix>>
    {
        basic_matrix P;
        basic_matrix cp = *this;
        size_t ipv = 0;
        auto find_pivot = [&cp, &ipv, n = size().first](){
            T max = abs(cp(ipv, ipv));
            size_t idx = ipv;
            for (auto i = ipv + 1; i < n; ++i) {
                if (abs(cp(i, ipv)) > max) {
                    max = abs(cp(i, ipv));
                    idx = i;
                }
            }
            return idx;
        };
        if constexpr (is_variable_length_v<S>) {
            if (size().first != size().second) throw std::domain_error("size error. designate square matrix");
            P.resize(size().first, size().first);
        }
        for (auto i = 0u; i < size().first; ++i) P(i, i) = T{ 1 };
        for (ipv = 0; ipv < size().first - 1; ++ipv) {
            auto local_pivot_index = find_pivot();
            P.swap_row(ipv, local_pivot_index);
            cp.swap_row(ipv, local_pivot_index);
            if (cp(ipv, ipv) == T{ 0 }) continue;
            T r = T{ 1 } / cp(ipv, ipv);
            for (auto j = ipv+1; j < cp.size().first; ++j) {
                cp(j, ipv) *= r;
                for (auto k = ipv+1; k < cp.size().second; ++k) {
                    cp(j, k) -= cp(j, ipv)*cp(ipv, k);
                }
            }
        }
        return { P, cp };
    }
    template<class S = Size>
    [[nodiscard]]
    constexpr auto lu() const noexcept(detail::is_square_v<S> == detail::condvalue::yes)
        ->std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), basic_matrix>
    {
        if constexpr (detail::is_square_v<S> == detail::condvalue::maybe) {
            if (size().first != size().second) throw std::domain_error("non-square matrix doesn't have determinant");
        }
        const size_t n = size().first;
        basic_matrix cp = *this;
        for (auto i = 0ul; i<n-1; ++i) {
            if (cp(i, i) == T{ 0 }) continue;
            T r = T{ 1 }/cp(i, i);
            for (auto j = i + 1; j<n; ++j) {
                cp(j, i) *= r;
                for (auto k = i+1; k<n; ++k) {
                    cp(j, k) -= cp(j, i)*cp(i, k);
                }
            }
        }
        return cp;
    }


    template<class S = Size>
    [[nodiscard]]
    constexpr auto inv() const noexcept(is_fixed_length_v<S>)
        -> std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), result::result<basic_matrix, std::string_view>>
    {
        basic_matrix inv;
        if constexpr (is_variable_length_v<S>) {
            if (size().first != size().second) return result::err("matrix needs to be square");
            inv.resize(size().first, size().first);
        }
        auto determinant = det(*this);
        if (determinant == T{ 0 }) return result::err("matrix needs to be regular");
        auto r = T{ 1 } / det(*this);
        for (int i = 0; i < size().first; ++i) {
            for (int j = 0; j < size().second; ++j)
                inv(i, j) = cofactor(j, i) * r;
        }
        return result::ok(inv);
    }
    /******** 算術演算 ********/

private:
    template<class M1, class M2, class RM>
    static constexpr auto add(const M1& m1, const M2& m2, RM& res) noexcept
        -> std::enable_if_t<(detail::add_possibility_v<typename M1::size_spec_type, typename M2::size_spec_type> > detail::condvalue::no)>
    {
        // privateなので十分な加算可能性が検証された後で呼ばれる前提。型チェックは行わない。
        for (auto i = 0u; i < res.total_size(); ++i) {
            res(i) = static_cast<T>(m1(i)) + static_cast<T>(m2(i));
        }
    }
    template<class M1, class M2, class RM>
    static constexpr auto mul(const M1& m1, const M2& m2, RM& res) noexcept
        ->std::enable_if_t<(detail::mul_possibility_v<typename M1::size_spec_type, typename M2::size_spec_type> > detail::condvalue::no)>
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
    [[nodiscard]]
    friend constexpr auto operator-(const basic_matrix& a) noexcept(is_fixed_length_v<Size>)
    {
        basic_matrix res;
        if constexpr (is_variable_length_v<typename basic_matrix::size_spec_type>) {
            res.resize(a.size().first, a.size().second);
        }
        for (auto i = 0u; i < res.total_size(); ++i)
            res(i) = -a(i);
        return res;
    }

    /******** 足し算 ********/

    template<class U, class S>
    [[nodiscard]]
    friend constexpr auto operator+(const basic_matrix& a, const basic_matrix<U, S>& b)
        noexcept(is_fixed_length_v<Size>&& is_fixed_length_v<S>)
        ->std::enable_if_t<(detail::add_possibility_v<Size, S> > detail::condvalue::no), basic_matrix<std::common_type_t<T, U>, detail::add_possibility_t<S, Size>>>
    {
        basic_matrix<std::common_type_t<T, U>, detail::add_possibility_t<S, Size>> res;
        // 分岐はコンパイル時だが、副文の実行は実行時
        if constexpr (is_variable_length_v<detail::add_possibility_t<Size, S>>) {
            if (a.size() != b.size()) throw std::domain_error("two matrixes that have different size cannot be added each other.");
            res.resize(a.size().first, a.size().second);
        }
        basic_matrix::add(a, b, res);
        return std::move(res);
    }
    template<class U, class S>
    [[nodiscard]]
    friend constexpr auto operator-(const basic_matrix& a, const basic_matrix<U, S>& b)
        noexcept(is_fixed_length_v<Size>&& is_fixed_length_v<S>)
    {
        return a + (-b);
    }

    /******** 掛け算 ********/

    template<class U, class S>
    [[nodiscard]]
    friend constexpr auto operator*(const basic_matrix<T, Size>& a, const basic_matrix<U, S>& b)
        noexcept(is_fixed_length_v<Size>&& is_fixed_length_v<S>)
        ->std::enable_if_t<(detail::mul_possibility_v<Size, S> > detail::condvalue::no), basic_matrix<std::common_type_t<T, U>, detail::mul_possibility_t<Size, S>>>
    {
        basic_matrix<std::common_type_t<T, U>, detail::mul_possibility_t<Size, S>> res;
        if constexpr (is_variable_length_v<detail::mul_possibility_t<Size, S>>) {
            if (a.size().second != b.size().first) throw std::domain_error("multiplication can be applied only if size of lhs.column == that of rhs.row");
            res.resize(a.size().first, b.size().second);
        }
        basic_matrix::mul(a, b, res);
        return std::move(res);
    }

    /************* スカラーとの積 *************/

    [[nodiscard]]
    friend constexpr auto operator*(const basic_matrix& a, const T& scalar)
        noexcept(is_fixed_length_v<Size> && noexcept(std::declval<T>() * std::declval<T>()))
        -> basic_matrix<T, Size>
    {
        basic_matrix<T, Size> ret;
        if constexpr (is_variable_length_v<Size>) {
            ret.resize(a.size().first, a.size().second);
        }
        for (auto i = 0ul; i < a.total_size(); ++i) {
            ret(i) = a(i) * scalar;
        }
        return std::move(ret);
    }
    [[nodiscard]]
    friend constexpr auto operator*(const T& scalar, const basic_matrix& a)
        noexcept(is_fixed_length_v<Size> && noexcept(std::declval<T>() * std::declval<T>()))
        -> basic_matrix<T, Size>
    {
        return a * scalar;
    }
    [[nodiscard]]
    friend constexpr auto operator/(const basic_matrix& a, const T& scalar)
        noexcept(is_fixed_length_v<Size> && noexcept(std::declval<T>() * std::declval<T>()))
        -> basic_matrix<T, Size>
    {
        basic_matrix<T, Size> ret;
        if constexpr (is_variable_length_v<Size>) {
            ret.resize(a.size().first, a.size().second);
        }
        for (auto i = 0ul; i < a.total_size(); ++i) {
            ret(i) = a(i) / scalar;
        }
        return std::move(ret);
    }

    // 比較演算
    template<class U, class S>
    friend constexpr bool operator==(const basic_matrix& m1, const basic_matrix<U, S>& m2) noexcept
    {
        if (m1.size() != m2.size()) return false;
        for (auto i = 0ul; i < m1.total_size(); ++i) {
            if (m1(i) != m2(i)) return false;
        }
        return true;
    }
    template<class U, class S>
    friend constexpr bool operator!=(const basic_matrix& m1, const basic_matrix<U, S> m2) noexcept
    {
        return !(m1 == m2);
    }
};

template<class T, class S>
[[nodiscard]]
constexpr auto fast_det(const basic_matrix<T, S>& m)
    noexcept(detail::is_square_v<S> == detail::condvalue::yes)
    ->std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), T>
{
    if constexpr (detail::is_square_v<S> == detail::condvalue::maybe) {
        if (m.size().first != m.size().second) throw std::domain_error("non-square matrix doesn't have determinant");
    }
    size_t n = m.size().first;
    T res = T{ 1 };
    basic_matrix<T, S> mat = m.lu();
    //対角部分の積
    for (auto i = 0ul; i<n; i++) {
        res *= mat(i, i);
    }
    return res;
}

template<class T, class S>
constexpr auto det(const basic_matrix<T, S>& m)
    noexcept(detail::is_square_v<S> == detail::condvalue::yes)
    ->std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no), T>
{
    auto sgn = [](auto i, auto j) { return (i + j) & 1 ? -1 : 1; };
    if constexpr (detail::is_square_v<S> == detail::condvalue::maybe) {
        if (m.size().first != m.size().second) throw std::domain_error("non-square matrix doesn't have determinant");
    }
    if (m.total_size() == 1) return m(0);
    T ret = T{};
    if constexpr (detail::is_n_by_n_or_larger_v<S, 2> > detail::condvalue::no) {
        for (auto i = 0ul; i < m.size().first; ++i) {
            ret += sgn(i, 0) * m(i, 0) * det(m.minor(i, 0));
        }
    }
    return ret;
}

template<class T, class S>
constexpr auto extract_lu(const basic_matrix<T, S>& lu) noexcept(is_fixed_length_v<S>)
->std::enable_if_t<(detail::is_square_v<S> > detail::condvalue::no),
                   std::pair<basic_matrix<T, S>, basic_matrix<T, S>>>
{
    basic_matrix<T, S> l, u;
    if constexpr (is_variable_length_v<S>) {
        if (lu.size().first != lu.size().second) throw std::domain_error("lu matrix needs to be square matrix");
        l.resize(lu.size().first, lu.size().first);
        u.resize(lu.size().first, lu.size().first);
    }
    for (auto i = 0ul; i < lu.size().first; ++i) {
        for (auto j = 0ul; j < lu.size().first; ++j) {
            l(i, j) = j < i ? lu(i, j) : j == i ? T{ 1 } : T{ 0 };
            u(i, j) = j >= i ? lu(i, j) : T{ 0 };
        }
    }
    return { l, u };
}

template<class T, class S>
constexpr auto solve(const basic_matrix<T, S>& m);

// 固定長の行列(constexpr)
template<class T, size_t R, size_t C>
using fl_matrix = basic_matrix<T, fixed_length<R, C>>;
// 可変長の行列(c++17では実行時計算しかできない)
template<class T>
using vl_matrix = basic_matrix<T, variable_length>;

template<class T, class S>
auto begin(basic_matrix<T, S>& m) { return m.begin(); }
template<class T, class S>
auto begin(const basic_matrix<T, S>& m) { return m.begin(); }
template<class T, class S>
auto end(basic_matrix<T, S>& m) { return m.end(); }
template<class T, class S>
auto end(const basic_matrix<T, S>& m) { return m.end(); }
}

