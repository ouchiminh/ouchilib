#include "../test.hpp"
#include "ouchilib/math/matrix2.hpp"

DEFINE_TEST(test_matrix2_instantiation)
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

DEFINE_TEST(test_matrix2_initialization)
{
    using namespace ouchi::math;
    vl_matrix<int> vm({ 1,2,3,4,5,6 }, 2, 3);
    fl_matrix<int, 2, 3> fl{
        1,2,3,
        4,5,6
    };
}
