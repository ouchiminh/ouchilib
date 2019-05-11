#pragma once
#include <string>
#include <string_view>
#include <list>
#include "separator.hpp"

namespace ouchi::tokenizer {

enum class token_type : std::uint32_t { primitive_word = (std::uint32_t)primitive_token::word,
                                        primitive_separator = (std::uint32_t)primitive_token::separator };

template<class CharT>
class tokenizer {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using container_type = std::list<std::pair<token_type, string>>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using value_type = typename std::iterator_traits<iterator>::value_type;

    tokenizer(string_view str,
              const separator<CharT>& sep)
        : str_{ str }
        , sep_{ sep }
    {
        tokenize();
    }

    tokenizer(string_view str,
              separator<CharT>&& sep)
        : str_{ str }
        , sep_{ std::move(sep) }
    {
        tokenize();
    }
    tokenizer(const separator<CharT>& sep)
        : sep_{ sep }
    {}
    tokenizer(separator<CharT>&& sep)
        : sep_{ std::move(sep) }
    {}

    const_iterator cbegin() const { return tokens_.cbegin(); }
    const_iterator cend() const { return tokens_.cend(); }
    const_iterator begin() const { return tokens_.cbegin(); }
    const_iterator end() const { return tokens_.cend(); }
    iterator begin() { return tokens_.begin(); }
    iterator end() { return tokens_.end(); }
    typename container_type::size_type size() const noexcept { return tokens_.size(); }

    /// <summary>
    /// posの位置にある要素を削除する
    /// </summary>
    /// <param name="pos">削除対象</param>
    /// <returns>削除した次の位置の要素</returns>
    iterator erase(const_iterator pos) noexcept
    {
        return tokens_.erase(pos);
    }
    /// <summary>
    /// [from, to]の範囲を一つのトークンにまとめる。
    /// </summary>
    /// <param name="from">始点</param>
    /// <param name="to">終点(まとめられない)</param>
    /// <returns>まとめられたトークンの位置</returns>
    iterator merge(iterator from, iterator to) noexcept
    {
        auto itr = std::next(from);
        std::advance(to, 1);
        do {
            from->second += itr->second;
            itr = tokens_.erase(itr);
        }
        while (itr != to);
        return from;
    }

private:
    void tokenize()
    {
        string buffer;
        string_view copy = str_;
        while (copy.size()) {
            auto i = sep_(copy);
            tokens_.push_back({ (token_type)i.first, string{copy.begin(), i.second} });
            copy.remove_prefix(std::distance(copy.begin(), i.second));
        }
    }
    container_type tokens_;
    string str_;
    separator<CharT> sep_;
};

}
