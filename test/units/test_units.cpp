#include "../test.hpp"
#include "ouchilib/units/units.hpp"
#include "ouchilib/units/si.hpp"
#include <type_traits>

struct l {};
struct m {};
struct t;
using system_t = ouchi::units::system_of_units<l, m, t>;
using meter_t = typename system_t::template unit_t<l>;
using kg_t = typename system_t::template unit_t<m>;
using g_t = typename system_t::template unit_t<m>;
namespace {
constexpr auto meter = system_t::make_unit<l>();
constexpr auto kg = system_t::make_unit<m, 1, std::kilo>();
constexpr auto g = system_t::make_unit<m>();
}

DEFINE_TEST(unit_operator_test)
{
    using namespace ouchi::units;

    1 | kg / (meter * meter * meter);
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
    auto s = 1 | kg;
    q <=> r;
    CHECK_TRUE(q < r);
    CHECK_TRUE(q <= r);
    CHECK_TRUE(r > q);
    CHECK_TRUE(r >= q);
    CHECK_TRUE(r == s);
    CHECK_TRUE(q != r);

}

