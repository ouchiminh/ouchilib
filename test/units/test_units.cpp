#include "../test.hpp"
#include "ouchilib/units/units.hpp"
#include "ouchilib/units/si.hpp"
#include <type_traits>

struct l {};
struct m {};
struct t {};
using system_t = ouchi::units::system_of_units<l, m, t>;
using meter_t = typename system_t::template unit_t<l>;
using kg_t = typename system_t::template unit_t<m, 1, std::kilo>;
using g_t = typename system_t::template unit_t<m, 1>;
namespace {
constexpr auto meter = system_t::make_unit<l>();
constexpr auto kg = system_t::make_unit<m, 1, std::kilo>();
constexpr auto g = system_t::make_unit<m>();
}

DEFINE_TEST(units_instantiate)
{
    
    using namespace ouchi::units;

    static_assert(detail::belongs<int, std::tuple<int, long>>::value);
    static_assert(!detail::belongs<int, std::tuple<double, long>>::value);
    static_assert(!detail::includes<std::tuple<int, long>, std::tuple<int, float>>::value);
    static_assert(detail::includes_v<std::tuple<int, long>, std::tuple<int, float, long>>);
    static_assert(detail::set_equal_v<std::tuple<int, long>, std::tuple<long, int>>);
    static_assert(!detail::set_equal_v<std::tuple<int, float>, std::tuple<long, int>>);

    [[maybe_unused]]
    basic_dimension<l, 1, std::kilo> km;
    using len = basic_dimension<l, 1>;
    using mass = basic_dimension<m, 1, std::kilo>;
    using time = basic_dimension<t, 1>;
    using time0 = basic_dimension<t, 0>;
    [[maybe_unused]]
    basic_units<len, mass, time> mks;
    static_assert(std::is_same_v<
                  typename detail::mul_unit<basic_units<len, mass, time>, basic_units<len, time, mass>>::type,
                  basic_units<basic_dimension<l, 2>, basic_dimension<m, 2, std::mega>, basic_dimension<t, 2>>>);
    static_assert(detail::is_convertible_unit<decltype(mks), decltype(mks)>::value);
    static_assert(detail::is_convertible_unit<decltype(mks), basic_units<len, time, mass>>::value);
    static_assert(!detail::is_convertible_unit<decltype(mks), basic_units<len, time0, mass>>::value);

    typedef system_of_units<l, m, t> mks_system;
    static_assert(std::is_same_v<
                  decltype(mks_system::make_unit<l>()),
                  basic_units<len, basic_dimension<m, 0>, time0>>);
}

DEFINE_TEST(unit_operator_test)
{
    using namespace ouchi::units;

    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(kg)>,
                  basic_units<basic_dimension<l, 0>, basic_dimension<m, 1, std::kilo>, basic_dimension<t, 0>>>);

    static_assert(std::is_same_v<
                  std::remove_cvref_t<decltype(kg / g)>,
                  basic_units<basic_dimension<l, 0>, basic_dimension<m, 0, std::kilo>, basic_dimension<t, 0>>>);

    static_assert(std::is_same_v<
                  decltype(kg.ratio(g)),
                  std::milli>);
    static_assert(std::is_same_v<
                  decltype(g.ratio(kg)),
                  std::kilo>);
    static_assert(g < kg);
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

