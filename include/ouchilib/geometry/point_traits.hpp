#pragma once
#include <type_traits>
#include "ouchilib/math/matrix.hpp"

namespace ouchi::geometry {

template<class P>
struct point_traits;

template<class T, size_t Dim>
struct point_traits<ouchi::math::fl_matrix<T, Dim, 1>> {
    using type = ouchi::math::fl_matrix<T, Dim, 1>;
    using coord_type = T;
    static constexpr size_t dim = Dim;

    static const T& get(const type& p, size_t d) noexcept { return p(d); }
    static T& set(type& p, size_t d, const coord_type& v) { return p(d) = v; }
    static type add(const type& lhs, const type rhs) { return lhs + rhs; }
    static type sub(const type& lhs, const type rhs) { return lhs - rhs; }
    static T inner_product(const type& lhs, const type& rhs)
    {
        T res{};
        for (auto d = 0ul; d < dim; ++d) {
            res += get(lhs, d) * get(rhs, d);
        }
        return res;
    }
    static type mul(const type& vec, const T& scalar)
    {
        return vec * scalar;
    }
};

}
