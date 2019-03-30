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

}
