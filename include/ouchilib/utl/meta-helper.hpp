#pragma once
#include <type_traits>
#include <utility>
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
}