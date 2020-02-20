#pragma once
#include <string>
#include <string_view>
#include <locale>
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "ouchilib/tokenizer/tokenize_algorithm.hpp"
#include "ouchilib/utl/step.hpp"

namespace ouchi::program_options {

// ASCII ONLY
template<class CharT, class Traits = std::char_traits<CharT>>
class key_parser {
    inline static const ouchi::tokenizer::separator<CharT> sep{(CharT)';',(CharT)',',(CharT)' '};
public:
    using string = std::basic_string<CharT, Traits>;
    using string_view = std::basic_string_view<CharT, Traits>;

    // input : first;second
    // output: first
    inline static string external_expression(const string& rawkey)
    {
        namespace tk = ::ouchi::tokenizer;
        tk::tokenizer<CharT> tok{ rawkey, sep };
        tok | tk::skip<CharT>(tk::token_type::primitive_separator);
        if(tok.size()) return tok.begin()->second;
        return string{};
    }
    // input : first;second
    // output: --first, -second
    inline static std::pair<string, string> internal_expression(const string& rawkey)
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
    inline static string_view& remove_hyphen(string_view& opt)
    {
        auto not_hy = opt.find_first_not_of(static_cast<CharT>('-'));
        if (not_hy == string_view::npos) return string_view{};
        opt.remove_prefix(not_hy);
        return opt;
    }

    // 先頭が'-'かつ先頭から一つ以上連続する'-'の次の文字がアルファベットである
    inline static bool is_option(string_view program_arg)
    {
        std::locale l;
        auto not_hy = program_arg.find_first_not_of(static_cast<CharT>('-'));
        return
            program_arg.front() == static_cast<CharT>('-') &&
            not_hy != string_view::npos &&
            std::isalpha(program_arg[not_hy], l);
    }
};

}
