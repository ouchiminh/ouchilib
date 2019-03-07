#pragma once
#include <set>
#include <string>
#include <string_view>

namespace ouchi::tokenizer {

enum class token { word = false, separator = true };

template<class CharT>
class separator {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;

    explicit separator(string_view separators)
        : separators_{separators.begin(), separators.end()}
    {}

    /// <summary>
    /// strの最初の位置から始まる文字列がどの種類のトークンか調べる
    /// [str.begin(), retval.second)がトークン
    /// </summary>
    /// <returns>トークン, トークンの終わりの位置</returns>
    [[nodiscard]]
    auto operator()(string_view str)  const
        ->std::pair<token, typename string_view::const_iterator>
    {
        std::pair<token, typename string_view::const_iterator> retval;
        if (is_separator(str.front())) {
            retval.first = token::separator;
            retval.second = ++str.begin();
            return retval;
        }
        retval.first = token::word;
        for (retval.second = ++str.begin();
             retval.second != str.end() && !is_separator(*retval.second);
             ++retval.second);
        return retval;
    }
    [[nodiscard]]
    bool is_separator(CharT c) const {
        return separators_.count(c);
    }
private:
    std::set<CharT> separators_;
};

}
