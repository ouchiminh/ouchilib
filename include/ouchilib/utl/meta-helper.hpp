#pragma once
#include <type_traits>
#include <utility>
#include <tuple>
#include <variant>

namespace ouchi {

template<class D, class Head, class ...Arg>
class search_base {
public:
	typedef std::conditional_t<
		std::is_base_of_v<Head, D>,
		Head,
		typename search_base<D, Arg...>::type
	> type;
};

template<class D, class Head>
class search_base<D, Head> {
public:
	typedef std::conditional_t<
		std::is_base_of_v<Head, D>,
		Head,
		void
	> type;
};

struct has_iterator_impl {
    template<class C>
    static auto check(...)->std::false_type;
    template<class C>
    static auto check(C&&)->decltype(std::declval<C>().begin(), std::declval<C>().end(),
                                     std::true_type{});
};
template<class C>
struct has_iterator : decltype(has_iterator_impl::check<C>(std::declval<C>())) {};
template<class T, size_t Size>
struct has_iterator<T[Size]> : std::true_type {};
template<class C>
inline constexpr bool has_iterator_v = has_iterator<C>::value;

struct has_size_impl {
    template<class C>
    static auto check(...)->std::false_type;
    template<class C>
    static auto check(C&&)->decltype(std::declval<C>().size(),
                                     std::true_type{});
};
template<class C>
struct has_size : decltype(has_size_impl::check<C>(std::declval<C>())) {};
template<class T, size_t Size>
struct has_size<T[Size]> : std::true_type {};
template<class C>
inline constexpr bool has_size_v = has_size<C>::value;

template<class C>
struct his_iterator {
    using type = decltype(begin(std::declval<C&>()));
};
template<class T, size_t Size>
struct his_iterator<T[Size]> {
    using type = T*;
};
template<class C>
using his_iterator_t = typename his_iterator<C>::type;

template<class B, class ...D>
struct find_derived {};

template<class B>
struct find_derived<B> { using type = void; };

template<class B, class Head, class ...D>
struct find_derived<B, Head, D...> {
    using type = std::conditional_t<
        std::is_base_of_v<B, Head>,
        Head,
        typename find_derived<B, D...>::type
    >;
};

template<class B, class ...D>
using find_derived_t = typename find_derived<B, D...>::type;

template<class T>
struct variant_compatible { using type = T; };
template<>
struct variant_compatible<void> { using type = std::monostate; };
template<class T>
using variant_compatible_t = typename variant_compatible<T>::type;

}
