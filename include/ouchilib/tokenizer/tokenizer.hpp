#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "separator.hpp"

namespace ouchi::tokenizer {

template<class CharT>
class tokenizer {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    enum class launch_policy { deferd, immediate };
    tokenizer(string_view str,
              const separator<CharT>& sep,
              launch_policy lp = launch_policy::deferd)
        : str_{ str }
        , sep_{ sep }
    {
        if (lp == launch_policy::immediate)
            tokenize();
    }

    tokenizer(string_view str,
              separator<CharT>&& sep,
              launch_policy lp = launch_policy::deferd)
        : str_{ str }
        , sep_{ std::move(sep) }
    {
        if (lp == launch_policy::immediate)
            tokenize();
    }

    using const_iterator = typename std::vector<std::pair<token, string>>::const_iterator;

    const_iterator begin()
    {
        if (str_.size() && tokens_.size() == 0)
            tokenize();
        return tokens_.cbegin();
    }
    const_iterator end()
    {
        if (str_.size() && tokens_.size() == 0)
            tokenize();
        return tokens_.cend();
    }
    const_iterator begin() const { tokens_.cbegin(); }
    const_iterator end() const { tokens_.cend(); }
private:
    void tokenize()
    {
        string buffer;
        string_view copy = str_;
        while (copy.size()) {
            auto i = sep_(copy);
            tokens_.push_back({ i.first, string{copy.begin(), i.second} });
            copy.remove_prefix(std::distance(copy.begin(), i.second));
        }
    }
    std::vector<std::pair<token, string>> tokens_;
    string str_;
    separator<CharT> sep_;
};

}
