#include <iostream>
#include <vector>
#include "../test.hpp"
#include "ouchilib/utl/constexpr_for.h"

DEFINE_TEST(test_constexpr_for)
{
    std::vector<size_t> ans{ 0,1,2,3,4 }, res;
    ouchi::constexpr_for<5>([&res](size_t i) {res.push_back(i); });
    CHECK_EQUAL(ans, res);
    res.clear();
    ouchi::constexpr_for<5>([&res]() {res.push_back(res.size()); });
    CHECK_EQUAL(ans, res);
    res.clear();
}

