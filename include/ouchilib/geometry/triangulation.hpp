#pragma once

#include <cmath>
#include <iterator>
#include <array>
#include <vector>
#include <optional>
#include "ouchilib/math/matrix.hpp"
#include "point_traits.hpp"

namespace ouchi::geometry {

namespace detail {

template<class T, size_t D>
constexpr ouchi::math::fl_matrix<T, D, 1> one() noexcept
{
    ouchi::math::fl_matrix<T, D, 1> ret{};
    for (auto i = 0ul; i < D; ++i) ret(i) = T{ 1 };
    return ret;
}

}

template<class Pt>
struct triangulation {
    static constexpr size_t dim = point_traits<Pt>::dim;
    using coord_type = typename point_traits<Pt>::coord_type;
    using id_simplex = std::array<size_t, dim + 1>;
    using et_simplex = std::array<Pt, dim + 1>;

    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};
    
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<id_simplex> operator()(const Itr first, const Itr last, return_as_idx_tag) const;
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<et_simplex>  operator()(const Itr first, const Itr last) const;

    void set_initial_state(et_simplex space) { space_ = space; }

//private:
    std::optional<et_simplex> space_;

    using pt = point_traits<Pt>;

    // N次元単体に外接する球の中心と半径の二乗を求める
    // https://img.atwikiimg.com/www7.atwiki.jp/neetubot/pub/neetubot-1.0.pdf
    static constexpr std::pair<Pt, coord_type> get_circumscribed_circle(const et_simplex& s) noexcept
    {
        using namespace ouchi::math;
        static constexpr fl_matrix<coord_type, dim, 1> one = detail::one<coord_type, dim>();
        static constexpr fl_matrix<coord_type, dim + 1, 1> onep = detail::one<coord_type, dim+1>();
        auto PtoL = [](const fl_matrix<coord_type, dim, dim + 1>& P)
            ->fl_matrix<coord_type, dim, dim>
        {
            fl_matrix<coord_type, dim, dim> L{};
            for (auto i = 0ul; i < dim; ++i) {
                for (auto j = 0ul; j < dim; ++j) {
                    L(i, j) = P(i, j+1) - P(i, 0);
                }
            }
            return L;
        };
        auto cofactor_sum_mat = [PtoL](const fl_matrix<coord_type, dim, dim + 1>& P) {
            auto L = PtoL(P);
            fl_matrix<coord_type, dim + 1, dim + 1> ret{};
            auto co = (L.transpose() * L).cofactor();
            ret(0, 0) = (one.transpose() * co * one)(0);
            auto ru = (-one).transpose() * co;
            auto lb = -co * one;
            for (auto i = 1ul; i < dim+1; ++i) ret(0, i) = ru(0, i - 1);
            for (auto i = 1ul; i < dim+1; ++i) ret(i, 0) = lb(i - 1, 0);
            for (auto i = 1ul; i < dim+1; ++i) {
                for (auto j = 1ul; j < dim+1; ++j) {
                    ret(i, j) = co(i - 1, j - 1);
                }
            }
            return ret;
        };
        fl_matrix<coord_type, dim, dim + 1> P;
        for (auto i = 0ul; i < dim; ++i) {
            for (auto j = 0ul; j < dim+1; ++j) {
                P(i, j) = pt::get(s[j], i);
            }
        }
        const auto PTP = P.transpose() * P;
        const auto co = PTP.cofactor();
        const auto den = (onep.transpose() * co * onep)(0);
        auto rvec = fl_matrix<coord_type, dim + 1, 1>{};
        for (auto i = 0ul; i < dim + 1; ++i) {
            rvec(i) = (P.column(i).transpose() * P.column(i))(0) / (coord_type)2;
        }
        auto p0 = (P * co * onep) / den + ((P * cofactor_sum_mat(P)) / den) * rvec;
        Pt o;
        for (auto i = 0ul; i < dim; ++i) {
            pt::set(o, i, p0(i));
        }
        return { o, pt::sqdistance(o, s[0]) };
    }

    template<class Itr>
    static constexpr et_simplex calc_space(Itr first, Itr last) noexcept
    {
        using std::sqrt;
        // N次元の点群について各次元の最大値と最小値を求める : min_i, max_i
        Pt max(*first);
        Pt min(*first);
        for (auto it = first; it != last; ++it) {
            for (auto d = 0u; d < dim; ++d) {
                if (pt::get(*it, d) > pt::get(max, d)) pt::set(max, d, pt::get(*it, d));
                if (pt::get(min, d) > pt::get(*it, d)) pt::set(min, d, pt::get(*it, d));
            }
        }
        // 求めた各次元の最大値と最小値を球面に含むN次元の球
        // N次元空間上の点(min_0, ..., min_n)から点(max_0, ..., max_n)に引いた線の長さを求める=N次元正単体の内接超球面の直径
        coord_type r{};
        for (auto d = 0u; d < dim; ++d) {
            auto diff = pt::get(max, d) - pt::get(min, d);
            r += diff * diff;
        }
        // N次元の正単体に外接する超球面は内接するもののN倍の半径
        r = sqrt(r) * dim;
        // 球の中心を求める。
        Pt o{};
        for (auto d = 0u; d < dim; ++d) {
            pt::set(o, d, (pt::get(max, d) - pt::get(min, d)) / 2);
        }
        // 辺の長さがsqrt(2)の正単体をf倍して初期正単体を作成する
        auto f = r / sqrt(2);
        et_simplex space;
        for (auto i = 0u; i < dim; ++i) {
            Pt p = pt::zero();
            pt::set(p, i, 1);
            space[i] = p;
        }
        // 最後の頂点
        Pt p{};
        // 正単体の重心
        Pt offset{};
        const auto coord = (1-sqrt(dim+1)) / dim;
        auto g = (coord + 1) / (coord_type)(dim + 1);
        for (auto i = 0u; i < dim; ++i) {
            pt::set(p, i, coord);
            pt::set(offset, i, g);
        }
        space[dim] = p;
        // 原点からのずれ
        offset = pt::add(o, offset);
        for (auto i = 0ul; i < dim + 1; ++i) {
            space[i] = pt::sub(space[i], offset);
            space[i] = pt::mul(space[i], (coord_type)f);
            space[i] = pt::add(space[i], o);
        }
        
        for (auto& lp : space) lp = pt::add(lp, offset);
        return space;
    }
    
};

}
