#pragma once
#include <type_traits>
#include <initializer_list>
#include <vector>
#include <cassert>
#include "ouchilib/utl/multiitr.hpp"

namespace ouchi::math {

template<class T, size_t Row, size_t Column>
struct matrix {
private:
    std::vector<T> data_;
public:
    constexpr matrix() : data_(Row* Column) {};
    matrix(std::initializer_list<T> il) : matrix()
    {
        assert(il.size() == Row * Column);
        data_.assign(il);
    }
    matrix(const T& value)
        : data_(Row * Column, value)
    {}
    matrix(const matrix&) = default;
    matrix(matrix&&) = default;
    matrix& operator=(const matrix&) = default;
    matrix& operator=(matrix&&) = default;
    
    template<size_t OtherColumn>
    friend matrix<T, Row, OtherColumn> 
        operator*(const matrix& lhs, const matrix<T, Column, OtherColumn>& rhs)
    {
        matrix<T, Row, OtherColumn> res(T{0});
        for (size_t i = 0; i < Row; ++i) {
            for (size_t j = 0; j < OtherColumn; ++j) {
                for (size_t k = 0; k < Column; ++k) {
                    res.at(i, j) += lhs.at(i, k) * rhs.at(k, j);
                }
            }
        }
        return std::move(res);
    }
    friend matrix operator+(const matrix& lhs, const matrix& rhs)
    {
        matrix res{};
        for (auto [a, l, r] : ouchi::multiitr{ res.data, lhs.data_, rhs.data_ }) {
            a = l + r;
        }
        return res;
    }
    friend matrix operator-(const matrix& lhs, const matrix& rhs)
    {
        matrix res{};
        for (auto [a, l, r] : ouchi::multiitr{ res.data, lhs.data_, rhs.data_ }) {
            a = l - r;
        }
        return res;
    }

    template<class F, std::enable_if_t<std::is_invocable_r_v<T, F, T>>* = nullptr>
    void apply_row(size_t row, F&& func)
    {
        for (size_t i = 0u; i < Column; ++i) {
            at(row, i) = func(std::as_const(this)->at(row, i));
        }
    }
    template<class F, std::enable_if_t<std::is_invocable_r_v<T, F, T>>* = nullptr>
    void apply_column(size_t column, F&& func)
    {
        for (size_t i = 0u; i < Row; ++i) {
            at(column, i) = func(std::as_const(this)->at(column, i));
        }
    }

    const T& at(size_t r, size_t c) const {
        return data_.at(r * Column + c);
    }
    T& at(size_t r, size_t c) {
        return data_.at(r * Column + c);
    }
    size_t size() const noexcept { return data_.size(); }
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
    auto begin() const { return data_.cbegin(); }
    auto end() const { return data_.cend(); }
    auto data() const { return data_.data(); }
    friend auto begin(matrix& m) { return m.begin(); }
    friend auto begin(const matrix& m) { return m.begin(); }
    friend auto end(matrix& m) { return m.end(); }
    friend auto end(const matrix& m) { return m.end(); }
};

}
