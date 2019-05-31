﻿#pragma once
#include <string>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <functional>
#include <iomanip>
#include "option_value.hpp"
#include "ouchilib/tokenizer/tokenizer.hpp"

namespace ouchi::program_options {

// options description
template<class CharT, class Traits = std::char_traits<CharT>>
class basic_options_description{
public:
    using string = std::basic_string<CharT, Traits>;
    using value_type = detail::option_info_base<CharT, Traits>;
    // 実際にコンテナに格納される型。firstは説明
    using compound_value_type = std::pair<string, std::unique_ptr<value_type>>;
    using container_type = std::map<string, compound_value_type>;

    template<class ...R>
    basic_options_description& add(const string& key, const string& description, R&& ...restrictions)
    {
        typedef detail::option_info<char, detail::value_type_policy_t<std::remove_cv_t<std::remove_reference_t<R>>...>, Traits> option_t;
        std::unique_ptr<option_t> p;
        if constexpr (std::is_same_v<option_t, detail::flag_value> ||
                      std::is_same_v<void, find_derived_t<detail::default_value<void>, R...>>) p = std::make_unique<option_t>();
        else p = std::make_unique<option_t>(detail::find_default_value(std::forward<R>(restrictions)...));
        options_.insert_or_assign(
            key,
            std::make_pair<string, std::unique_ptr<value_type>>(description.data(), std::move(p))
        );
        return *this;
    }

    friend std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, const basic_options_description& bod)
    {
        auto org_flag = os.flags();
        size_t kmax_length = 0;
        for(const auto& d : bod.options_){
            kmax_length = std::max(d.first.size(), kmax_length);
        }
        os.setf(std::ios_base::left);
        for(const auto& d : bod.options_){
            os << std::setw(kmax_length+2)
               << d.first << d.second.first << '\n';
        }
        os.flags(org_flag);
        return os;
    }

    container_type& get_container() & noexcept { return options_; }
    const container_type& get_container() const& noexcept { return options_; }
private:
    container_type options_;
};

using options_description = basic_options_description<char>;
using woptions_description = basic_options_description<wchar_t>;

}
