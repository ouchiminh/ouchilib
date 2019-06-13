#include "../test.hpp"
#include "ouchilib/utl/indexed_iterator.hpp"
#include <vector>

DEFINE_TEST(test_indexed_iterator)
{
    std::vector<int> v{ 0, 1, 2, 3 };

    for (auto [i, v] : ouchi::indexed_iterator(v)) {
        CHECK_EQUAL(i, v);
    }
    for (auto [i, v] : ouchi::indexed_iterator(v.begin(), v.end(), 1)) {
        CHECK_EQUAL(i, v+1);
    }
}
