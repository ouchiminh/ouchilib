#include "../test.hpp"
#include "ouchilib/geometry/metric.hpp"
#include "ouchilib/math/matrix.hpp"
#include "ouchilib/geometry/point_traits.hpp"

DEFINE_TEST(test_metric_basic)
{
    using namespace ouchi::geometry;
    metric<int> m;
    metric<int> m2(0, 2);
    m.assign(0, 1);
    CHECK_TRUE(m > m2);
    CHECK_TRUE(m2 < m);
    CHECK_TRUE(m2 != m);
}

DEFINE_TEST(test_metric_operations)
{
    using namespace ouchi::geometry;
    metric<int> m;
    metric<int> m2(0, 2);
    m.assign(0, 1);
    auto d = m - m2;
    CHECK_TRUE(d > m2);

    auto p = m * m2;
    CHECK_TRUE(m > p);
    CHECK_TRUE(m2 > p);
}

DEFINE_TEST(test_singned_area_metric)
{
    using namespace ouchi::geometry;
    ouchi::math::fl_matrix<metric<int>, 3,3> sa = {
        metric<int>(0, 1ull), metric<int>(0, 20ull), 1,
        metric<int>(1, 2ull), metric<int>(1, 21ull), 1,
        metric<int>(2, 3ull), metric<int>(2, 22ull), 1
    };
    auto signedarea = det(sa);
    
    assert(signedarea<0);
}

