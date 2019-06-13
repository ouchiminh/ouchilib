#pragma once
#include <string>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <functional>
#include <iomanip>
#include "option_value.hpp"
#include "key_parser.hpp"
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "ouchilib/tokenizer/tokenize_algorithm.hpp"
#include "ouchilib/utl/step.hpp"

namespace ouchi::program_options {

// options description
template<class CharT, class Traits = std::char_traits<CharT>>
class basic_options_description{
public:
    using string = std::basic_string<CharT, Traits>;
    using string_view = std::basic_string_view<CharT, Traits>;
    using value_type = detail::option_info_base<CharT, Traits>;
    // 実際にコンテナに格納される型。firstは説明
    using compound_value_type = std::pair<string, std::unique_ptr<value_type>>;
    using container_type = std::map<string, compound_value_type>;
    using const_iterator = typename container_type::const_iterator;
    using iterator = typename container_type::iterator;

    template<class ...R>
    basic_options_description& add(string_view key, string_view description, R&& ...restrictions)
    {
        typedef detail::option_info<CharT, detail::value_type_policy_t<std::remove_cv_t<std::remove_reference_t<R>>...>, Traits> option_t;
        std::unique_ptr<option_t> p;
        if constexpr (std::is_same_v<void, find_derived_t<detail::default_value<void>,
                      std::remove_cv_t<std::remove_reference_t<R>>...>>)
            p = std::make_unique<option_t>();
        else
            p = std::make_unique<option_t>(detail::find_default_value(std::forward<R>(restrictions)...));
        
        options_.insert_or_assign(
            key.data(),
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

    iterator find_option(string_view internal_expr)
    {
        using kp = key_parser<CharT, Traits>;
        std::pair<string, string> ikey;
        for (auto it = begin(); it != end(); ++it) {
            ikey = kp::internal_expression(it->first);
            if (ikey.first == internal_expr || ikey.second == internal_expr) return it;
        }
        return end();
    }

    const_iterator begin() const noexcept { return options_.cbegin(); }
    const_iterator end() const noexcept { return options_.cend(); }
    iterator begin() noexcept { return options_.begin(); }
    iterator end() noexcept { return options_.end(); }

    container_type& get_container() & noexcept { return options_; }
    const container_type& get_container() const& noexcept { return options_; }
private:
    container_type options_;
};

using options_description = basic_options_description<char>;
using woptions_description = basic_options_description<wchar_t>;

}
