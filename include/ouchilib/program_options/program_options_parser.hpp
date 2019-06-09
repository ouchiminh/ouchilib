#pragma once
#include <map>
#include <any>
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "program_options_description.hpp"
#include "key_parser.hpp"
#include "ouchilib/utl/step.hpp"

namespace ouchi::program_options {

template<class CharT, class Traits = std::char_traits<CharT>>
class arg_parser {
public:
    using string = std::basic_string<CharT, Traits>;
    using string_view = std::basic_string_view<CharT, Traits>;
    using arg_map = std::map<string, std::any, std::less<>>;

    arg_parser() = default;
    arg_parser(basic_options_description<CharT, Traits>& desc,
               const CharT** argv,
               size_t argc)
    {
        parse(desc, argv, argc);
    }

    const arg_map& parse(basic_options_description<CharT, Traits>& desc,
                         const CharT** argv,
                         size_t argc)
    {
        using kp = key_parser<CharT, Traits>;
        arguments_.clear();
        auto opt_value_it = desc.begin();
        for (auto&& i : step(1ull, argc)) {
            string_view sv(argv[i]);
            if(kp::is_option(sv)){
                if(desc.end() == (opt_value_it = desc.find_option(sv.data())))
                    throw std::runtime_error("parse error:unknown option");
                opt_value_it->second.second->set();
                continue;
            }
            opt_value_it->second.second->translate(argv[i]);
        }
        for (const auto& i : desc) {
            if (!i.second.second->get().has_value()) continue;
            arguments_.insert_or_assign(kp::external_expression(i.first),
                                        i.second.second->get());
        }
        return arguments_;
    }
    const arg_map& get() const { return arguments_; }
    template<class T>
    auto get(string_view key) const
        -> std::remove_cv_t<std::remove_reference_t<T>>
    {
        using res_t = std::remove_cv_t<std::remove_reference_t<T>>;
        return std::any_cast<res_t>(arguments_.find(key)->second);
    }
    bool exist(string_view key) const
    {
        return arguments_.end() != arguments_.find(key);
    }
private:
    arg_map arguments_;
};

}
