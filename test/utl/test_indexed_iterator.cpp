#include "../test.hpp"
#include "ouchilib/utl/indexed_iterator.hpp"
#include <vector>

DEFINE_TEST(test_indexed_iterator)
{
    std::vector<int> vec{ 0, 1, 2, 3 };

    for (auto [i, v] : ouchi::indexed_iterator(vec)) {
        CHECK_EQUAL(i, v);
    }
    for (auto [i, v] : ouchi::indexed_iterator(vec.begin(), vec.end(), 1)) {
        CHECK_EQUAL(i, v+1);
    }
}
