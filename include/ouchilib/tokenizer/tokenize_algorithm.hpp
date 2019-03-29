#pragma once
#include <vector>
#include <type_traits>
#include "tokenizer.hpp"

namespace ouchi::tokenizer {

template<class CharT, class F, std::enable_if_t<std::is_invocable_r_v<tokenizer<CharT>&, F, tokenizer<CharT>&>>* = nullptr>
tokenizer<CharT>& operator|(tokenizer<CharT>& t, F&& tokenize_algorithm)
{
    return tokenize_algorithm(t);
}

template<class CharT>
struct skip {
    constexpr skip(std::initializer_list<CharT> skip_char = { ' ', '\t' })
        : skipper{ skip_char }
    {}

    tokenizer<CharT>& operator() (tokenizer<CharT>& t)
    {
        for (auto i = t.begin(); i != t.end();) {
            if (i->second.size() == 1 && i->first != token_type::primitive_word &&
                std::find(skipper.begin(), skipper.end(), i->second.front()) != skipper.end())
                i = t.erase(i);
            else ++i;
        }
        return t;
    }

    std::vector<CharT> skipper;
};

template<class CharT>
struct merge_enclosed {
    merge_enclosed(std::initializer_list<std::basic_string<CharT>> il
                   = { "\"\"", "''" })
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
            if (auto merge_begin = std::find_if(t.begin(), t.end(),
                                                [&e](typename tokenizer<CharT>::value_type& p) {return p.second == e.first; });
                merge_begin != t.end())
            {
                if (auto merge_end = std::find_if(std::next(merge_begin), t.end(),
                                                  [&e](auto& p) {return p.second == e.second;});
                    merge_end != t.end())
                    t.merge(merge_begin, merge_end);
            }
        }
        return t;
    }

    std::vector<std::pair<std::basic_string<CharT>, std::basic_string<CharT>>> enclosed;
};

}
