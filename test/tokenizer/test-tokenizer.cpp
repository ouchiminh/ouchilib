
#include "../test.hpp"
#include "ouchilib/tokenizer/tokenizer.hpp"
#include "ouchilib/tokenizer/csv-parser.hpp"
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
    const char(&csv)[] = { R"(key3,key3,key3
data1,data2,data3)" };
    ouchi::parser::csv_parser<char> pcsv;
    CHECK_NOTHROW(pcsv.parse(csv));
}

DEFINE_TEST(csv_parser_data_get_test)
{
    const char(&csv)[] = {
        R"(,head2,head3
k1,1,2
k2,3,4)" };
    ouchi::parser::csv_parser<char> pcsv;
    CHECK_NOTHROW(pcsv.parse(csv));
    CHECK_EQUAL(pcsv.at("head3", "k2"), "4");
    CHECK_TRUE(pcsv.at("head3", "k2") != "2");
}
