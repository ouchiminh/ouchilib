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

    [[nodiscard]]
    static constexpr const T& get(const type& p, size_t d) noexcept { return p(d); }
    static constexpr T& set(type& p, size_t d, const coord_type& v) noexcept { return p(d) = v; }
    [[nodiscard]]
    static constexpr type zero() { return type{}; }
    [[nodiscard]]
    [[deprecated]]
    static constexpr type add(const type& lhs, const type rhs) { return lhs + rhs; }
    [[nodiscard]]
    [[deprecated]]
    static constexpr type sub(const type& lhs, const type rhs) { return lhs - rhs; }
    [[nodiscard]]
    [[deprecated]]
    static constexpr T inner_product(const type& lhs, const type& rhs)
    {
        T res{};
        for (auto d = 0ul; d < dim; ++d) {
            res += get(lhs, d) * get(rhs, d);
        }
        return res;
    }
    [[nodiscard]]
    [[deprecated]]
    static constexpr type mul(const type& vec, const T& scalar)
    {
        return vec * scalar;
    }
    [[nodiscard]]
    [[deprecated]]
    static constexpr coord_type sqdistance(const type& lhs, const type& rhs)
    {
        coord_type sum{};
        for (auto i = 0ul; i < dim; ++i) {
            auto d = get(lhs, i) - get(rhs, i);
            sum += d * d;
        }
        return sum;
    }
    [[nodiscard]]
    [[deprecated]]
    static coord_type distance(const type& lhs, const type& rhs)
    {
        using std::sqrt;
        return sqrt(sqdistance(lhs, rhs));
    }
};

template<class P>
constexpr auto add(const P& lhs, const P& rhs)
->typename point_traits<P>::type
{
    P res;
    for (auto d = 0u; d < point_traits<P>::dim; ++d) {
        point_traits<P>::set(res, d,
                             point_traits<P>::get(lhs, d) + point_traits<P>::get(rhs, d));
    }
    return res;
}

template<class P>
constexpr auto sub(const P& lhs, const P& rhs)
->typename point_traits<P>::type
{
    P res;
    for (auto d = 0u; d < point_traits<P>::dim; ++d) {
        point_traits<P>::set(res, d,
                             point_traits<P>::get(lhs, d) - point_traits<P>::get(rhs, d));
    }
    return res;
}

template<class P>
constexpr auto inner_product(const P& lhs, const P& rhs)
->typename point_traits<P>::coord_type
{
    using T = typename point_traits<P>::coord_type;
    T res{};
    for (auto d = 0ul; d < point_traits<P>::dim; ++d) {
        res += point_traits<P>::get(lhs, d) * point_traits<P>::get(rhs, d);
    }
    return res;
}

template<class P>
constexpr auto mul(const P& vec, const typename point_traits<P>::coord_type& scalar)
->typename point_traits<P>::type
{
    P res;
    for (auto d = 0u; d < point_traits<P>::dim; ++d) {
        point_traits<P>::set(res, d, point_traits<P>::get(vec, d) * scalar);
    }
    return res;
}

template<class P>
constexpr auto sqdistance(const P& lhs, const P& rhs)
->typename point_traits<P>::coord_type
{
    using T = typename point_traits<P>::coord_type;
    T s{};
    for (auto i = 0u; i < point_traits<P>::dim; ++i) {
        auto d = point_traits<P>::get(lhs, i) - point_traits<P>::get(rhs, i);
        s += d * d;
    }
    return s;
}

template<class P>
constexpr auto distance(const P& lhs, const P& rhs)
->typename point_traits<P>::coord_type
{
    using std::sqrt;
    return sqrt(sqdistance(lhs, rhs));
}
}

