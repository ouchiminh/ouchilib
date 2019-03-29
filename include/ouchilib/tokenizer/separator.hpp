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
{
    Compare c;
    while (value_first != value_last && first != last) {
        if(!c(*value_first++, *first++)) return false;
    }
    return true;
}

} // detail

enum class primitive_token { word = false, separator = true };

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
    auto operator()(string_view str)  const
        ->std::pair<primitive_token, typename string_view::const_iterator>
    {
        std::pair<primitive_token, typename string_view::const_iterator> retval;
        if (auto match = is_separator(str); match != separators_.end()) {
            retval.first = primitive_token::separator;
            retval.second = std::next(str.begin(), std::min(match->size(), str.size()));
            return retval;
        }

        retval.first = primitive_token::word;
        for (retval.second = ++str.begin();
             retval.second != str.end() && is_separator(&*retval.second) == separators_.end();
             ++retval.second);
        return retval;
    }

    // cがseparatorで始まる場合マッチしたseparators_の要素を指すイテレータを返す
    [[nodiscard]]
    auto is_separator(string_view c) const {
        for (auto itr = separators_.begin();
             itr != separators_.end();
             ++itr) {
            if(detail::begin_with(c.begin(), c.end(), itr->begin(), itr->end()))
                return itr;
        }
        return separators_.end();
    }
private:
    std::vector<string> separators_;
};

}
