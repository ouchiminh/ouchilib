#include "../test.hpp"
#include "ouchilib/units/units.hpp"
#include "ouchilib/units/si.hpp"
#include <type_traits>

static_assert(
    std::is_same_v<std::integer_sequence<int, 2, 4>, ouchi::units::detail::add_sequence<std::integer_sequence<int, 1, 3>, std::integer_sequence<int, 1, 1>>::result>
);


using namespace ouchi::units;

static_assert(
    std::is_same_v<
    typename si::length::template mul_t<1,0,0,0,0,0,0>,
    basic_system_of_units<si::detail::si_tag, 2,0,0,0,0,0,0>
    >
);

DEFINE_TEST(quantity_instantiate)
{
    quantity<double, si::length> l = 2.0 * si::m;
    CHECK_EQUAL(l.get_value(), 2.0);
}

DEFINE_TEST(quantity_mul_test)
{
    quantity<double, si::length> l{ 1 }, m{ 2 };
    auto m2 = l * m;
    static_assert(
        std::is_same_v<
        typename decltype(m2)::units,
        basic_system_of_units<si::detail::si_tag, 2,0,0,0,0,0,0>
        >
    );
    CHECK_EQUAL(m2.get_value(), 2);

    auto bai = 2 * m;
    static_assert(
        std::is_same_v<
        typename decltype(bai)::units,
        basic_system_of_units<si::detail::si_tag, 1,0,0,0,0,0,0>
        >
    );
    CHECK_EQUAL(bai.get_value(), 4);

}

DEFINE_TEST(quantity_div_test)
{
    quantity<double, si::length> l{ 1 }, m{ 2 };
    {
        auto dl = l / m;
        static_assert(
            std::is_same_v<
            typename decltype(dl)::units,
            basic_system_of_units<si::detail::si_tag, 0, 0, 0, 0, 0, 0, 0>
            >
            );
        CHECK_EQUAL(dl.get_value(), 0.5);
    }
    {
        auto inv = 2 / m;
        static_assert(
            std::is_same_v<
            typename decltype(inv)::units,
            basic_system_of_units<si::detail::si_tag, -1, 0, 0, 0, 0, 0, 0>
            >
            );
        CHECK_EQUAL(inv.get_value(), 1);
    }
    {
        auto bai = m / 2;
        static_assert(
            std::is_same_v<
            typename decltype(bai)::units,
            basic_system_of_units<si::detail::si_tag, 1, 0, 0, 0, 0, 0, 0>
            >
            );
        CHECK_EQUAL(bai.get_value(), 1);
    }
}

