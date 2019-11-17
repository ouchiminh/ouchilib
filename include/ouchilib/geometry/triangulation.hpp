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
    using coord_type = point_traits<Pt>::coord_type;
    using simplex = std::array<size_t, dim>;
    using et_simplex = std::array<Pt, dim>;

    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};
    
    template<class Itr, std::enable_if_t<std::is_same_v<std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<simplex> operator()(Itr first, Itr last, return_as_idx_tag) const;
    template<class Itr, std::enable_if_t<std::is_same_v<std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<et_simplex>  operator()(Itr first, Itr last) const;

    void set_initial_state(et_simplex space) { space_ = space; }
private:
    std::optional<et_simplex> space_;

    using pt = point_traits<Pt>;

    template<class Itr>
    static constexpr et_simplex calc_space(Itr first, Itr last) noexcept
    {
        using std::sqrt;
        if (space_) return space_.value();
        // N次元の点群について各次元の最大値と最小値を求める : min_i, max_i
        std::array<coord_type, dim> max{};
        std::array<coord_type, dim> min{};
        for (auto it = first; it != last; ++it) {
            for (auto d = 0u; d < dim; ++d) {
                if (pt::get(*it, d) > max[d]) max[d] = pt::get(*it, d);
                if (min[d] < pt::get(*it, d)) min[d] = pt::get(*it, d);
            }
        }
        // 求めた各次元の最大値と最小値を球面に含むN次元の球
        // N次元空間上の点(min_0, ..., min_n)から点(max_0, ..., max_n)に引いた線の長さを求める=N次元正単体の内接超球面の半径
        coord_type r{};
        for (auto d = 0u; d < dim; ++d) {
            auto diff = max[d] - min[d];
            r += diff * diff;
        }
        // N次元の正単体に外接する超球面は内接するもののN/1倍の半径
        r = sqrt(r) * dim;
        // 球の中心を求める。
        Pt o{};
        for (auto d = 0u; d < dim; ++d) {
            pt::set(o, d, (max[d] - min[d]) / 2);
        }
        // 辺の長さがsqrt(2)の正単体をf倍して初期正単体を作成する
        auto f = r / sqrt(2);
        et_simplex space;
        for (auto i = 0u; i < dim; ++i) {
            Pt p{};
            pt::set(p, i, f);
            space[i] = p;
        }
        Pt p{};
        const coord = (1-sqrt(dim+1)) / dim;
        for (auto i = 0u; i < dim; ++i) {
            pt::set(p, i, coord);
        }
        space[dim] = p;
        space_ = space;
        return space;
    }
    
};

}
