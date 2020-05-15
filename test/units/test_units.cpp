#include "../test.hpp"
#include "ouchilib/units/units.hpp"
#include "ouchilib/units/si.hpp"
#include <type_traits>

struct l {};
struct m {};
struct t {};
struct b {};
using system_t = ouchi::units::system_of_units<l, m, t, b>;
using meter_t = typename system_t::template unit_t<l>;
using kg_t = typename system_t::template unit_t<m>;
using g_t = typename system_t::template unit_t<m>;
using bit_t = typename system_t::template unit_t<b>;
using s_t = typename system_t::template unit_t<t>;
namespace {
constexpr auto meter = system_t::make_unit<l>();
constexpr auto kg = system_t::make_unit<m, 1, std::kilo>();
constexpr auto g = system_t::make_unit<m>();
constexpr auto bit = system_t::make_unit<b>();
constexpr auto byte = std::ratio<8>{} * bit;
constexpr auto gb = std::giga{} * byte;
constexpr auto sec = system_t::make_unit<t>();
constexpr auto min = std::ratio<60>{} * sec;
constexpr auto h = std::ratio<60>{} * min;
constexpr auto day = std::ratio<24>{} * h;
constexpr auto mon = std::ratio<30>{} * day;
}

DEFINE_TEST(quantity_conversion_test)
{
    using namespace ouchi::units;

    auto q = 1 | g;
    auto r = 1 | kg;
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(q)>,
                  quantity<int, std::remove_cvref_t<decltype(g)>>
    >);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(q + r)>,
                  quantity<int, std::remove_cvref_t<decltype(g)>>
    >);
    auto tg = r.convert(g);
    CHECK_EQUAL(tg.get_value(), 1000);
    tg = r;
    CHECK_EQUAL(tg.get_value(), 1000);
    auto sum = q + r;
    CHECK_EQUAL(sum.get_value(), 1001);
    auto diff = r - q;
    CHECK_EQUAL(diff.get_value(), 999);
    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(diff)>,
                  quantity<int, g_t>
    >);
}

DEFINE_TEST(quantity_compare_test)
{
    using namespace ouchi::units;
    auto q = 1 | g;
    auto r = 1 | kg;
    auto s = 1000.0 | g;
    q <=> r;
    CHECK_TRUE(q < r);
    CHECK_TRUE(q <= r);
    CHECK_TRUE(r > q);
    CHECK_TRUE(r >= q);
    CHECK_TRUE(r == s);
    CHECK_TRUE(q != r);
}

DEFINE_TEST(quantity_operator_test)
{
    using namespace ouchi::units;

    {
        auto d1 = 1 | kg / (meter * meter * meter);
        auto mass = 1 | std::kilo{} * g;
        auto volume = 1 | meter * meter * meter;
        auto d2 = mass / volume;

        CHECK_EQUAL(d1, d2);
    }
    {
        auto are = system_t::make_unit<l, 2, std::deca>();
        auto hectare = std::hecto{} * are;
        static_assert(decltype(are)::ratio::num == 100);
        static_assert(decltype(hectare)::ratio::num == 10'000);
        auto onemeter = 1 | std::deca{}*meter;
        auto oneare = onemeter * onemeter;
        CHECK_EQUAL(1 | are, oneare);
    }
}
#if 0
DEFINE_TEST(quantity_test)
{
    using namespace ouchi::units;
    auto data_q = 100 | std::mega{} * bit / sec;
    auto gbpmon = gb / mon;
    auto data_p_mon = data_q.convert<decltype(gbpmon)::unit_type>();
    std::cout << data_p_mon.get_value() << std::endl;
}
#endif

