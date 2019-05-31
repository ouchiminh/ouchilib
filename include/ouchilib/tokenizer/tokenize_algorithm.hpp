#pragma once
#include <vector>
#include <type_traits>
#include <regex>
#include <functional>
#include "tokenizer.hpp"

namespace ouchi::tokenizer {

namespace detail {

// binary search [first, last)
template<
	class RanItr,
    class T = typename std::iterator_traits<RanItr>::value_type,
    class Pred = std::less<std::remove_reference_t<T>>,
    std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<RanItr>::iterator_category>&&
                     std::is_invocable_r_v<bool, Pred, const std::remove_reference_t<T>&,  const std::remove_reference_t<T>&> >* = nullptr
>
constexpr RanItr binary_find(RanItr first, RanItr last, T&& value, Pred&& less = std::less<std::remove_reference_t<T>>{}) {
	if (less(value, *first) || less(*std::next(last, -1), value)) return last;
	auto size = std::distance(first, std::next(last, -1));
	auto buf = first + size / 2;
	return
		less(value, *buf) ? binary_find(first, buf, std::forward<T>(value), std::forward<Pred>(less))
		: !less(*buf, value) ? buf
		: binary_find(++buf, last, std::forward<T>(value), std::forward<Pred>(less));
}

} // detail

template<class CharT, class F, std::enable_if_t<std::is_invocable_r_v<tokenizer<CharT>&, std::remove_reference_t<F>, tokenizer<CharT>&>>* = nullptr>
tokenizer<CharT>& operator|(tokenizer<CharT>& t, F&& tokenize_algorithm)
{
    return tokenize_algorithm(t);
}

// 任意の文字と一致した字句を削除します。
template<class CharT>
[[nodiscard]] inline auto skip(const std::basic_regex<CharT>& pattern) {
    return
        [pattern](tokenizer<CharT>& t) -> tokenizer<CharT>& {
        for (auto i = t.begin(); i != t.end();) {
            i = (std::regex_match(i->second, pattern))
                ? t.erase(i)
                : std::next(i);
        }
        return t;
    };
}

// 任意のトークンタイプと一致した字句を削除します。
template<class CharT>
[[nodiscard]] inline auto skip(token_type tt) {
    return
        [tt](tokenizer<CharT>& t) -> tokenizer<CharT>& {
        for (auto i = t.begin(); i != t.end();) {
            i = (i->first == tt)
                ? t.erase(i)
                : std::next(i);
        }
        return t;
    };
}

// 条件を満たした字句を削除します。
template<class CharT, class Pred,
         std::enable_if_t<std::is_invocable_r_v<bool,
                                                std::remove_reference_t<Pred>,
                                                std::pair<token_type, std::basic_string<CharT>>>
                         >* = nullptr>
[[nodiscard]] inline auto erase_if(Pred&& pred)
{
    return
        [pred](tokenizer<CharT> & t) -> tokenizer<CharT> & {
            for (auto i = t.begin(); i != t.end();)
                i = pred(*i) ? t.erase(i) : std::next(i);
            return t;
        };
}

// 任意の文字列で囲まれた字句を一つのトークンにまとめます
template<class CharT>
[[nodiscard]] inline auto merge_enclosed(std::initializer_list<std::basic_string<CharT>> il)
{
    std::vector<std::pair<std::basic_string<CharT>, std::basic_string<CharT>>> enclosed;
    using string = std::basic_string<CharT>;
    enclosed.reserve(il.size());
    for (auto&& i : il) {
        if (i.size() % 2 == 0 && i.size()) {
            enclosed.emplace_back(std::make_pair(string{ i.begin(), i.begin() + i.size() / 2 },
                                                 string{ i.begin() + i.size() / 2, i.end() }));
        } else {
            enclosed.emplace_back(std::make_pair(i, i));
        }
    }
    return
        [enclosed = std::move(enclosed)](tokenizer<CharT> & t)->tokenizer<CharT> & {
            for (const auto& e : enclosed) {
                for (auto itr = t.begin(); itr != t.end();) {
                    auto mb = std::find_if(itr, t.end(),
                                           [&e](const auto & p) {return p.second == e.first; });
                    if (mb == t.end()) { ++itr;  break; }
                    auto me = std::find_if(std::next(mb), t.end(),
                                           [&e](const auto & p) {return p.second == e.second; });
                    if (me == t.end()) { ++itr; break; }
                    itr = t.merge(mb, me);
                }
            }
            return t;
        };
}

// 字句にトークンの種別を割り振ります
template<class CharT>
[[nodiscard]] inline auto assign_token(std::initializer_list<std::pair<std::basic_regex<CharT>, token_type>> il)
{
    return
        [ttlist_ = std::move(il)](tokenizer<CharT>& t)->tokenizer<CharT> & {
            for (auto& i : t) {
                auto m = std::find_if(ttlist_.begin(), ttlist_.end(), [&i](auto& p) {return std::regex_match(i.second, p.first); });
                if (m != ttlist_.end()) i.first = m->second;
            }
            return t;
        };
}

}
