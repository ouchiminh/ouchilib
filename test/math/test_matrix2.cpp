#include "../test.hpp"
#include "ouchilib/math/matrix2.hpp"

DEFINE_TEST(test_atrix2_instantiation)
{
    /*constexpr*/ ouchi::math::vl_matrix<int> vm;
    constexpr ouchi::math::fl_matrix<int, 10, 10> fm;
    auto [fr, fc] = fm.size();
    auto [vr, vc] = vm.size();
    CHECK_EQUAL(fc, 10);
    CHECK_EQUAL(fr, 10);
    CHECK_EQUAL(vr, 0);
    CHECK_EQUAL(vc, 0);
}
