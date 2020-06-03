#include "../test.hpp"
#include "ouchilib/geometry/metric.hpp"

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

