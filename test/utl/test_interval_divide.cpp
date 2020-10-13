#include "../test.hpp"
#include "ouchilib/utl/interval_divide.hpp"
#include <vector>
#include <numeric>

using namespace ouchi;

DEFINE_TEST(test_interval_divide_instantiate)
{
    interval_divide<int> id{interval_divide<int>{0, 3}};
    interval_divide<int>{id};

    interval_divide<int>{std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(4)};
}

DEFINE_TEST(test_interval_divide_integral)
{
    // [1, 9)
    interval_divide<int> id{ 1, 9 };
    int sum = 0;
    int count;
    for (size_t i = 0; i < 3; ++i) {
        count = 0;
        for (auto s : id.divide_into_steps(i, 3)) {
            sum += s;
            ++count;
        }
        CHECK_TRUE(count <= 3);
    }
    CHECK_EQUAL(sum, 36);
}

DEFINE_TEST(test_interval_divide_container)
{
    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1);
    interval_divide id{v.begin(), v.end()};
    int sum = 0;
    int count;
    for (size_t i = 0; i < 3; ++i) {
        count = 0;
        for (auto s : id.divide_into_steps(i, 3)) {
            sum += *s;
            ++count;
        }
        CHECK_TRUE(count <= 4);
    }
    CHECK_EQUAL(sum, 55);
}

