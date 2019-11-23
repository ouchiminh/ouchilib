#pragma once

#include <cmath>
#include <cassert>
#include <iterator>
#include <array>
#include <vector>
#include <optional>
#include <unordered_map>
#include <numeric>

#include "ouchilib/crypto/common.hpp" // for hash

#include "ouchilib/data_structure/tree.hpp"
#include "ouchilib/math/matrix.hpp"
#include "point_traits.hpp"

namespace ouchi::geometry {

namespace detail {

template<class Int>
constexpr Int bits_msb(Int v)
{
    using uint = std::make_unsigned_t<Int>;
    constexpr uint t = ~((~(uint)0) >> 1);
    uint buf = v;
    for (auto i = 0ul; i < sizeof(uint) * 8; ++i) {
        auto msb = buf & (t >> i);
        if (msb) return msb;
    }
    return 0;
}

template<class T, size_t D>
constexpr ouchi::math::fl_matrix<T, D, 1> one() noexcept
{
    ouchi::math::fl_matrix<T, D, 1> ret{};
    for (auto i = 0ul; i < D; ++i) ret(i) = T{ 1 };
    return ret;
}

template<size_t Dim>
struct hash_id_simplex {
    using argument_type = std::array<size_t, Dim + 1>;
    using result_type = size_t;

    size_t operator()(const argument_type& key) const noexcept
    {
        constexpr auto width = std::max(sizeof(result_type) * 8 / (Dim + 1), (size_t)1);
        result_type res{};
        for (auto i = 0ul; i < key.size(); ++i) {
            res ^= ouchi::crypto::rotl(key[i], (width * i) & (sizeof(size_t) * 8 - 1));
        }
        return res;
    }
};

constexpr size_t fact(size_t i)
{
    size_t ret = 1;
    while (i > 1) ret *= i--;
    return ret;
}

template<size_t C, size_t ...I>
constexpr std::array<size_t, C> root_id_impl(std::index_sequence<I...>)
{
    constexpr size_t ff = ~(size_t)0;
    std::array<size_t, C> ret;
    ((ret[I] = ff - I), ...);
    return ret;
}

template<size_t C>
constexpr std::array<size_t, C> root_id()
{
    return root_id_impl<C>(std::make_index_sequence<C>{});
}

}

template<class Pt>
struct triangulation {
    static constexpr size_t dim = point_traits<Pt>::dim;
    using coord_type = typename point_traits<Pt>::coord_type;
    using id_simplex = std::array<size_t, dim + 1>;
    using et_simplex = std::array<Pt, dim + 1>;
    using id_face = std::array<size_t, dim>;
    using et_face = std::array<Pt, dim>;
private:
    using pt = point_traits<Pt>;

    using alpha_t = std::pair<size_t, coord_type>;
    using id_point_set = std::vector<size_t>;
    using id_face_set = std::vector<id_face>;
    using id_simplex_set = std::vector<id_simplex>;
    using id_spatial_index = std::unordered_map<size_t, id_point_set>;
public:
    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};
    
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<id_simplex> operator()(const Itr first, const Itr last, return_as_idx_tag)
    {
        id_point_set P(std::distance(first, last));
        id_face_set dummy;
        id_simplex_set sigma;
        std::iota(P.begin(), P.end(), 0);
        index(first, last, max, min);

        return {};
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<et_simplex> operator()(const Itr first, const Itr last);

    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    constexpr et_simplex id_to_et(const id_simplex& s, const Itr first, [[maybe_unused]] const Itr last = {}) const
    {
        et_simplex ret{};
        for (auto i = 0ul; i < s.size(); ++i) {
            ret[i] = id_to_et(s[i], first, last);
        }
        return ret;
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    constexpr Pt id_to_et(size_t id, const Itr first, [[maybe_unused]] const Itr last = {}) const
    {
            return *std::next(first, id);
    }


private:
    Pt cell_width_;
    Pt cell_min_, cell_max_;
    id_spatial_index spatial_index_;

    std::array<unsigned, dim> get_cell(const Pt& p) const noexcept
    {
        auto diff = pt::sub(p, cell_min_);
        std::array<unsigned, dim> rawidx{};
        for (auto d = 0ul; d < dim; ++d) {
            rawidx[d] = (unsigned)std::floor(pt::get(diff, d) / pt::get(cell_width_, d));
        }
        return rawidx;
    }
    size_t hash_index(const std::array<unsigned, dim>& index)
    {
        size_t hash{};
        constexpr auto d_bit_width = (sizeof(size_t)*8 / dim);
        constexpr auto d_limit = (size_t)1 << d_bit_width;
        for (auto d = 0ul; d < dim; ++d) {
            hash |= (index[d] & (d_limit - 1)) << (d_bit_width * d);
        }
        return hash;
    }

    template<class Itr>
    void dewall(Itr first, Itr last, id_point_set& P, id_face_set& afl, id_simplex_set& sigma)
    {
        std::vector<id_face> afl_alpha, afl_1, afl_2;
        std::pair<size_t, coord_type> alpha; // .first次元について.secondを閾値として区切る
        id_point_set id_p_1, id_p_2;
        id_simplex t;
        id_face f;

        auto alpha = pointset_partition(first, last, id_p_1, id_p_2);

        if(afl.size() == 0){

        }
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    id_simplex make_first_simplex(Itr first, Itr last, const id_point_set& P, const alpha_t& alpha) const
    {

    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    alpha_t pointset_partition(Itr first, Itr last, id_point_set& p1, id_point_set& p2){
        // define alpha
        auto size = std::distance(first, last);
        Pt max{*first};
        Pt min{*first};
        for (auto itr = first; itr != last; ++itr) {
            auto& p = *itr;
            for (auto d = 0ul; d < dim; ++d) {
                auto c = pt::get(p, d);
                if (pt::get(max, d) < c) pt::set(max, d, c);
                else if (pt::get(min, d) > c) pt::set(min, d, c);
            }
        }
        coord_type diff = 0;
        size_t ret = 0;
        for (auto d = 0ul; d < dim; ++d) {
            auto db = pt::get(max, d) - pt::get(min, d);
            if (diff < db) {
                diff = db;
                ret = d;
            }
        }
        auto t = pt::get(min, ret) + (diff / (coord_type)2);
        size_t idx = 0;
        // p1, p2に分ける
        for (auto itr = first; itr != last; ++itr) {
            (pt::get(*itr, ret) < t ? p1 : p2).push_back(idx++);
        }
        return { ret, t };
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    void index(Itr first, Itr last, const Pt& max, const Pt& min)
    {
        Pt intmax, intmin;
        const auto cell_cnt_d = std::min((size_t)1 << (sizeof(size_t)*8 / dim),
                                         std::max<size_t>(detail::bits_msb(std::distance(first, last) >> std::min<size_t>(dim, 63)), 1));
        for (auto d = 0ul; d < dim; ++d) {
            pt::set(cell_max_, d, std::ceil(pt::get(max, d)));
            pt::set(cell_min_, d, std::floor(pt::get(min, d)));
            pt::set(cell_width_, d, (pt::get(cell_max_, d) - pt::get(cell_min_, d)) / cell_cnt_d);
        }
        for (auto itr = first; itr != last; ++itr) {
            spatial_index_[hash_index(get_cell(*itr))].push_back(std::distance(itr, first));
        }
    }

    // d次元単体の体積
    static constexpr coord_type volume(const et_simplex& s) noexcept
    {
        using namespace ouchi::math;
        using std::abs;
        fl_matrix<coord_type, dim + 1, dim + 1> a;
        constexpr auto den = detail::fact(dim);
        // 列ベクトルがsの各頂点の座標  .append(1)
        for (auto i = 0ul; i < dim + 1; ++i) {
            // 行ベクトル
            for (auto j = 0ul; j < dim + 1; ++j) {
                a(i, j) = (i != dim ? pt::get(s[j], i) : 1);
            }
        }
        return abs(slow_det(a)) / den;
    }

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
        // 余因子総和行列
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
