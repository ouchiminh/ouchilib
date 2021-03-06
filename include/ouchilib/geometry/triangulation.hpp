﻿#pragma once

#include <cmath>
#include <cassert>
#include <iterator>
#include <array>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <numeric>
#include <future>
#include <mutex>

#include "ouchilib/crypto/common.hpp" // for hash
#include "ouchilib/crypto/algorithm/aes.hpp" // for hash

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

template<class Pt>
constexpr Pt one_pt()
{
    using pt = point_traits<Pt>;
    Pt p{};
    for (auto d = 0ul; d < pt::dim; ++d) {
        pt::set(p, d, pt::coord_type(1));
    }
    return p;
}

template<class T, size_t V>
struct facet {
    std::array<T, V> vertexes;
    std::optional<T> opposite;

    facet() : vertexes{}, opposite{ std::nullopt } {}
    facet(const std::array<T, V>& v)
        : vertexes{}
        , opposite(std::nullopt)
    {
        for (auto i = 0ul; i < v.size(); ++i) vertexes[i] = v[i];
    }
    facet(const std::array<T, V>& v, T o)
        : facet(v)
    {
        opposite = o;
    }
    operator std::array<T, V>& () { return vertexes; }
    operator const std::array<T, V>& () const { return vertexes; }
    auto begin() const { return vertexes.cbegin(); }
    auto begin() { return vertexes.begin(); }
    auto end() const { return vertexes.cend(); }
    auto end() { return vertexes.end(); }
    decltype(auto) operator[](size_t i) { return vertexes[i]; }
    decltype(auto) operator[](size_t i) const { return vertexes[i]; }
    friend bool operator<(const facet& a, const facet& b) { return a.vertexes < b.vertexes; }
    friend bool operator>(const facet& a, const facet& b) { return b < a; }
    friend bool operator==(const facet& a, const facet& b) { return !(a < b) && !(a > b); }
    friend bool operator!=(const facet& a, const facet& b) { return !(a == b); }
};

template<class Int, size_t V>
struct hash_ids {
    using argument_type = std::array<Int, V>;
    using result_type = size_t;

    size_t operator()(const argument_type& key) const noexcept
    {
        using namespace ouchi::crypto;
        constexpr auto width = std::max(sizeof(result_type) * 8 / (V), (size_t)1);
        result_type res{};
        for (auto i = 0ul; i < key.size(); ++i) {
            //res ^= ouchi::crypto::rotl(key[i], (width * i) & (sizeof(size_t) * 8 - 1));
            res ^= rotl((size_t)cvt(key[i]), (i + 6) & (sizeof(Int) * 8 - 1));
            res ^= rotr(res, std::max<unsigned>(sizeof(Int) * 4 / V, 1));
        }
        return res;
    }
    template<class Int>
    static constexpr Int cvt(Int v) noexcept
    {
        unsigned char m[sizeof(Int)]{};
        ouchi::crypto::detail::unpack(v, m);
        for (auto& i : m) i = ouchi::crypto::aes128::subchar(i);
        return ouchi::crypto::detail::pack<Int>(m);
    }
};

constexpr size_t fact(size_t i)
{
    size_t ret = 1;
    while (i > 1) ret *= i--;
    return ret;
}

}

template<class Pt, unsigned Parallel = 0>
struct triangulation {
    static constexpr size_t dim = point_traits<Pt>::dim;
    using coord_type = typename point_traits<Pt>::coord_type;
    using id_simplex = std::array<size_t, dim + 1>;
    using et_simplex = std::array<Pt, dim + 1>;
    using id_face = detail::facet<size_t, dim>;
    using et_face = detail::facet<Pt, dim>;
private:
    using pt = point_traits<Pt>;

    using alpha_t = std::pair<size_t, coord_type>;
    using id_point_set = std::unordered_set<size_t>;
    using id_face_set = std::vector<id_face>;
    using id_simplex_set = std::vector<id_simplex>;
    using id_spatial_index = std::unordered_map<std::array<long, dim>, id_point_set, detail::hash_ids<long, dim>>;
    using visited_cell_set = std::unordered_set<std::array<long, dim>, detail::hash_ids<long, dim>>;
public:
    struct return_as_idx_tag {};
    static constexpr return_as_idx_tag return_as_idx{};

    triangulation()
        : triangulation(std::numeric_limits<coord_type>::epsilon())
    {}
    triangulation(coord_type epsilon)
        : epsilon(epsilon)
    {}
    
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<id_simplex> operator()(const Itr first, const Itr last, return_as_idx_tag)
    {
        id_point_set P;
        id_face_set dummy;
        id_simplex_set sigma;
        auto size = std::distance(first, last);
        for (auto i = 0ul; i < size; ++i) P.emplace(i);
        if (size < dim + 1) return {};
        make_spatial_index(first, last);
        dewall(first, last, P, dummy, sigma);
        spatial_index_.clear();
        return std::move(sigma);
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    std::vector<et_simplex> operator()(const Itr first, const Itr last);

    template<size_t V, class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    static constexpr std::array<Pt, V> id_to_et(const std::array<size_t, V>& s, const Itr first, [[maybe_unused]] const Itr last = {})
    {
        std::array<Pt, V> ret{};
        for (auto i = 0ul; i < s.size(); ++i) {
            ret[i] = id_to_et(s[i], first, last);
        }
        return ret;
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    static constexpr Pt id_to_et(size_t id, const Itr first, [[maybe_unused]] const Itr last = {})
    {
            return *std::next(first, id);
    }

private:

    coord_type epsilon;
    Pt cell_width_;
    Pt cell_min_, cell_max_;
    std::array<long, dim> cell_cnt_max_;
    id_spatial_index spatial_index_;
    std::mutex mt_sigma;

    std::array<long, dim> get_cell(const Pt& p) const noexcept
    {
        namespace og = ouchi::geometry;
        using std::floor;
        auto diff = og::sub(p, cell_min_);
        std::array<long, dim> rawidx{};
        for (auto d = 0ul; d < dim; ++d) {
            rawidx[d] = (long)floor(pt::get(diff, d) / pt::get(cell_width_, d));
        }
        return rawidx;
    }
    template<class Itr>
    void make_spatial_index(Itr first, Itr last)
    {
        using std::ceil, std::floor;
        namespace og = ouchi::geometry;
        Pt min = *first; // 各次元ごとの最大値
        Pt max = *first;
        for (auto d = 0u; d < dim; ++d) {
            pt::set(min, d, floor(pt::get(min, d)));
            pt::set(max, d, ceil(pt::get(max, d)));
        }
        for (auto itr = first; itr != last; ++itr) {
            auto& p = *itr;
            for (auto d = 0ul; d < dim; ++d) {
                auto c = pt::get(p, d);
                if (pt::get(max, d) < c) pt::set(max, d, ceil(c));
                else if (pt::get(min, d) > c) pt::set(min, d, floor(c));
            }
        }
        Pt diff = og::sub(max, min);
        coord_type diff_min = pt::get(diff, 0);
        for (auto i = 1u; i < dim; ++i) if (diff_min > pt::get(diff, i)) diff_min = pt::get(diff, i);
        auto den = std::max<size_t>(detail::bits_msb((long)std::pow(std::distance(first, last), 1.0 / dim)) >> 2, 1ul);
        for (auto d = 0ul; d < dim; ++d) {
            //pt::set(diff, d, pt::get(diff, d) / den);
            pt::set(diff, d, diff_min / den);
        }
        cell_min_ = min;
        cell_max_ = max;
        cell_width_ = diff;
        cell_cnt_max_ = get_cell(max);
        
        size_t i{};
        for (auto itr = first; itr != last; ++itr) {
            spatial_index_[get_cell(*itr)].emplace(i++);
        }
        spatial_index_.rehash(spatial_index_.size());
    }
    template<size_t D, class F, class Where>
    auto for_cell_minimize_impl(visited_cell_set& si_visited, std::array<long, dim> cell, long r,
                                size_t& idx, std::optional<std::invoke_result_t<F, size_t>>& res,
                                const id_point_set& P, F&& pred, Where&& w,
                                bool is_edge = false) const
        -> void
    {
        constexpr auto invalid = ~(size_t)0;
        auto [f, l] = std::make_pair(std::clamp<long>(cell[D] - r, 0, cell_cnt_max_[D]),
                                     std::clamp<long>(cell[D] + r, 0, cell_cnt_max_[D]));
        if constexpr (D == dim - 1) {
            auto in = [&P](size_t id) {
                return P.count(id) > 0;
            };
            for (cell[D] = f; cell[D] <= l; ++cell[D]) {
                if (!(is_edge || cell[D] == f || cell[D] == l) ||
                    !spatial_index_.count(cell) ||
                    si_visited.count(cell)) continue;
                si_visited.insert(cell);
                auto [tmpidx, tmpr] = minimize_where(spatial_index_.at(cell),
                                                     pred,
                                                     [&w, &in](size_t id) {return in(id) && std::invoke(w, id); });
                if (tmpidx != invalid && (!res.has_value() || res.value() > tmpr.value()))
                { res = tmpr; idx = tmpidx; }
            }
        }
        else {
            for (cell[D] = f; cell[D] <= l; ++cell[D]) {
                for_cell_minimize_impl<D + 1>(si_visited, cell, r, idx, res, P, pred, w,
                                              is_edge || cell[D] == f || cell[D] == l);
            }
        }
    }
    template<class F, class Where>
    auto for_cell_minimize(visited_cell_set& si_visited, const Pt& c, coord_type radius, const id_point_set& P,
                             F&& pred, Where&& where) const
        ->std::pair<size_t, std::optional<std::invoke_result_t<F, size_t>>>
    {
        constexpr size_t invalid = ~(size_t)0;
        auto cell = get_cell(c);
        const auto dcell_cnt = (pt::get(cell_max_, 0) - pt::get(cell_min_, 0)) / pt::get(cell_width_, 0);
        size_t idx = invalid;
        std::optional<std::invoke_result_t<F, size_t>> result;
        unsigned radii = 0;
        if (radius > 0) {
            for (auto d = 0ul; d < dim; ++d) {
                auto b = c;
                pt::set(b, d, pt::get(c, d) + radius);
                unsigned rb = get_cell(b)[d] - cell[d];
                if (rb > radii) radii = rb;
                pt::set(b, d, pt::get(c, d) - radius);
                rb = cell[d] - get_cell(b)[d];
                if (rb > radii) radii = rb;
            }
        }// else return minimize_where(P, pred, where);
        auto r = 0ul;
        do {
            for_cell_minimize_impl<0>(si_visited, cell, r++, idx, result, P,
                                      pred, where);
        } while (si_visited.size() != spatial_index_.size() &&
                 (radius > 0 ? r <= radii : idx == invalid));
        return { idx, result };
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
            mt_sigma.lock();
            sigma.push_back(t);
            mt_sigma.unlock();
        }
        for (auto& f : afl) {
            auto [intersect1, intersect2] = is_intersected(first, last, f, alpha);
            if (intersect1 && intersect2) afl_alpha.push_back(f);
            else if (intersect1) afl_1.push_back(f);
            else afl_2.push_back(f);
        }
        while (!afl_alpha.empty()) {
            auto f = afl_alpha.back();
            auto t = make_simplex(first, last, P, f);

            afl_alpha.pop_back();
            // t is not null
            if (t[dim] != ~(size_t)0) {
                mt_sigma.lock();
                sigma.push_back(t);
                mt_sigma.unlock();
                for (auto&& g : faces(t)) {
                    if (f == g) continue;
                    auto [i1, i2] = is_intersected(first, last, g, alpha);
                    if (i1 && i2) update(g, afl_alpha);
                    else if (i1) update(g, afl_1);
                    else if (i2) update(g, afl_2);
                }
            }
        }
        if constexpr (Parallel > 0) {
            constexpr auto normal = std::launch::async | std::launch::deferred;
            std::future<void> t[2];
            if (!afl_1.empty())
                t[0] = std::async((id_p_1.size() > Parallel ? normal : std::launch::deferred),
                                  [this, &id_p_1, &afl_1, &sigma, &first, &last]()
                                  {dewall(first, last, id_p_1, afl_1, sigma); });
            if (!afl_2.empty())
                t[1] = std::async((id_p_2.size() > Parallel ? normal : std::launch::deferred),
                                  [this, &id_p_2, &afl_2, &sigma, &first, &last]()
                                  {dewall(first, last, id_p_2, afl_2, sigma); });
            t[0].get();
            t[1].get();
        }
        else {
            if (!afl_1.empty()) dewall(first, last, id_p_1, afl_1, sigma);
            if (!afl_2.empty()) dewall(first, last, id_p_2, afl_2, sigma);
        }
    }

    void update(const id_face& f, id_face_set& l) const
    {
        if (auto itr = std::find(l.begin(), l.end(), f); itr != l.end()) l.erase(itr);
        else {
            l.push_back(f);
        }
    }

    template<class Itr>
    std::pair<bool, bool> is_intersected(Itr first, Itr last, const id_face& f, const alpha_t& alpha) const
    {
        bool c1{ false }, c2{ false };
        // fの任意の点についてalphaで区切られた半空間１，２のどちらにあるか判定し、

        for (auto idx : f) {
            if (is_lower_space(id_to_et(idx, first, last), alpha)) c1 = true;
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
                if (i == j) {
                    buf.opposite = s[j];
                    continue;
                }
                buf[d++] = s[j];
            }
            dest.push_back(buf);
        }
    }
    std::array<id_face, dim + 1> faces(const id_simplex& s) const
    {
        std::array<id_face, dim + 1> fs;
        id_face buf;
        for (auto i = 0ul; i < s.size(); ++i) {
            unsigned d = 0;
            for (auto j = 0ul; j < s.size(); ++j) {
                if (i == j) {
                    buf.opposite = s[j];
                    continue;
                }
                buf[d++] = s[j];
            }
            fs[i] = buf;
        }
        return fs;
    }

    template<class Itr, size_t V>
    id_simplex make_first_simplex_impl([[maybe_unused]] Itr first, [[maybe_unused]] Itr last, [[maybe_unused]] id_point_set& P,
                                       [[maybe_unused]] const alpha_t& alpha, const std::array<size_t, V> c) const
    {
        if constexpr (V == dim + 1) return c;
        else return make_first_simplex_impl(first, last, P, alpha, make_simplex<1>(first, last, P, detail::facet<size_t, V>(c)));
    }

    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    id_simplex make_first_simplex(Itr first, Itr last, id_point_set& P, const alpha_t& alpha) const
    {
        using std::abs;
        namespace og = ouchi::geometry;
        coord_type min_dist = std::numeric_limits<coord_type>::max();
        size_t min_idx = 0;
        for (auto p : P) {
            auto d = abs(pt::get(id_to_et(p, first), alpha.first) - alpha.second);
            if (min_dist > d) {
                min_dist = d;
                min_idx = p;
            }
        }
        std::array<size_t, 2> segment{
            min_idx,
            minimize_where(P,
                           [pt = id_to_et(min_idx, first), &first, this](size_t pid)
                           {return og::sqrdistance(pt, id_to_et(pid, first)); },
                           [&first, &alpha, low = is_lower_space(id_to_et(min_idx, first), alpha)](size_t pid)->bool
                           {bool p = is_lower_space(id_to_et(pid, first), alpha); return low != p; })
            .first
        };
        return make_first_simplex_impl(first, last, P, alpha, segment);
    }
    template<class Pred, class Where>
    auto minimize_where(const id_point_set& p, Pred&& pred, Where&& where) const
        -> std::pair<size_t, std::optional<std::invoke_result_t<Pred, size_t>>>
    {
        if (p.empty()) return { ~(size_t)0, {} };
        auto itr = p.begin();
        std::optional<std::invoke_result_t<Pred, size_t>> min;
        size_t ret = ~(size_t)0;
        for (; itr != p.end(); ++itr) {
            auto i = *itr;
            if (!std::invoke(where, i)) continue;
            auto res = std::invoke(pred, i);
            if (!min.has_value() || res < min.value()) {
                min = res;
                ret = i;
            }
        }
        return { ret, min };
    }
    template<int DD = 1, size_t V, class Itr>
    std::array<size_t, V+1> make_simplex(Itr first, Itr last, const id_point_set& p, const detail::facet<size_t, V>& f) const
    {
        constexpr auto invalid_idx = ~(size_t)0;
        std::array<Pt, V + 1> pts;
        std::array<size_t, V + 1> id_pts;
        for (auto i = 0ul; i < V; ++i) {
            pts[i] = id_to_et(f[i], first, last);
            id_pts[i] = f[i];
        }
        if constexpr (V == dim && DD == 1) {
            int pth;
            using std::sqrt;
            auto cc = std::make_pair(get_circumscribed_circle(id_to_et(f.vertexes, first)).first,
                                     (coord_type)-1.0);
            size_t visited;
            thread_local visited_cell_set si_visited(spatial_index_.bucket_count());
            //auto res = std::make_pair(std::numeric_limits<coord_type>::max(),
            //                          std::numeric_limits<coord_type>::max());
            coord_type res = std::numeric_limits<coord_type>::max();
            id_pts[V] = invalid_idx;
            auto halfspace_pt = [epsilon = this->epsilon, &pts](const Pt& p) {
                using std::abs;
                pts[V] = p;
                auto r = ouchi::math::det(PtoL(atomat(pts)));
                return abs(r) <= epsilon ? 0 
                    : r < 0 ? -1
                    : 1;
            };
            auto halfspace_id = [this, &first, &halfspace_pt](size_t id) {
                return halfspace_pt(id_to_et(id, first));
            };
            auto dd = [this, &halfspace_pt,&halfspace_id, &first, &pts, &pth](size_t id) {
                using std::isnan;
                pts[V] = id_to_et(id, first);
                auto [c, r] = get_circumscribed_circle(pts);
                if (isnan(r)) return std::numeric_limits<coord_type>::max();
                auto hspt = pth;
                auto hsct = halfspace_pt(c);
                return hsct == hspt ? r : -r;
            };
            auto where = [this, &f, &halfspace_id, &first, &pth](size_t id) -> bool
            {
                pth = halfspace_id(id);
                if (!f.opposite.has_value()) return true;
                auto foh = halfspace_id(f.opposite.value());
                return foh != pth && pth != 0;
            };
            do {
                visited = si_visited.size();
                auto local_res = for_cell_minimize(si_visited, cc.first, cc.second, p, dd, where);
                if (local_res.first != invalid_idx) {
                    if (local_res.second.value() < res) {
                        res = local_res.second.value(); id_pts[V] = local_res.first;
                        cc = get_circumscribed_circle(id_to_et(id_pts, first));
                        cc.second = sqrt(cc.second);
                    } else break;
                } else break;
            } while (visited != si_visited.size());

            //auto [miniidx, miniresult] = minimize_where(p, dd, where);
            //if (miniidx != invalid_idx && miniresult.value() != res)
            //    throw std::runtime_error("error triangulation");
            si_visited.clear();
            //id_pts[V] = minimize_where(p, dd, where).first;
        }
        else {
            id_pts[V] = minimize_where(p,
                                       [&pts, &first, this](size_t id) mutable
                                       { pts[V] = id_to_et(id, first); return get_circumscribed_circle(pts).second; },
                                       [](...) {return true; }).first;
        }
        std::sort(id_pts.begin(), id_pts.end());
        return id_pts;
    }
    static inline bool is_lower_space(const Pt& pt, const alpha_t& alpha)
    {
        return pt::get(pt, alpha.first) < alpha.second;
    }
    template<class Itr, std::enable_if_t<std::is_same_v<typename std::iterator_traits<Itr>::value_type, Pt>, int> = 0>
    alpha_t pointset_partition(Itr first, Itr last, id_point_set& P, id_point_set& p1, id_point_set& p2) const
    {
        // define alpha
        Pt max{id_to_et(*P.begin(), first)};
        Pt min{id_to_et(*P.begin(), first)};

        for (auto pid : P) {
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
        alpha_t alpha{ ret, t };
        // p1, p2に分ける
        p1.reserve(P.size() / 2);
        p2.reserve(P.size() / 2);
        for (auto&& pid : P) {
            (is_lower_space(id_to_et(pid, first), alpha) ? p1 : p2).emplace(pid);
        }
        return alpha;
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
    static constexpr coord_type volume(const std::array<Pt, dim+1>& s) noexcept
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
        return abs(det(a)) / den;
    }

    // N次元単体に外接する球の中心と半径の二乗を求める
    // https://img.atwikiimg.com/www7.atwiki.jp/neetubot/pub/neetubot-1.0.pdf
    template<size_t V>
    constexpr std::pair<Pt, coord_type> get_circumscribed_circle(const std::array<Pt, V>& s) const noexcept
    {
        using namespace ouchi::math;
        namespace og = ouchi::geometry;
        static const fl_matrix<coord_type, dim, 1> one = detail::one<coord_type, dim>();
        static const fl_matrix<coord_type, V, 1> onep = detail::one<coord_type, V>();
        if constexpr (V == 2) {
            auto sum = og::add(s[0], s[1]);
            auto c = og::mul(sum, (coord_type)0.5);
            return { c, og::sqrdistance(s[0], s[1]) };
        } else {
        // 行列要素の総和
            auto sum_mat = [](auto&& mat)->coord_type {
                coord_type sum{};
                for (auto i = 0ul; i < mat.total_size(); ++i) {
                        sum += mat(i);
                }
                return sum;
            };
            // 余因子総和行列
            auto cofactor_sum_mat = [&sum_mat](const fl_matrix<coord_type, dim, V>& P)
                ->fl_matrix<coord_type, V, V>
            {
                auto L = PtoL(P);
                fl_matrix<coord_type, V, V> ret{};
                auto co = (L.transpose() * L).cofactor_matrix();
                //ret(0, 0) = (detail::one<coord_type, V-1>().transpose() * co * detail::one<coord_type, V-1>())(0);
                ret(0, 0) = sum_mat(co);
                auto ru = (-detail::one<coord_type, V-1>()).transpose() * co;
                auto lb = -co * detail::one<coord_type, V-1>();
                for (auto i = 1ul; i < V; ++i) ret(0, i) = ru(0, i - 1);
                for (auto i = 1ul; i < V; ++i) ret(i, 0) = lb(i - 1, 0);
                for (auto i = 1ul; i < V; ++i) {
                    for (auto j = 1ul; j < V; ++j) {
                        ret(i, j) = co(i - 1, j - 1);
                    }
                }
                return ret;
            };
            using std::abs;
            fl_matrix<coord_type, dim, V> P = atomat(s);
            const auto PTP = P.transpose() * P;
            const auto co = PTP.cofactor_matrix();
            //const auto den = (onep.transpose() * co * onep)(0);
            const auto den = sum_mat(co);
            auto rvec = fl_matrix<coord_type, V, 1>{};
            for (auto i = 0ul; i < V; ++i) {
                rvec(i) = (P.column(i).transpose() * P.column(i))(0) / (coord_type)2;
            }
            auto po = ((P * co * onep) + ((P * cofactor_sum_mat(P))) * rvec);
            if (abs(den) <= epsilon) return { Pt{}, std::numeric_limits<coord_type>::signaling_NaN() };
            po = po / den;
            Pt o;
            for (auto i = 0ul; i < dim; ++i) {
                pt::set(o, i, po(i));
            }
            return { o, og::sqrdistance(o, s[0]) };
        }
    }

};

}
