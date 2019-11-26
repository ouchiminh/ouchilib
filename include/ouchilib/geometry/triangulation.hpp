#pragma once

#include <cmath>
#include <cassert>
#include <iterator>
#include <array>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
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

template<size_t V>
struct hash_ids {
    using argument_type = std::array<size_t, V>;
    using result_type = size_t;

    size_t operator()(const argument_type& key) const noexcept
    {
        constexpr auto width = std::max(sizeof(result_type) * 8 / (V), (size_t)1);
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
    using id_point_set = std::unordered_set<size_t>;
    using id_face_set = std::unordered_set<id_face, detail::hash_ids<dim>>;
    using id_simplex_set = std::vector<id_simplex>;
    using id_spatial_index = std::unordered_map<size_t, id_point_set>;
public:
    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};
    
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<id_simplex> operator()(const Itr first, const Itr last, return_as_idx_tag)
    {
        id_point_set P;
        id_face_set dummy;
        id_simplex_set sigma;
        auto size = std::distance(first, last);
        P.reserve(size);
        for (auto i = 0ul; i < size; ++i) P.emplace(i);
        if (P.empty()) return {};
        dewall(first, last, P, dummy, sigma);
        return std::move(sigma);
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<et_simplex> operator()(const Itr first, const Itr last);

    template<size_t V, class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    constexpr et_simplex id_to_et(const std::array<size_t, V>& s, const Itr first, [[maybe_unused]] const Itr last = {}) const
    {
        std::array<Pt, V> ret{};
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
        id_face_set afl_alpha, afl_1, afl_2;
        id_point_set id_p_1, id_p_2;

        auto alpha = pointset_partition(first, last, P, id_p_1, id_p_2);

        if(afl.empty()){
            auto t = make_first_simplex(first, last, P, alpha);
            faces(t, afl);
            sigma.push_back(t);
        }
        for (auto& f : afl) {
            auto [intersect1, intersect2] = is_intersected(first, last, f, alpha);
            if (intersect1 && intersect2) afl_alpha.emplace(f);
            else if (intersect1) afl_1.emplace(f);
            else afl_2.emplace(f);
        }
        while (!afl_alpha.empty()) {
            auto f = *afl_alpha.begin();
            auto t = make_simplex(first, last, P, f);

            afl_alpha.erase(afl_alpha.begin());
            // t is not null
            if (t[dim] != ~(size_t)0) {
                sigma.push_back(t);
                for (auto&& g : faces(t)) {
                    if (f == g) continue;
                    auto [i1, i2] = is_intersected(first, last, g, alpha);
                    if (i1 && i2) update(g, afl_alpha);
                    else if (i1) update(g, afl_1);
                    else if (i2) update(g, afl_2);

                }
            }
        }
        if (!afl_1.empty() && !id_p_1.empty()) dewall(first, last, id_p_1, afl_1, sigma);
        if (!afl_2.empty() && !id_p_2.empty()) dewall(first, last, id_p_2, afl_2, sigma);
    }

    void update(const id_face& f, id_face_set& l)
    {
        if (l.count(f)) l.erase(f);
        else {
            l.emplace(f);
        }
    }

    template<class Itr>
    std::pair<bool, bool> is_intersected(Itr first, Itr last, const id_face& f, const alpha_t& alpha) const
    {
        bool c1{}, c2{};
        // fの任意の点についてalphaで区切られた半空間１，２のどちらにあるか判定し、
        // c1 && c2の値を返す

        for (auto idx : f) {
            if (pt::get(id_to_et(idx, first, last), alpha.first) < alpha.second) c1 = true;
            else c2 = true;
        }
        return { c1, c2 };
    }

    void faces(const id_simplex& s, id_face_set& dest) const
    {
        id_face buf;
        for (auto i = 0ul; i < s.size(); ++i) {
            unsigned d = 0;
            for (auto j = 0ul; j < s.size(); ++j) {
                if (i == j) continue;
                buf[d++] = s[j];
            }
            dest.emplace(buf);
        }
    }
    id_face_set faces(const id_simplex& s) const
    {
        id_face_set fs;
        faces(s, fs);
        return fs;
    }

    template<class Itr, size_t V>
    id_simplex make_first_simplex_impl([[maybe_unused]] Itr first, [[maybe_unused]] Itr last, [[maybe_unused]] id_point_set& P,
                                       [[maybe_unused]] const alpha_t& alpha, const std::array<size_t, V> c)
    {
        if constexpr (V == dim + 1) return c;
        else return make_first_simplex_impl(first, last, P, alpha, make_simplex<0>(first, last, P, c));
    }

    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    id_simplex make_first_simplex(Itr first, Itr last, id_point_set& P, const alpha_t& alpha)
    {
        coord_type min_dist = std::numeric_limits<coord_type>::max();
        size_t min_idx = 0;
        for (auto p : P) {
            auto d = std::abs(pt::get(id_to_et(p, first), alpha.first) - alpha.second);
            if (min_dist > d) {
                min_dist = d;
                min_idx = p;
            }
        }
        std::array<size_t, 2> segment{
            min_idx,
            minimize_where(first, last, P,
                           [pt = id_to_et(min_idx, first)](const Pt& p)
                           {return pt::sqdistance(pt, p); },
                           [&alpha, minus = pt::get(id_to_et(min_idx, first), alpha.first) < alpha.second](const Pt& p)->bool
                           {return minus ? pt::get(p, alpha.first) > alpha.second:pt::get(p, alpha.first) < alpha.second; })
        };
        return make_first_simplex_impl(first, last, P, alpha, segment);
    }
    template<class Pred, class Where, class Itr>
    size_t minimize_where(Itr first, Itr last, const id_point_set& p, Pred&& pred, Where&& where, std::invoke_result_t<Pred, Pt> initial = std::numeric_limits<std::invoke_result_t<Pred, Pt>>::max()) const
    {
        if (p.empty()) return ~(size_t)0;
        auto min = initial;
        size_t ret = ~(size_t)0;
        for (auto i : p) {
            auto pt = id_to_et(i, first, last);
            if (!std::invoke(where, pt)) continue;
            auto res = std::invoke(pred, pt);
            if (res < min) {
                min = res;
                ret = i;
            }
        }
        return ret;
    }
    template<int DD = 1, size_t V, class Itr>
    std::array<size_t, V+1> make_simplex(Itr first, Itr last, id_point_set& p, const std::array<size_t, V>& f) const
    {
        std::array<Pt, V + 1> pts;
        std::array<size_t, V + 1> id_pts;
        for (auto i = 0ul; i < V; ++i) {
            pts[i] = id_to_et(f[i], first, last);
            id_pts[i] = f[i];
        }
        if constexpr (V == dim && DD == 1) {
            auto dd = [this, &first, &last, &pts](const Pt& pt) -> coord_type {
                ouchi::math::fl_matrix<coord_type, dim, dim> planept;
                ouchi::math::fl_matrix<coord_type, dim, dim> planect;
                pts[V] = pt;
                planect = planept = PtoL(atomat(pts));
                auto [c, r] = get_circumscribed_circle(pts);
                for (auto d = 0ul; d < dim; ++d) {
                    planect(d, dim-1) = pt::get(c, d) - pt::get(pts[0], d);
                }
                bool lt0pt = ouchi::math::slow_det(planept) < 0;
                bool lt0ct = ouchi::math::slow_det(planect) < 0;
                if (lt0ct == lt0pt) return r;
                else return -r;
            };
            id_pts[V] = minimize_where(first, last, p, dd, [](...) {return true; });
        }
        else {
            id_pts[V] = minimize_where(first, last, p,
                                       [&pts, this](const Pt& pt) mutable
                                       { pts[V] = pt; return this->get_circumscribed_circle(pts).second; },
                                       [](...) {return true; });
        }
        std::sort(id_pts.begin(), id_pts.end());
        return id_pts;
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    alpha_t pointset_partition(Itr first, Itr last, id_point_set& P, id_point_set& p1, id_point_set& p2){
        // define alpha
        Pt max{id_to_et(*P.begin(), first)};
        Pt min{id_to_et(*P.begin(), first)};
        for (auto&& pid : P) {
            auto&& p = id_to_et(pid, first, last);
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
        // p1, p2に分ける
        for (auto&& pid : P) {
            (pt::get(id_to_et(pid, first, last), ret) < t ? p1 : p2).emplace(pid);
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

    template<size_t V>
    static auto PtoL(const ouchi::math::fl_matrix<coord_type, dim, V>& P)
        ->ouchi::math::fl_matrix<coord_type, dim, V-1>
    {
        ouchi::math::fl_matrix<coord_type, dim, V-1> L{};
        for (auto i = 0ul; i < dim; ++i) {
            for (auto j = 0ul; j < V-1; ++j) {
                L(i, j) = P(i, j+1) - P(i, 0);
            }
        }
        return L;
    };
    template<size_t V>
    static auto atomat(const std::array<Pt, V>& a)
        ->ouchi::math::fl_matrix<coord_type, dim, V>
    {
        ouchi::math::fl_matrix<coord_type, dim, V> ret;
        for (auto i = 0ul; i < dim; ++i) {
            for (auto j = 0ul; j < V; ++j) {
                ret(i, j) = pt::get(a[j], i);
            }
        }
        return std::move(ret);
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
    template<size_t V>
    static constexpr std::pair<Pt, coord_type> get_circumscribed_circle(const std::array<Pt, V>& s) noexcept
    {
        using namespace ouchi::math;
        static constexpr fl_matrix<coord_type, dim, 1> one = detail::one<coord_type, dim>();
        static constexpr fl_matrix<coord_type, V, 1> onep = detail::one<coord_type, V>();
        // 余因子総和行列
        auto cofactor_sum_mat = [](const fl_matrix<coord_type, dim, V>& P) 
            ->fl_matrix<coord_type, V, V>
        {
            auto L = PtoL(P);
            fl_matrix<coord_type, V, V> ret{};
            auto co = (L.transpose() * L).cofactor();
            ret(0, 0) = (detail::one<coord_type, V-1>().transpose() * co * detail::one<coord_type, V-1>())(0);
            auto ru = (-detail::one<coord_type,V-1>()).transpose() * co;
            auto lb = -co * detail::one<coord_type,V-1>();
            for (auto i = 1ul; i < V; ++i) ret(0, i) = ru(0, i - 1);
            for (auto i = 1ul; i < V; ++i) ret(i, 0) = lb(i - 1, 0);
            for (auto i = 1ul; i < V; ++i) {
                for (auto j = 1ul; j < V; ++j) {
                    ret(i, j) = co(i - 1, j - 1);
                }
            }
            return ret;
        };
        fl_matrix<coord_type, dim, V> P = atomat(s);
        const auto PTP = P.transpose() * P;
        const auto co = PTP.cofactor();
        const auto den = (onep.transpose() * co * onep)(0);
        auto rvec = fl_matrix<coord_type, V, 1>{};
        for (auto i = 0ul; i < V; ++i) {
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
