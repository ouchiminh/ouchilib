#pragma once
#include <optional>
#include <string>

namespace ouchi {

template<class From, class To>
struct static_cast_translator {
    using internal_type = From;
    using external_type = To;
    std::optional<To> get_value(const From& v) const { return std::optional<To>(static_cast<To>(v)); }
    std::optional<From> put_value(const To& v) const { return std::optional<From>(static_cast<From>(v)); }
};

template<class From, class To>
struct construction_translator {
    using internal_type = From;
    using external_type = To;
    std::optional<To> get_value(const From& v) const { return std::optional<To>(To(v)); }
    std::optional<From> put_value(const To& v) const { return std::optional<From>(From(v)); }
};

template<class From, class To>
struct translator_between {
    using type = construction_translator<From, To>;
};

// string => integer
template<class Str = std::string, class Int = int>
struct string_int_translator{};

#define DEF_STR_NUM_TRANSLATOR(integer_type, str_to_integer)\
template<>\
struct string_int_translator<std::string, integer_type> {\
    using internal_type = std::string;\
    using external_type = integer_type;\
\
    std::optional<integer_type> get_value(const std::string& v) const\
    try { return std::optional<integer_type>(str_to_integer(v)); }\
    catch (...) { return std::nullopt; }\
\
    std::optional<std::string> put_value(integer_type v) const \
    try { return std::optional<std::string>(std::to_string(v)); }\
    catch (...) { return std::nullopt; }\
};\
template<>\
struct translator_between<std::string, integer_type> {\
    using type = string_int_translator<std::string, integer_type>;\
};

#define DEF_WSTR_NUM_TRANSLATOR(integer_type, str_to_integer)\
template<>\
struct string_int_translator<std::wstring, integer_type> {\
    using internal_type = std::wstring;\
    using external_type = integer_type;\
\
    std::optional<integer_type> get_value(const std::wstring& v) const\
    try { return std::optional<integer_type>(str_to_integer(v)); }\
    catch (...) { return std::nullopt; }\
\
    std::optional<std::wstring> put_value(integer_type v) const \
    try { return std::optional<std::wstring>(std::to_wstring(v)); }\
    catch (...) { return std::nullopt; }\
};\
template<>\
struct translator_between<std::wstring, integer_type> {\
    using type = string_int_translator<std::wstring, integer_type>;\
};

#define DEF_BOTH_STR_INT_TRANSLATOR(integer_type, str_to_integer)\
DEF_STR_NUM_TRANSLATOR(integer_type, str_to_integer)\
DEF_WSTR_NUM_TRANSLATOR(integer_type, str_to_integer)

DEF_BOTH_STR_INT_TRANSLATOR(unsigned long long, std::stoull)
DEF_BOTH_STR_INT_TRANSLATOR(long long, std::stoll)
DEF_BOTH_STR_INT_TRANSLATOR(unsigned long, std::stoul)
DEF_BOTH_STR_INT_TRANSLATOR(long, std::stol)
DEF_BOTH_STR_INT_TRANSLATOR(int, std::stoi)
DEF_BOTH_STR_INT_TRANSLATOR(float, std::stof)
DEF_BOTH_STR_INT_TRANSLATOR(double, std::stod)
DEF_BOTH_STR_INT_TRANSLATOR(long double, std::stold)

#undef DEF_BOTH_STR_INT_TRANSLATOR
#undef DEF_STR_NUM_TRANSLATOR
#undef DEF_WSTR_NUM_TRANSLATOR
}
