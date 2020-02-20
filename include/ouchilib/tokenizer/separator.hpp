#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

namespace ouchi::tokenizer {

namespace detail {

template<class InputIterator,
    class InputIterator2,
    class Compare = std::equal_to<>>
constexpr bool begin_with(InputIterator first, InputIterator last,
                          InputIterator2 value_first, InputIterator2 value_last)
    noexcept(noexcept(Compare{}(*first, *value_first)))
{
    Compare c;
    while (value_first != value_last && first != last) {
        if(!c(*value_first++, *first++)) return false;
    }
    return true;
}

} // detail

enum class primitive_token : std::uint32_t { word = 0xFFFF'FFFF, separator = 0xFFFF'FFFE };

template<class CharT>
class separator {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;

    /// <summary>
    /// セパレータとして扱う文字が全て1符号単位で表せる場合
    /// </summary>
    /// <param name="separators"></param>
    explicit separator(string_view separators)
        : seplen_max_{1}
    {
        for (auto&& c : separators) {
            separators_.emplace_back(1, c);
        }
    }
    separator(std::in_place_t, std::initializer_list<string> separators)
        : separators_{ separators }
    {
        std::sort(separators_.begin(), separators_.end(),
                  [](const auto& a, const auto& b) {return a.size() > b.size(); });
        seplen_max_ = separators_.size() ? separators_.front().size() : 0;
    }
    separator(std::initializer_list<CharT> separators)
        : seplen_max_{1}
    {
        for (auto&& c : separators) {
            separators_.emplace_back(1, c);
        }
    }
    separator(string_view separators, std::initializer_list<string> string_separators)
        : separator{ std::in_place, string_separators }
    {
        for (auto&& c : separators) {
            separators_.emplace_back(1, c);
        }
        std::sort(separators_.begin(), separators_.end(),
                  [](const auto& a, const auto& b) {return a.size() > b.size(); });
        seplen_max_ = separators_.size() ? separators_.front().size() : 0;
    }

    /// <summary>
    /// strの最初の位置から始まる文字列がどの種類のトークンか調べる
    /// [str.begin(), retval.second)がトークン
    /// </summary>
    /// <returns>トークン, トークンの終わりの位置</returns>
    [[nodiscard]]
    auto operator()(string_view str)  const noexcept
        ->std::pair<primitive_token, typename string_view::const_iterator>
    {
        auto [p, s] = find_separator(str);
        // if str begins with separator,
        if (p == str.begin()) return std::make_pair(primitive_token::separator, p + s);
        // else if str does not begins with separator, str begins with word.
        else return std::make_pair(primitive_token::word, p);
    }

    // cがseparatorで始まる場合マッチしたseparators_の要素を指すイテレータを返す
    [[nodiscard]]
    auto is_separator(string_view c) const noexcept {
        for (auto itr = separators_.begin();
             itr != separators_.end();
             ++itr) {
            if(detail::begin_with(c.begin(), c.end(), itr->begin(), itr->end()))
                return itr;
        }
        return separators_.end();
    }
    // cで最初に現れるいずれかseparators_と一致する部分の最初のイテレータとマッチしたseparatorの文字数を返す
    [[nodiscard]]
    auto find_separator(string_view c) const
    {
        auto retcond = [c](auto p, auto cnt, const auto& cur_ret) {
            return (size_t)p <= (size_t)std::distance(c.begin(), cur_ret.first) &&
                cnt > cur_ret.second;
        };
        constexpr size_t invalid = ~(size_t)0;
        // メモリ確保を減らすため
        thread_local std::vector<size_t> cnts;
        auto retval = std::make_pair(c.end(), (size_t)0);
        size_t countdown = invalid;
        cnts.clear();
        cnts.resize(separators_.size(), 0);
        // cを1文字ずつ見ていき、cのi文字目を見ている時点でj番目のセパレータに一致している文字数をcntsに格納
        // cntsがセパレータの文字数と等しくなったらリターン
        for (auto i = 0ul; i < c.size(); ++i) {
            auto l = c[i];
            for (auto j = 0ul; j < separators_.size(); ++j) {
                if (l == separators_[j][cnts[j]]) {
                    auto cnt = ++(cnts[j]);
                    auto p = i - cnt + 1;
                    if (cnt == separators_[j].size() && retcond(p, cnt, retval)) {
                        retval = std::make_pair(c.begin() + p, cnt);
                        if (countdown == invalid) countdown = seplen_max_;
                    }
                } else cnts[j] = 0;
            }
            if (countdown != invalid) --countdown;
            if (countdown == 0) return retval;
        }
        return retval;
    }
private:
    std::vector<string> separators_;
    size_t seplen_max_;
};

}
