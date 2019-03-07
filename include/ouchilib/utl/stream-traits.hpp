#pragma once
#include <type_traits>
#include <cstdint>
#include <istream>
#if __has_include("SFML/System/InputStream.hpp")
#	include "SFML/System/InputStream.hpp"
#endif
#include "meta-helper.hpp"


namespace ouchi {
	template<class T>
	class istream_traits_impl {};

#if __has_include ("SFML/System/InputStream.hpp")
	template<>
	class istream_traits_impl<sf::InputStream>{
	public:
		using stream_t = sf::InputStream;
		
		static std::int64_t read(stream_t & s, void * data, std::int64_t size) {
			return s.read(data, size);
		}
		static std::int64_t seekg(stream_t  & s, std::int64_t pos) {
			return s.seek(pos);
		}
		static std::int64_t get_size(stream_t & s) {
			return s.getSize();
		}
		static std::int64_t tellg(stream_t & s) {
			return s.tell();
		}
	};
#endif

	template<typename CharT>
	class istream_traits_impl<std::basic_istream<CharT>> {
	public:
		static_assert(sizeof(CharT) == 1, "Size of T should be 1 byte for perfect compatibility with sf::InputStream.");

		using stream_t = std::basic_istream<CharT>;

		static std::int64_t read(stream_t & s, void * data, std::int64_t size) {
			s.read((CharT*)data, size);
			return s.gcount();
		}
		static std::int64_t seekg(stream_t  & s, std::int64_t pos) {
			s.seekg(pos, std::ios::beg);
			return s.tellg();
		}
		static std::int64_t get_size(stream_t & s) {
			auto org = s.tellg();
			std::int64_t size = s.seekg(0, std::ios::end).tellg();
			s.seekg(org, std::ios::beg);
			return size * sizeof(CharT);
		}
		static std::int64_t tellg(stream_t & s) {
			return s.tellg();
		}
	};

	template<class T>
	class istream_traits :
		public istream_traits_impl<typename search_base<T, 
#if __has_include("SFML/System/InputStream.hpp")
		sf::InputStream,
#endif
		std::basic_istream<char>, std::basic_istream<unsigned char>>::type>
	{};



	template<class T>
	class ostream_traits_impl{};

	template<typename CharT>
	class ostream_traits_impl<std::basic_ostream<CharT>> {
	public:
		static_assert(sizeof(CharT) == 1, "Size of T should be 1 byte for perfect compatibility with sf::InputStream.");

		using stream_t = std::basic_ostream<CharT>;

		static void write(stream_t & s, void * data, std::int64_t size) {
			s.write((CharT*)data, size);
		}
		static std::int64_t seekp(stream_t & s, std::int64_t pos) {
			s.seekp(pos, std::ios::beg);
			return s.tellp();
		}
		static std::int64_t tellp(stream_t & s) {
			return s.tellp();
		}
	};

	template<class T>
	class ostream_traits :
		public ostream_traits_impl<typename search_base<T, std::basic_ostream<char>, std::basic_ostream<unsigned char>>::type>
	{ };


}
