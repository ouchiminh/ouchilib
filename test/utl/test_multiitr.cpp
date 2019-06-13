#include "../test.hpp"
#include "ouchilib/utl/multiitr.hpp"
#include <vector>

DEFINE_TEST(test_multiitr_instantiate)
{
    std::vector<int> v1{ 1, 2, 3 };
    int v2[] = { 3, 2, 1 };
    size_t c = 0;

    for (auto [i1, i2] : ouchi::multiitr{ v1, v2 }) {
        CHECK_EQUAL(i1 + i2, 4);
        ++c;
    }
    CHECK_EQUAL(c, 3);
    int v3[] = { 1 };
    try {
        for (auto [i1, i2] : ouchi::multiitr{ std::as_const(v1), std::as_const(v3) });
        CHECK_TRUE(false);
    } catch (std::runtime_error&) {}
    for (auto [i1, i2] : ouchi::multiitr{ v1, v2 });
}
