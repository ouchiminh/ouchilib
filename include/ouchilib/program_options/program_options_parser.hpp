#pragma once
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "option_value.hpp"

namespace ouchi::program_options {
// ASCII ONLY
template<class CharT, class Traits = std::char_traits<CharT>>
class key_parser {
    inline static const ouchi::tokenizer::separator<CharT> sep{(CharT)';',(CharT)',',(CharT)' '};
public:
    using string = std::basic_string<CharT, Traits>;

    static string external_expression(const string& rawkey)
    {
        namespace tk = ::ouchi::tokenizer;
        tk::tokenizer<CharT> tok{ rawkey, sep };
        tok | tk::skip<CharT>(tk::token_type::primitive_separator);
        if(tok.size()) return tok.begin()->second;
        return string{};
    }
    static std::pair<string, string> internal_expression(const string& rawkey)
    {
        namespace tk = ouchi::tokenizer;
        tk::tokenizer<CharT> tok{ rawkey, sep };
        std::pair<string, string> r;
        tok | tk::skip<CharT>(tk::token_type::primitive_separator);
        auto tok_it = tok.begin();
        for (auto&& i : ouchi::step(std::min(tok.size(), 2ull))) {
            auto hc = tok.size() - i;
            string formatted_key(hc, static_cast<CharT>('-'));
            formatted_key += (tok_it++)->second;
            (r.first.size() ? r.second : r.first) = std::move(formatted_key);
        }
        return r;
    }
};

template<class CharT, class Traits = std::char_traits<CharT>>
class arg_parser {

};

}
