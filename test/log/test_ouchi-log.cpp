#pragma once
#include "../../include/ouchilib/log/out.hpp"
#include "../../include/ouchilib/log/rule.hpp"
#include "../test.hpp"

namespace ouchi::test {
#if 0
DEFINE_TEST(log_test) {
	std::ofstream f1(".\\log\\app2.log");
	std::ofstream f2(".\\log\\app3.log");
	ouchi::log::basic_out<char> logger1;
	ouchi::log::basic_out<char> logger2{ [&f2](std::string_view str) {f2 << str << std::endl; } };
	ouchi::log::rule r;
	
	ouchi::log::ruled_logger<char> rl{ {r,logger1}, {r,logger2} };
	logger1.reset_format() << ouchi::log::time << ouchi::log::cat << ouchi::log::msg;

	rl.trace("unko");
	rl.info("inpo");
}

DEFINE_TEST(out_func_test) {
	ouchi::log::out_func<char> of(".\\log\\out_func_test.log");
	ouchi::log::basic_out<char> logger(of);
	ouchi::log::rule r;
	ouchi::log::ruled_logger<char> rl{ {r, logger} };

	rl.info("test success??");
	rl.fatal("fatal success??");
}
#endif

DEFINE_TEST(read_setting_from_json_test) {
	ouchi::log::ruled_logger<char> logger(".\\log_setting.json", "setting");
	logger.info("success");
	logger.fatal("success");
}

}// ouchi::test
