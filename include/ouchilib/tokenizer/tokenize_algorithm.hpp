#pragma once
#include <vector>
#include <type_traits>
#include <regex>
#include "tokenizer.hpp"

namespace ouchi::tokenizer {

namespace detail {

// binary search [first, last)
template<class RanItr,
    class T = typename std::iterator_traits<RanItr>::value_type,
    class Pred = std::less<T>,
    std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<RanItr>::iterator_category>&&
                     std::is_invocable_r_v<bool, Pred, T, T> >* = nullptr
>
constexpr RanItr binary_find(RanItr first, RanItr last, T&& value, Pred&& pred = std::less<T>{}) {
    if (pred(value, *first) || pred(*std::next(last, -1), value)) return last;
    auto size = std::distance(first, std::next(last, -1));
    auto buf = first + size / 2;
    return
        pred(value, *buf) ? binary_find(first, buf, std::forward<T>(value), std::forward<Pred>(pred))
        : !pred(*buf, value) ? buf
        : binary_find(++buf, last, std::forward<T>(value), std::forward<Pred>(pred));
}

} // detail

template<class CharT, class F, std::enable_if_t<std::is_invocable_r_v<tokenizer<CharT>&, F, tokenizer<CharT>&>>* = nullptr>
tokenizer<CharT>& operator|(tokenizer<CharT>& t, F&& tokenize_algorithm)
{
    return tokenize_algorithm(t);
}

// 任意の文字と一致した字句を削除します。
template<class CharT>
struct skip {
    skip(std::initializer_list<CharT> skip_char = { ' ', '\t' })
        : skipper{ skip_char }
    {
        std::sort(skipper.begin(), skipper.end());
    }

    tokenizer<CharT>& operator() (tokenizer<CharT>& t)
    {
        for (auto i = t.begin(); i != t.end();) {
            if (i->second.size() == 1 &&
                detail::binary_find(skipper.begin(), skipper.end(), i->second.front()) != skipper.end())
                i = t.erase(i);
            else ++i;
        }
        return t;
    }
private:
    std::vector<CharT> skipper;
};

// 任意の文字列で囲まれた字句を一つのトークンにまとめます
template<class CharT>
struct merge_enclosed {
    merge_enclosed(std::initializer_list<std::basic_string<CharT>> il = { "\"\"", "''" })
    {
        using string = std::basic_string<CharT>;
        enclosed.reserve(il.size());
        for (auto&& i : il) {
            if (i.size() % 2 == 0 && i.size()) {
                enclosed.emplace_back(std::make_pair(string{i.begin(), i.begin() + i.size() / 2},
                                                     string{i.begin() + i.size() / 2, i.end()}));
            }
            else {
                enclosed.emplace_back(std::make_pair(i, i));
            }
        }
    }
    tokenizer<CharT>& operator() (tokenizer<CharT>& t) const
    {
        for (const auto& e : enclosed) {
            for (auto itr = t.begin(); itr != t.end();) {
                auto mb = std::find_if(itr, t.end(),
                                       [&e](const auto& p) {return p.second == e.first; });
                if (mb == t.end()) { ++itr;  break; }
                auto me = std::find_if(std::next(mb), t.end(),
                                       [&e](const auto& p) {return p.second == e.second; });
                if (me == t.end()) { ++itr; break; }
                itr = t.merge(mb, me);
            }
        }
        return t;
    }

    std::vector<std::pair<std::basic_string<CharT>, std::basic_string<CharT>>> enclosed;
};

// 字句にトークンの種別を割り振ります
template<class CharT>
struct assign_token {
    assign_token(std::initializer_list<std::pair<std::basic_regex<CharT>, token_type>> il)
        : ttlist_{ il }
    {}

    tokenizer<CharT>& operator() (tokenizer<CharT>& t) const
    {
        for (auto& i : t) {
            auto m = std::find_if(ttlist_.begin(), ttlist_.end(), [&i](auto& p) {return std::regex_match(i.second, p.first); });
            if (m != ttlist_.end()) i.first = m->second;
        }
        return t;
    }

private:
    std::vector<std::pair<std::basic_regex<CharT>, token_type>> ttlist_;
};

}
