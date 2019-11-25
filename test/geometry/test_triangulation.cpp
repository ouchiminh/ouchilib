#include "../test.hpp"
#include "ouchilib/geometry/triangulation.hpp"
#include "ouchilib/math/matrix.hpp"

#include <chrono>

#include <random>

#if 1
DEFINE_TEST(test_t_gcc)
{
    {
        using namespace ouchi::geometry;
        using namespace ouchi::math;
        using pt = point_traits<fl_matrix<double, 2, 1>>;
        triangulation<fl_matrix<double, 2, 1>> t;
        std::array<fl_matrix<double, 2, 1>, 3> pts = {
            fl_matrix<double, 2, 1>{ -2, 0 },
            fl_matrix<double, 2, 1>{ 2, 0 },
            fl_matrix<double, 2, 1>{ 0, 2 }
        };
        auto o = t.get_circumscribed_circle(pts);
        CHECK_EQUAL(o.first(0), 0);
        CHECK_EQUAL(o.first(1), 0);
        CHECK_EQUAL(o.second, 4);
    }
    {
        using namespace ouchi::geometry;
        using namespace ouchi::math;
        using pt = point_traits<fl_matrix<double, 2, 1>>;
        triangulation<fl_matrix<double, 2, 1>> t;
        std::array<fl_matrix<double, 2, 1>, 3> pts = {
            fl_matrix<double, 2, 1>{ 0, 1 },
            fl_matrix<double, 2, 1>{ 2, 1 },
            fl_matrix<double, 2, 1>{ 1, 2 }
        };
        auto o = t.get_circumscribed_circle(pts);
        CHECK_EQUAL(o.first(0), 1);
        CHECK_EQUAL(o.first(1), 1);
        CHECK_EQUAL(o.second, 1);
    }
    {
        using namespace ouchi::geometry;
        using namespace ouchi::math;
        using pt = point_traits<fl_matrix<double, 3, 1>>;
        triangulation<fl_matrix<double, 3, 1>> t;
        std::array<fl_matrix<double, 3, 1>, 4> pts = {
            fl_matrix<double, 3, 1>{ 0, 0, 1 },
            fl_matrix<double, 3, 1>{ 0, 1, 0 },
            fl_matrix<double, 3, 1>{ 1, 0, 0 },
            fl_matrix<double, 3, 1>{ -1,0, 0 }
        };
        auto o = t.get_circumscribed_circle(pts);
        CHECK_EQUAL(o.first(0), 0);
        CHECK_EQUAL(o.first(1), 0);
        CHECK_EQUAL(o.first(2), 0);
        CHECK_EQUAL(o.second, 1);
    }
    {
        using namespace ouchi::geometry;
        using namespace ouchi::math;
        using pt = point_traits<fl_matrix<double, 3, 1>>;
        triangulation<fl_matrix<double, 3, 1>> t;
        std::array<fl_matrix<double, 3, 1>, 4> pts = {
            fl_matrix<double, 3, 1>{ 1, 1, 2 },
            fl_matrix<double, 3, 1>{ 1, 2, 1 },
            fl_matrix<double, 3, 1>{ 2, 1, 1 },
            fl_matrix<double, 3, 1>{ 0, 1, 1 }
        };
        auto o = t.get_circumscribed_circle(pts);
        CHECK_EQUAL(o.first(0), 1);
        CHECK_EQUAL(o.first(1), 1);
        CHECK_EQUAL(o.first(2), 1);
        CHECK_EQUAL(o.second, 1);
    }
    {
        using namespace ouchi::geometry;
        using namespace ouchi::math;
        using pt = point_traits<fl_matrix<double, 3, 1>>;
        triangulation<fl_matrix<double, 3, 1>> t;
        std::array<fl_matrix<double, 3, 1>, 3> pts = {
            fl_matrix<double, 3, 1>{ 1, 0, 1 },
            fl_matrix<double, 3, 1>{ 2, 1, 1 },
            fl_matrix<double, 3, 1>{ 0, 1, 1 },
        };
        auto o = t.get_circumscribed_circle(pts);
        CHECK_EQUAL(o.first(0), 1);
        CHECK_EQUAL(o.first(1), 1);
        CHECK_EQUAL(o.first(2), 1);
        CHECK_EQUAL(o.second, 1);
    }
}

DEFINE_TEST(test_tri_volume)
{
    using namespace ouchi::geometry;
    using namespace ouchi::math;
    using pt = point_traits<fl_matrix<double, 2, 1>>;
    {
        triangulation<fl_matrix<double, 2, 1>> t;
        // 面積2の三角形
        std::array<fl_matrix<double, 2, 1>, 3> pts = {
            fl_matrix<double, 2, 1>{ 0, 0 },
            fl_matrix<double, 2, 1>{ 2, 0 },
            fl_matrix<double, 2, 1>{ 0, 2 }
        };
        auto s = t.volume(pts);
        CHECK_EQUAL(s, 2);
    }
    {
        triangulation<fl_matrix<double, 2, 1>> t;
        // 面積2の三角形
        std::array<fl_matrix<double, 2, 1>, 3> pts = {
            fl_matrix<double, 2, 1>{ 1, 1 },
            fl_matrix<double, 2, 1>{ 3, 1 },
            fl_matrix<double, 2, 1>{ 1, 3 }
        };
        auto s = t.volume(pts);
        CHECK_EQUAL(s, 2);
    }
}
#endif

DEFINE_TEST(test_tri)
{
    using namespace ouchi::geometry;
    using namespace ouchi::math;
    using pt = point_traits<fl_matrix<double, 2, 1>>;
    {
        triangulation<fl_matrix<double, 2, 1>> t;
        // 面積2の三角形
        std::array<fl_matrix<double, 2, 1>, 4> pts = {
            fl_matrix<double, 2, 1>{ 0, 0 },
            fl_matrix<double, 2, 1>{ 3, 0 },
            fl_matrix<double, 2, 1>{ 1, 1 },
            fl_matrix<double, 2, 1>{ 0, 3 }
        };
        auto r = t(pts.begin(), pts.end(), t.return_as_idx);
        CHECK_EQUAL(r.size(), 3);
    }
    {
        triangulation<fl_matrix<double, 2, 1>> t;
        // 面積2の三角形
        std::array<fl_matrix<double, 2, 1>, 4> pts = {
            fl_matrix<double, 2, 1>{ 0, 0 },
            fl_matrix<double, 2, 1>{ 3, 0 },
            fl_matrix<double, 2, 1>{ 3, 3 },
            fl_matrix<double, 2, 1>{ 0, 3 }
        };
        auto r = t(pts.begin(), pts.end(), t.return_as_idx);
        CHECK_EQUAL(r.size(), 2);
    }
    //{
    //    triangulation<fl_matrix<double, 2, 1>> t;
    //    // 面積2の三角形
    //    std::vector<fl_matrix<double, 2, 1>> pts;
    //    std::mt19937 mt;
    //    std::normal_distribution<> di(0, 1.0);
    //    for (auto i = 0; i < 10000; ++i) {
    //        pts.push_back(
    //            {
    //               di(mt) * 10,
    //               di(mt) * 10
    //            });
    //    }
    //    auto beg = std::chrono::high_resolution_clock::now();
    //    auto r = t(pts.begin(), pts.end(), t.return_as_idx);
    //    auto d = std::chrono::high_resolution_clock::now() - beg;
    //    std::cout << d.count() / (double)std::chrono::high_resolution_clock::period::den << std::endl;
    //}
}


