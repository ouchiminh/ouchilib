
#include "../test.hpp"
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "ouchilib/tokenizer/csv.hpp"
#if 0
DEFINE_TEST(tokenizer_tokenize_test)
{
    ouchi::tokenizer::separator<char> sep(", ;");
    ouchi::tokenizer::tokenizer<char> t(",,abc,def ghi;jk,,,", sep);
    for (auto i : t) {
        std::cout << i.second << '\n';
    }
}
#endif

DEFINE_TEST(csv_parser_parse_test)
{
    const char(&csv)[] = { "key3,key3,key3\ndata1,data2,data3" };
    ouchi::parser::csv<char> pcsv;
    CHECK_NOTHROW(pcsv.parse(csv));
}

DEFINE_TEST(csv_parser_data_get_test)
{
    const char(&csv)[] = {
        R"(,head2,head3
k1,1,2
k2,3,4
)" };
    ouchi::parser::csv<char> pcsv;
    CHECK_NOTHROW(pcsv.parse(csv));
    CHECK_EQUAL(pcsv.at("head3", "k2"), "4");
    CHECK_TRUE(pcsv.at("head3", "k2") != "2");
    CHECK_TRUE(pcsv.at("head3", 1) == "2");
    CHECK_TRUE(pcsv.at(1, 1) == "1");
}
#if 1

DEFINE_TEST(csv_file_wcharbe_bom_test)
{
    ouchi::parser::csv<wchar_t> pcsv;
    REQUIRE_TRUE(std::filesystem::exists("u16be_test.csv"));
    pcsv.parse("u16be_test.csv", std::locale(std::locale::empty(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    REQUIRE_TRUE(pcsv.get().size());
    CHECK_EQUAL(pcsv.at(L"key1", 0), L"key1");
}
DEFINE_TEST(csv_file_wcharle_bom_test)
{
    ouchi::parser::csv<wchar_t> pcsv;
    REQUIRE_TRUE(std::filesystem::exists("u16le_test.csv"));
    pcsv.parse("u16le_test.csv", std::locale(std::locale::empty(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
    REQUIRE_TRUE(pcsv.get().size());
    CHECK_EQUAL(pcsv.at(L"key1", 0), L"key1");
}
DEFINE_TEST(csv_file_u8_bom_test)
{
    ouchi::parser::csv<char> pcsv;
    auto locale = std::locale{ std::locale{""} };
    REQUIRE_TRUE(std::filesystem::exists("u8test.csv"));
    pcsv.parse("u8test.csv", locale);
    REQUIRE_TRUE(pcsv.get().size());
    CHECK_EQUAL(pcsv.at(u8"キー1", 0), u8"キー1");
}
#endif
