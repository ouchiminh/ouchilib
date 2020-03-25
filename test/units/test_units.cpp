#include "../test.hpp"
#include "ouchilib/units/units.hpp"
#include "ouchilib/units/si.hpp"
#include <type_traits>

static_assert(
    std::is_same_v<std::integer_sequence<int, 2>, ouchi::units::detail::add_sequence<std::integer_sequence<int, 1>, std::integer_sequence<int, 1>>::result>
);

using namespace ouchi::units;

DEFINE_TEST(quantity_instantiate)
{
    quantity<double, si::length> l;
}

