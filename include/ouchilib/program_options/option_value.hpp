#pragma once
#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <any>
#include <stdexcept>
#include "option_value_restriction.hpp"
#include "ouchilib/utl/translator.hpp"

namespace ouchi::program_options::detail{

// コマンドライン引数を制約に基づいてユーザーが利用できる形に変換、補完、エラー報告をするクラス
template<class CharT, class Traits = std::char_traits<CharT>>
struct option_info_base {
    virtual ~option_info_base() = default;
    virtual std::any translate(const std::basic_string<CharT, Traits>& value) = 0;
    virtual const std::any& get() const noexcept = 0;
};

template<
    class CharT,
    class Type,
    class Traits = std::char_traits<CharT>
>
struct option_info : option_info_base<CharT, Traits> {
    std::any value;

    template<class T>
    option_info(T&&){}
    option_info() = default;

    virtual std::any translate(const std::basic_string<CharT, Traits>& value) override
    { return std::any{}; }
    virtual const std::any& get() const noexcept override
    {
        return value;
    }
};

template<
    class CharT,
    class T,
    class Traits
>
struct option_info<CharT, single_value<T>, Traits> : option_info_base<CharT, Traits> {
    using value_type = typename single_value<T>::value_type;
    std::any value;
    
    option_info(value_type&& default_value)
        : value(std::move(default_value))
    {}
    option_info() = default;

    virtual std::any translate(const std::basic_string<CharT, Traits>& origin) override {
        typename ouchi::translator_between<std::basic_string<CharT, Traits>, value_type>::type
            t;
        if (auto r = t.get_value(origin); r) {
            value = r;
        } else throw std::runtime_error("parse error.");
        return value;
    }

    virtual const std::any& get() const noexcept override
    {
        return value;
    }
};

template<
    class CharT,
    class T,
    class Traits
>
struct option_info<CharT, multi_value<T>, Traits> : option_info_base<CharT, Traits> {
    using value_type = typename multi_value<T>::value_type;
    std::any value;
    
    option_info(std::initializer_list<T> default_value)
        : value(value_type(default_value))
    {}
    option_info(typename value_type::value_type&& default_value)
        : value(value_type{})
    {
        auto& v = std::any_cast<value_type&>(value);
        v.push_back(default_value);
    }
    option_info()
        : value(value_type{})
    {}

    virtual std::any translate(const std::basic_string<CharT, Traits>& origin) override {
        typename ouchi::translator_between<std::basic_string<CharT, Traits>, T>::type
            t;
        auto& v = std::any_cast<value_type&>(value);
        if (auto r = t.get_value(origin); r) {
            v.push_back(r.value());
        } else throw std::runtime_error("parse error.");

        return value;
    }

    virtual const std::any& get() const noexcept override
    {
        return value;
    }
};

}
