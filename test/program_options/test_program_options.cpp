#include <sstream>
#include "../test.hpp"
#include "ouchilib/program_options/program_options_description.hpp"

DEFINE_TEST(test_program_options_description)
{
    namespace opo = ouchi::program_options;
    opo::basic_options_description<char> d;
    // instantiate test
    d.add("key", "something", opo::single<int>, opo::default_value = 12);
    //    "key;k"

    CHECK_EQUAL(1, d.get_container().size());
    CHECK_EQUAL(12, std::any_cast<int>(d.get_container().at("key").second->get()));
    (void)d.get_container().at("key").second->translate("31");
    CHECK_EQUAL(31, std::any_cast<int>(d.get_container().at("key").second->get()));

    std::stringstream ss;
    ss << d;
}

DEFINE_TEST(test_key_parser)
{
    namespace opo = ouchi::program_options;
    using kp = opo::key_parser<char>;

    CHECK_EQUAL(kp::external_expression("key;k"), "key");
    auto fk = kp::internal_expression("key;k");
    CHECK_EQUAL(fk.first, "--key");
    CHECK_EQUAL(fk.second, "-k");
}
