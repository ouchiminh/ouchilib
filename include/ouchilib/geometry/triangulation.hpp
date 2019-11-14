#pragma once

#include <cmath>
#include <iterator>
#include <array>
#include <vector>
#include <optional>
#include "ouchilib/math/matrix.hpp"
#include "point_traits.hpp"

namespace ouchi::geometry {

template<class Pt>
struct triangulation {
    static constexpr size_t dim = point_traits<Pt>::dim;
    using calc_type = point_traits<Pt>::coord_type;
    using simplex = std::array<size_t, dim>;

    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};
    
    template<class Itr, std::enable_if_t<std::is_same_v<std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<simplex> operator()(Itr first, Itr last, return_as_idx_tag) const;
    template<class Itr, std::enable_if_t<std::is_same_v<std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<std::array<Pt, dim>>  operator()(Itr first, Itr last) const;

    void set_initial_state(simplex space) { space_ = space; }
private:
    std::optional<simplex> space_;

    using pt = point_traits<Pt>;

    template<class Itr>
    simplex initialize(Itr first, Itr last)
    {
        using std::sqrt;
        // N次元の点群について各次元の最大値と最小値を求める : min_i, max_i
        std::array<calc_type, dim> max{};
        std::array<calc_type, dim> min{};
        for (auto it = first; it != last; ++it) {
            for (auto d = 0u; d < dim; ++d) {
                if (pt::get(*it, d) > max[d]) max[d] = pt::get(*it, d);
                if (min[d] < pt::get(*it, d)) min[d] = pt::get(*it, d);
            }
        }
        // 求めた各次元の最大値と最小値を球面に含むN次元の球
        // N次元空間上の点(min_0, ..., min_n)から点(max_0, ..., max_n)に引いた線の長さを求める=半径
        calc_type r{};
        for (auto d = 0u; d < dim; ++d) {
            auto diff = max[d] - min[d];
            r += diff * diff;
        }
        r = sqrt(r);
        // 球の中心を求める。
        Pt o{};
        for (auto d = 0u; d < dim; ++d) {
            pt::set(o, d, (max[d] - min[d]) / 2);
        }
        // 球に外接する単体を求める
        simplex ret{};
    }
    
};

}
