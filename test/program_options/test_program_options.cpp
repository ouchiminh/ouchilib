#include <sstream>
#include "../test.hpp"
#include "ouchilib/program_options/program_options_description.hpp"
#include "ouchilib/program_options/program_options_parser.hpp"

DEFINE_TEST(test_program_options_description)
{
    namespace opo = ouchi::program_options;
    opo::basic_options_description<char> d;
    // instantiate test
    d.add("key", "something", opo::single<int>, opo::default_value = 12);
    d.add("multi", "", opo::multi<int>);
    //    "key;k"

    CHECK_EQUAL(2, d.get_container().size());
    CHECK_EQUAL(12, std::any_cast<int>(d.get_container().at("key").second->get()));
    (void)d.get_container().at("key").second->translate("31");
    CHECK_EQUAL(31, std::any_cast<int>(d.get_container().at("key").second->get()));
    auto& m = d.get_container().at("multi").second;
    m->translate("31");
    m->translate("32");
    CHECK_EQUAL(32, std::any_cast<std::vector<int>>(m->get())[1]);

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

DEFINE_TEST(test_wprogram_options_description)
{
    namespace opo = ouchi::program_options;
    opo::woptions_description d;
    // instantiate test
    d.add(L"key", L"something", opo::single<int>, opo::default_value = 12);
    //    "key;k"

    CHECK_EQUAL(1, d.get_container().size());
    CHECK_EQUAL(12, std::any_cast<int>(d.get_container().at(L"key").second->get()));
    (void)d.get_container().at(L"key").second->translate(L"31");
    CHECK_EQUAL(31, std::any_cast<int>(d.get_container().at(L"key").second->get()));

    std::wstringstream ss;
    ss << d;
}

DEFINE_TEST(test_wkey_parser)
{
    namespace opo = ouchi::program_options;
    using kp = opo::key_parser<wchar_t>;

    CHECK_EQUAL(kp::external_expression(L"key;k"), L"key");
    auto fk = kp::internal_expression(L"key;k");
    CHECK_EQUAL(fk.first, L"--key");
    CHECK_EQUAL(fk.second, L"-k");
}

DEFINE_TEST(test_arg_parse)
{
    namespace opo = ouchi::program_options;
    opo::arg_parser<char> ap;
    opo::options_description d;
    constexpr char argv_e[][32] = { "hoge.exe", "-s", "2", "-m", "31", "33", "-f" };
    constexpr auto argc = sizeof(argv_e)/sizeof(*argv_e);
    const char* argv[argc];
    for (auto i : ouchi::step(argc)) {
        argv[i] = argv_e[i];
    }

    d
        .add("", "", opo::single<int>, opo::default_value = 0)
        .add("key;k", "something", opo::single<int>, opo::default_value = 4)
        .add("second;s", "second", opo::single<int>)
        .add("multi;m", "v", opo::multi<int>);
    d
        .add("flag;f", "f", opo::flag)
        .add("nope;n", "f", opo::flag);
    

    ap.parse(d, argv, argc);
    CHECK_EQUAL(ap.get<int>(""), 0);
    CHECK_EQUAL(ap.get<int>("key"), 4);
    CHECK_EQUAL(ap.get<int>("second"), 2);
    CHECK_EQUAL(ap.get<std::vector<int>>("multi")[1], 33);
    CHECK_TRUE(ap.exist("flag"));
    CHECK_TRUE(!ap.exist("nope"));
}
