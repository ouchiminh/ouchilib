#pragma once
#include <cassert>
#include <functional>
#include <vector>
#include <string>
#include <cmath>
#include <cstdio>
#include "ouchilib/utl/step.hpp"
#include "ouchilib/utl/indexed_iterator.hpp"
#include "ouchilib/math/gf.hpp"
#include "ouchilib/result/result.hpp"
#include "../common.hpp"

namespace ouchi::crypto {

/// <summary>
/// T : 四則演算が定義される体の表現型。1 byte符号なし整数と縮小せずに相互に変換可能でなければならない。
/// </summary>
template<class T = ouchi::math::gf256<>>
class secret_sharing {
public:
    /// <summary>
    /// シャミアの秘密分散を初期化する。
    /// </summary>
    /// <param name="randgen">シェアを作成する際に必要な乱数生成器。 引数を取らずランダムなTのインスタンスを返す。</param>
    /// <param name="threshold">閾値</param>
    template<class RandomGenerator, std::enable_if_t<std::is_invocable_r_v<T, RandomGenerator>>* = nullptr>
    secret_sharing(RandomGenerator&& randgen, unsigned threshold)
        : threshold_{threshold}
        , polynominal_(threshold - 1)
        , secret_{}
    {
        assert(threshold && threshold < 255);
        for (auto& i : polynominal_)
            i = randgen();

        secret_.reserve(256);
    }

    /// <summary>
    /// シャミアの秘密分散を復号用に初期化する。
    /// </summary>
    /// <param name="threshold">閾値</param>
    /// <remarks>このコンストラクタで初期化されたインスタンスではシェアを作成することはできません</remarks>
    constexpr secret_sharing(unsigned threshold)
        : threshold_{ threshold }
        , polynominal_{}
        , secret_{}
    {}
    constexpr secret_sharing() = default;
    ~secret_sharing()
    {
        if (secret_.size())
            secure_memset(secret_.data(), 0, secret_.size());
        if (polynominal_.size())
            secure_memset(polynominal_.data(), 0, polynominal_.size() * sizeof(T));
    }

    template<class RandomGenerator, std::enable_if_t<std::is_invocable_r_v<T, RandomGenerator>>* = nullptr>
    void set_threshold(RandomGenerator&& randgen, unsigned threshold)
    {
        // GF(256)上で計算することが前提なので、閾値には0を除いた元の数以下の数しか受け付けない。
        assert(threshold && threshold < 255);
        set_threshold(threshold);
        for (auto& i : polynominal_)
            i = randgen();

        secret_.reserve(256);
    }
    void set_threshold(unsigned threshold) noexcept { threshold_ = threshold; }

    /// <summary>
    /// 秘密情報を追加する
    /// </summary>
    /// <param name="secret">秘密情報が書かれているアドレス</param>
    /// <param name="size">秘密情報のサイズ</param>
    /// <returns>秘密情報の累計サイズ</returns>
    size_t push(const void* secret, size_t size)
    {
        secret_.reserve(secret_.size() + size);
        for (auto i = 0u; i < size; ++i) {
            secret_.push_back(((std::uint8_t*)secret)[i]);
        }
    }
    void push(std::string_view secret)
    {
        secret_.reserve(secret_.size() + secret.size());
        for (auto c : secret)
            secret_.push_back((std::uint8_t)c);
    }
    /// <summary>
    /// シェアを計算する
    /// </summary>
    /// <param name="n">シェアの番号</param>
    /// <returns>シェア</returns>
    std::string get_share(unsigned n) const
    {
        if (n == 0) n = 255;
        T x{ (std::uint8_t)n };
        std::string ret;
        char buffer[3] = {};
        ret.reserve(secret_.size() * 2 + 2);
        sprintf_s(buffer, sizeof(buffer), "%02x", 0xFF & n);
        ret.append(buffer, 2ul);

        for (auto s : secret_) {
            sprintf_s(buffer, sizeof(buffer), "%02x", (unsigned)(std::uint8_t)f(x, T{ s }));
            ret.append(buffer, 2ul);
        }
        return ret;
    }
    /// <summary>
    /// 秘密情報のみリセットする。
    /// </summary>
    void reset() noexcept
    {
        secure_memset(secret_.data(), 0, secret_.size());
        secret_.clear();
    }

    /// <summary>
    /// シェアからシークレットを復元する
    /// </summary>
    /// <param name="buffer">シークレットの書き込みバッファ</param>
    /// <param name="size">バッファのサイズ</param>
    /// <param name="share">シェア</param>
    /// <returns>結果。エラー情報のみが意味を持つ。</returns>
    ouchi::result::result<std::monostate, std::string_view>
    recover_secret(void* buffer, size_t size, const std::vector<std::string>& share) const
    {
        if (share.size() < threshold_) return ouchi::result::err("too few share! "
                                                                 "at least, number of share shall be equal to threshold.");
        // テキスト表現をTの配列に変換し、solveにかける。
        const auto s_len = share.front().size() / 2;
        std::vector<T> x;
        std::vector<T> y;
        x.reserve(threshold_);
        y.reserve(threshold_);
        // init x
        for (auto sh : ouchi::step(threshold_)) {
            x.push_back(T{ (std::uint8_t)std::stoul(share[sh].substr(0, 2), nullptr, 16) });
        }
        // solve
        for (auto l : ouchi::step(1ull, s_len)) { // l byte 目の秘密情報について
            for (auto i : ouchi::step(threshold_)) { // i番目のシェアについて
                y.push_back(T{ (std::uint8_t)std::stoul(share[i].substr(l * 2, 2), nullptr, 16) });
            }
            auto r = solve(x, y);
            if (r.is_err()) return ouchi::result::err(r.unwrap_err());
            *((std::uint8_t*)buffer + l - 1) = (std::uint8_t)r.unwrap();
            y.clear();
        }
        return ouchi::result::ok(std::monostate{});
    }
#if !defined(_DEBUG)
private:
#endif
    /// <summary>
    /// 多項式を計算する
    /// </summary>
    /// <param name="nshare">シェアの番号。(多項式中のx)</param>
    /// <param name="secret">秘密情報。(多項式中の切片C)</param>
    /// <returns></returns>
    T f(T nshare, T secret) const noexcept
    {
        T res{ secret };
        for (auto i : polynominal_) {
            res += i * nshare;
            nshare *= nshare;
        }
        return res;
    }

    ouchi::result::result<T, std::string_view>
    solve(const std::vector<T>& x, const std::vector<T>& y) const
    {
        // a(x**1) + b(x**2) + .... + c = y
        assert(x.size() == y.size() && x.size() > 0);
        std::vector<T> mat(x.size() * x.size() + x.size());
        auto at = [&](auto r, auto c) mutable -> decltype(auto)
        { return mat.at(r * (x.size()+1) + c); };
        auto apply = [at, &x, &mat](auto r, auto f) mutable {
            for (auto i : ouchi::step(x.size() + 1)) {
                f(at(r, i));
            }
        };
        auto sub_row = [&](auto lhs, auto rhs, auto n) mutable {
            apply(lhs, [&, rhs, i = 0](auto& val) mutable { val -= at(rhs, i++) * n; });
        };
        auto non_zero_column = [&](auto r) {
            for (auto i : ouchi::step(x.size() + 1))
                if (at(r, i) != T{ 0 }) return i;
            return x.size();
        };

        // init mat
        auto xit = x.begin();
        for(auto r : ouchi::step(x.size())){
            for (auto c : ouchi::step(x.size())) {
                using std::pow;
                at(r, c) = pow(*xit, x.size() - c - 1);
            }
            ++xit;
        }
        for (auto [idx, y] : ouchi::indexed_iterator(y)) at(idx, x.size()) = y;
        // solve
        // mul ~[i, i] for each row
        for (auto i : ouchi::step(x.size())) {
            auto f = non_zero_column(i);
            if (f < x.size())
                apply(i, [v = T{ 1 } / at(i, f)](auto& el){ el *= v; });
            else return ouchi::result::err("あ～！いけません！不正な値は計算できません！");
            for (auto j : ouchi::step(x.size())) {
                if (j == i) continue;
                sub_row(j, i, at(j, f) / at(i, f));
            }
        }
        for (auto i : ouchi::step(x.size())) {
            if (non_zero_column(i) == x.size() - 1) {
                return ouchi::result::ok(T{ at(i, x.size()) });
            }
        }
        return ouchi::result::err("error; no solution");
    }
    unsigned threshold_;
    std::vector<T> polynominal_;
    std::vector<std::uint8_t> secret_;
};

template<class Rnd, class To>
To rnd_cvt(Rnd&& f) { return To(f()); }

}
