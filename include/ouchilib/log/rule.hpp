#pragma once
#include <algorithm>
#include <vector>
#include <initializer_list>
#include <filesystem>
#include <chrono>
#include "format.hpp"
#include "out.hpp"
#if __has_include("boost/property_tree/json_parser.hpp") && __has_include("boost/foreach.hpp")
#	include "boost/property_tree/json_parser.hpp"
#	include "boost/foreach.hpp"
#	define OUCHI_BOOST_ENABLED
#endif

namespace ouchi::log {

struct rule {
public:
	cat_v upper_ = cat_v::fatal,
		lower_ = cat_v::info;

	void set(cat_v upper, cat_v lower = cat_v::info) noexcept {
		upper_ = std::clamp(upper, cat_v::trace, cat_v::fatal);
		lower_ = std::clamp(lower, cat_v::trace, upper_);
	}
	bool is_valid(cat_v level) const noexcept {
		return level <= upper_ && level >= lower_;
	}
	rule(cat_v upper, cat_v lower)
        : upper_{upper}
        , lower_{lower}
	{}
	rule() = default;
};

template<class CharT>
class ruled_logger {
	std::vector<std::pair<rule, basic_out<CharT>>> logger_;
#ifdef OUCHI_BOOST_ENABLED
    std::optional<ouchi::time_keeper<std::chrono::system_clock>> reload_duration_;
    std::filesystem::path file_;
    std::string key_;

	void read_json(std::filesystem::path const & file, std::string const & key) try {
		using namespace std::string_literals;
		if (!std::filesystem::exists(file)) throw std::runtime_error("no such file or directory."s + file.string());
		namespace pjson = boost::property_tree;
		pjson::ptree pt;
		pjson::read_json(file.string(), pt);
		auto tar = pt.get_child_optional(key);
		if (!tar) throw std::exception("key not found.");
        logger_.clear();
        reload_duration_.reset();
		/**
		 *	key : {
         *      rule : [
		 *  		{"file":"~~", "upper":N, "lower":N},
		 *  		{...}, ...
		 *	    ],
         *      auto_reload : N(seconds)
         *  }
		 **/
        if (auto child = tar.get().get_child_optional("auto_reload")) {
            reload_duration_.emplace(std::chrono::seconds(child.get().get_value_optional<int>().get_value_or(60)));
        }
		BOOST_FOREACH(const pjson::ptree::value_type & child, tar.get().get_child("rule")){
			const pjson::ptree & data = child.second;
			out_func<CharT> of;
			rule r;
			if (auto file = data.get_optional<std::string>("file"))
				of.set(file.value());
			if (auto upper = data.get_optional<unsigned>("upper"))
				r.upper_ = convert<cat_v, unsigned>(upper.value());
			if (auto lower = data.get_optional<unsigned>("lower"))
				r.lower_ = convert<cat_v, unsigned>(lower.value());
			logger_.push_back({ r ,basic_out<CharT>(std::move(of)) });
		}
        file_ = file;
        key_ = key;
	} catch (std::exception & e) {
		logger_.clear();
		logger_.push_back(std::make_pair<rule, basic_out<CharT>>(rule{}, basic_out<CharT>{}));
		LOG_WITH_FILEINFO(this->warn, e.what());
	}
#endif
	
public:
	ruled_logger(std::initializer_list<std::pair<rule, basic_out<CharT>>> && il)
        : logger_(std::move(il))
    {}
#ifdef OUCHI_BOOST_ENABLED
	ruled_logger(std::filesystem::path const & json_setting, std::string const & key) { read_json(json_setting, key); }
	ruled_logger(std::filesystem::path && json_setting, std::string && key)
        : ruled_logger(json_setting, key)
    {}
#endif

	void out(cat_v c, std::basic_string_view<CharT> msg) {
#ifdef OUCHI_BOOST_ENABLED
        if (reload_duration_ && reload_duration_.value().is_time()) read_json(file_, key_);
#endif
		for (auto & i : logger_) {
			if (i.first.is_valid(c)) i.second.out(c, msg);
		}
	}
	void fatal(std::basic_string_view<CharT> msg) {
		out(cat_v::fatal, msg);
	}
	void error(std::basic_string_view<CharT> msg) {
		out(cat_v::error, msg);
	}
	void warn(std::basic_string_view<CharT> msg) {
		out(cat_v::warn, msg);
	}
	void info(std::basic_string_view<CharT> msg) {
		out(cat_v::info, msg);
	}
	void debug(std::basic_string_view<CharT> msg) {
		out(cat_v::debug, msg);
	}
	void trace(std::basic_string_view<CharT> msg) {
		out(cat_v::trace, msg);
	}
    void raw(std::basic_string_view<CharT> msg) {
        for (auto& i : logger_) {
            i.second.raw(msg);
        }
    }

};

}
#ifdef OUCHI_BOOST_ENABLED
#	undef OUCHI_BOOST_ENABLED
#endif
