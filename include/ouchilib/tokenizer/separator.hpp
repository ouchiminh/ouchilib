﻿#pragma once
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
    }
    separator(std::initializer_list<CharT> separators)
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
    // cで最初に現れるいずれかseparators_と一致する部分の最初のイテレータとマッチした文字数を返す
    [[nodiscard]]
    auto find_separator(string_view c) const noexcept
    {
        auto p = c.end();
        size_t msize = 0;
        for (auto&& s : separators_) {
            auto d = c.find(s.data());
            if (d == string_view::npos) continue;
            auto r = d + c.begin();
            if (d == 0) return std::make_pair(r, s.size());
            else if (d < std::distance(c.begin(), p)) p = r, msize = s.size();
        }
        return std::make_pair(p, msize);
    }
private:
    std::vector<string> separators_;
};

}
