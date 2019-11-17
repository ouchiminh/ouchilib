#include "../test.hpp"
#include "ouchilib/geometry/triangulation.hpp"
#include "ouchilib/math/matrix.hpp"

DEFINE_TEST(test_tri_init)
{
    using namespace ouchi::geometry;
    using namespace ouchi::math;
    using pt = point_traits<fl_matrix<double, 2, 1>>;
    triangulation<fl_matrix<double, 2, 1>> t;
    fl_matrix<double, 2, 1> pts[] = {
        { 0, 0 }, { 1, 0 }, { 0, 1 }
    };
    auto simplex = t.calc_space(pts, pts+3);

    for (auto i = 0ul; i < 3; ++i) {
        auto j = (i + 1) % 3;
        CHECK_TRUE(std::abs(pt::sqdistance(simplex[i], simplex[j]) - 8) < 1e-5);
    }
}
