#pragma once
#include <string>
#include <map>
#include <memory>
#include <optional>
#include "option_value.hpp"

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
        typedef detail::value_type_policy_t<R...> option_t;
        std::unique_ptr<option_t> p;
        if constexpr (std::is_same_v<option_t, detail::flag_value>||
                      std::is_same_v<void, find_derived_t<detail::default_value<void>, R...>>) p = std::make_unique<option_t>();
        else p = std::make_unique<option_t>(detail::find_default_value(std::forward<R>(restrictions)...));
        options_.insert_or_assign(
            key,
            std::make_pair<string, std::unique_ptr<value_type>>(description, std::move(p))
        );
    }

#if !defined NDEBUG
    container_type& get_container() noexcept { return options_; }
#endif
private:
    std::map<string, compound_value_type> options_;
};

}
