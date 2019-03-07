#pragma once
#include <time.h>
#include <memory>
#include <iostream>
#include <functional>
#include <optional>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <fstream>
#include <sstream>
#include <utility>
#include <map>
#ifdef _MSC_VER
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif
#include "format.hpp"

namespace ouchi::log{

inline auto default_out = [](std::string_view str) { std::clog << str << std::endl; };


template<class CharT>
class out_func {
	std::string my_file_;
    
    std::map<std::string, std::unique_ptr<std::basic_ofstream<CharT>>>& access_map() {
    	static std::map<std::string, std::unique_ptr<std::basic_ofstream<CharT>>> files_;
        return files_;
    }

	void open_file() {
		using namespace std::literals;
		if (access_map().find(my_file_) != access_map().end()) return;
        auto ptr = std::make_unique<std::basic_ofstream<CharT>>(my_file_
                                                                , std::ios_base::out
#ifdef _MSC_VER
                                                                , _SH_DENYRW
#endif
        );
		if (!*ptr)
			throw std::runtime_error("file cannot open"s + my_file_);
		access_map().insert_or_assign(my_file_, std::move(ptr));
	}
public:
	out_func(std::string && filename) noexcept(false) : my_file_(std::move(filename)) { open_file(); }
	out_func(std::string const & filename) noexcept(false) : my_file_(filename) { open_file(); }
	out_func() = default;
	void set(std::string_view str) noexcept(false) {
		my_file_ = str;
		open_file();
	}

	void operator() (std::basic_string_view<CharT> str) {
		*access_map().at(my_file_) << str << std::endl;
	}
};

template<class CharT>
class unlock_eachtime_out_func {
	std::string my_file_;
	std::basic_ofstream<CharT> stream_;
	[[nodiscard]] bool open_file() {
		stream_.open(my_file_
					 , std::ios_base::out | std::ios_base::app
#ifdef _MSC_VER
					 , _SH_DENYRW
#endif
		);
		return !!stream_;
	}
public:
	unlock_eachtime_out_func(std::string&& filename)
		: my_file_{ std::move(filename) }
	{ }
	unlock_eachtime_out_func(const std::string& filename)
		: my_file_{ filename }
	{ }
	unlock_eachtime_out_func() = default;
	unlock_eachtime_out_func(unlock_eachtime_out_func&&) = default;
	unlock_eachtime_out_func(const unlock_eachtime_out_func& s)
		: my_file_(s.my_file_)
	{ }
	void set(std::string_view str) {
		my_file_ = str;
	}
	void operator() (std::basic_string_view<CharT> str) try {
		namespace interprocess = boost::interprocess;
		if(!open_file()) 
			throw std::runtime_error("cannot open file.");
		stream_ << str << std::endl;
		stream_.close();
	}catch(std::exception& e){
		std::clog << e.what();
	}
};

/// <summary>
/// this class writes log.
/// </summary>
/// <param name="Func">このクラスの()演算子に渡された文字列が出力されます。operator()(std::basic_string_view)</param>
/// <param name="Args">Funcのコンストラクタに渡す引数。</param>
template<class CharT>
class basic_out {
	inline static constexpr char category_str[][32] = {
		"[TRACE]","[DEBUG]","[INFO ]", "[WARN ]", "[ERROR]","[FATAL]"
		//"[FATAL]", "[ERROR]", "[WARN ]", "[INFO ]", "[DEBUG]", "[TRACE]"
	};
	using Func = std::function<void(std::basic_string_view<CharT>)>;
	Func f_;
	message_format<CharT> format_;
	// visitorのためのヘルパー型。
	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
	template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

public:
	basic_out(Func&& func = default_out) : f_(std::move(func)) {
		std::basic_ostringstream<CharT> ss;
		format_ << time << ss.widen(' ') << cat << ss.widen(' ') << msg;
	}

	void out(cat_v c, std::basic_string_view<CharT> msg) {
		std::basic_string<CharT> str;
		for (msg_element<CharT> const & i : format_)
			std::visit(
				overloaded{
					[&str](Time) { str.append(time_to_ISO8601<CharT>()); },
                    [&](Category) { std::basic_ofstream<CharT> s{}; std::basic_string<CharT> result; for (auto const & i : category_str[(unsigned)c]) { if (i == '\0') break; result.push_back(s.widen(i)); } str.append(result); },
					[&msg, &str](Msg) { str.append(msg); },
					[&str](std::basic_string<CharT> const & arg) { str.append(arg); },
					[&str](CharT c) { str.push_back(c); }
				},
				i.get_content()
			);

		f_(str);
	}

	basic_out<CharT> & fatal(std::basic_string_view<CharT> msg) {
		out(cat_v::fatal, msg);
		return *this;
	}
	basic_out<CharT> & error(std::basic_string_view<CharT> msg) {
		out(cat_v::error, msg);
		return *this;
	}
	basic_out<CharT> & warn(std::basic_string_view<CharT> msg) {
		out(cat_v::warn, msg);
		return *this;
	}
	basic_out<CharT> & info(std::basic_string_view<CharT> msg) {
		out(cat_v::info, msg);
	}
	basic_out<CharT> & debug(std::basic_string_view<CharT> msg) {
		out(cat_v::debug, msg);
		return *this;
	}
	basic_out<CharT> & trace(std::basic_string_view<CharT> msg) {
		out(cat_v::trace, msg);
		return *this;
	}
    basic_out<CharT> & raw(std::basic_string_view<CharT> msg) {
        f_(msg);
        return *this;
    }

    [[nodiscard]]
	message_format<CharT> & reset_format() noexcept {
		format_.clear();
		return format_;
	}
};

#pragma region "log out utl macro"

#define MYFILE (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG_WITH_FILEINFO(logger, msg)	do{\
	std::basic_stringstream<char> ss;\
	ss << msg << '@' << MYFILE << ':' << __LINE__;\
	logger(ss.str());\
}while(false)

#pragma endregion

} // ouchi::log
#ifdef OUCHI_BOOST_ENABLED
#	undef OUCHI_BOOST_ENABLED
#endif
