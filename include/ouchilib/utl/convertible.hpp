#pragma once
#include <type_traits>
#include <utility>

namespace ouchi {

namespace detail {

template<int... Is>
struct seq { };

template<int N, int... Is>
struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

template<int... Is>
struct gen_seq<0, Is...> : seq<Is...> { };

template<typename T, typename F, int... Is>
inline void for_each(T&& t, F && f, seq<Is...>) {
	auto l = { (f(std::get<Is>(t)), 0)... };
}

template<class T>
constexpr bool false_v = false;

}//detail

template<typename F, typename... Ts>
inline void for_each_in_tuple(std::tuple<Ts...> const& t, F && f) {
	detail::for_each(t, std::forward<F>(f), detail::gen_seq<sizeof...(Ts)>());
}

template<typename F, typename... Ts>
inline void for_each_in_tuple(std::tuple<Ts...>& t, F && f) {
	detail::for_each(t, std::forward<F>(f), detail::gen_seq<sizeof...(Ts)>());
}

template<class T, class ...Converters>
class auto_convert {
	std::tuple<Converters...> converters_;
	T val_;
public:
	constexpr auto_convert(T && val, Converters &&...converters) :
		converters_{ std::move(converters)... },
		val_(std::move(val))
	{}
	constexpr auto_convert(T const & val, Converters &&...converters) :
		converters_{ std::move(converters)... },
		val_(val)
	{}
	constexpr auto_convert(Converters &&...converters) :
		converters_{ std::move(converters)... },
		val_(T{})
	{}

	template<class U>
	constexpr explicit operator U() const {
		U result;
		auto convert = [&result, this](auto && converter) {
			if constexpr (std::is_invocable_r_v<U, decltype(converter), T>) {
				result = converter(val_);
			} else if constexpr (std::is_convertible_v<T, U>) {
				result = static_cast<U>(val_);
			} else;
		};
		for_each_in_tuple(converters_, convert);
		return result;
	}
	template<class U>
	auto_convert & operator=(U&& val) {
		auto convert = [&val, this](auto && converter) {
			if constexpr (std::is_invocable_r_v<T, decltype(converter), U>) {
				val_ = converter(std::forward<U>(val));
			} else if constexpr (std::is_convertible_v<T, U>) {
				val_ = static_cast<T>(val);
			} else;
		};
		for_each_in_tuple(converters_, convert);
		return *this;
	}
};

// helper functions
template<class T, class ...Converters>
constexpr auto make_auto_convert(T && val, Converters ...converters) {
	return auto_convert(std::forward<T>(val), std::forward<Converters>(converters)...);
}

}// ouchi
