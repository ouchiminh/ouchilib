#pragma once
#include <type_traits>
#include <cmath>
#include "ouchilib/math/matrix.hpp"

namespace ouchi::geometry {

template<class T, size_t Dim>
using point = ouchi::math::fl_matrix<T, Dim, 1>;

template<class P>
struct point_traits;

template<class T, size_t Dim>
struct point_traits<ouchi::math::fl_matrix<T, Dim, 1>> {
    using type = ouchi::math::fl_matrix<T, Dim, 1>;
    using coord_type = T;
    static constexpr size_t dim = Dim;

    static constexpr const T& get(const type& p, size_t d) noexcept { return p(d); }
    static constexpr T& set(type& p, size_t d, const coord_type& v) { return p(d) = v; }
    static constexpr type add(const type& lhs, const type rhs) { return lhs + rhs; }
    static constexpr type sub(const type& lhs, const type rhs) { return lhs - rhs; }
    static constexpr T inner_product(const type& lhs, const type& rhs)
    {
        T res{};
        for (auto d = 0ul; d < dim; ++d) {
            res += get(lhs, d) * get(rhs, d);
        }
        return res;
    }
    static constexpr type mul(const type& vec, const T& scalar)
    {
        return vec * scalar;
    }
    static constexpr type zero() { return type{}; }
    static constexpr coord_type sqdistance(const type& lhs, const type& rhs)
    {
        coord_type sum{};
        for (auto i = 0ul; i < dim; ++i) {
            auto d = get(lhs, i) - get(rhs, i);
            sum += d * d;
        }
        return sum;
    }
    static coord_type distance(const type& lhs, const type& rhs)
    {
        using std::sqrt;
        return sqrt(sqdistance(lhs, rhs));
    }
};

}
