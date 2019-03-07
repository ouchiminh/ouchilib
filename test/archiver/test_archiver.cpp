#pragma once

#include "test.hpp"
#if 0
#include "../../include/ouchilib/archiver.hpp"
namespace ouchi::archive::test {
	DEFINE_TEST(test_archiver) {
		ouchi::archive::archiver a;
		namespace fs = std::filesystem;
		REQUIRE_TRUE(a.add_file(".\\res\\sfml-logo-big.png"));
		REQUIRE_TRUE(a.add_file(".\\res\\fruit_cut_orange.png"));
		a.write(fs::path(".\\res\\graphic.arc"));
		REQUIRE_TRUE(a.load(fs::path(".\\res\\graphic.arc")));
	}
}
#endif
