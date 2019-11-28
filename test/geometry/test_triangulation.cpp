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
            fl_matrix<double, 2, 1>{ 4, 4 },
            fl_matrix<double, 2, 1>{ 0, 3 }
        };
        auto r = t(pts.begin(), pts.end(), t.return_as_idx);
        CHECK_EQUAL(r.size(), 2);
    }
    //for(auto cnt = 0ul; cnt <= 20000ul; cnt += 1000) {
    //    triangulation<fl_matrix<double, 2, 1>, 0> t;
    //    // 面積2の三角形
    //    std::vector<fl_matrix<double, 2, 1>> pts;
    //    std::mt19937 mt;
    //    std::normal_distribution<> di(0, 1.0);
    //    for (auto i = 0ul; i < cnt; ++i) {
    //        pts.push_back(
    //            {
    //               di(mt) * 10,
    //               di(mt) * 10
    //            });
    //    }
    //    auto beg = std::chrono::high_resolution_clock::now();
    //    auto r = t(pts.begin(), pts.end(), t.return_as_idx);
    //    auto d = std::chrono::high_resolution_clock::now() - beg;
    //    std::cout << cnt << ' ' << d.count() / (double)std::chrono::high_resolution_clock::period::den << std::endl;
    //}
    //{
    //    constexpr auto cnt = 50000;
    //    triangulation<fl_matrix<double, 2, 1>, 0> t;
    //    std::vector<fl_matrix<double, 2, 1>> pts;
    //    std::mt19937 mt;
    //    std::normal_distribution<> di(0, 1.0);
    //    for (auto i = 0ul; i < cnt; ++i) {
    //        pts.push_back(
    //            {
    //               di(mt) * 10,
    //               di(mt) * 10
    //            });
    //    }
    //    auto beg = std::chrono::high_resolution_clock::now();
    //    auto r = t(pts.begin(), pts.end(), t.return_as_idx);
    //    auto d = std::chrono::high_resolution_clock::now() - beg;
    //    std::cout << cnt << ' ' << d.count() / (double)std::chrono::high_resolution_clock::period::den << std::endl;
    //}
    //{
    //    constexpr auto cnt = 500000;
    //    triangulation<fl_matrix<double, 2, 1>, 1000> t;
    //    std::vector<fl_matrix<double, 2, 1>> pts;
    //    std::mt19937 mt;
    //    std::normal_distribution<> di(0, 1.0);
    //    for (auto i = 0ul; i < cnt; ++i) {
    //        pts.push_back(
    //            {
    //               di(mt) * 10,
    //               di(mt) * 10
    //            });
    //    }
    //    auto beg = std::chrono::high_resolution_clock::now();
    //    auto r = t(pts.begin(), pts.end(), t.return_as_idx);
    //    auto d = std::chrono::high_resolution_clock::now() - beg;
    //    std::cout << cnt << ' ' << d.count() / (double)std::chrono::high_resolution_clock::period::den << std::endl;
    //}
}
#include <fstream>
#include <iostream>
DEFINE_TEST(tri_plot)
{
    using namespace ouchi::geometry;
    using namespace ouchi::math;
    using pt = point_traits<fl_matrix<double, 2, 1>>;
    constexpr auto cnt = 50;
    triangulation<fl_matrix<double, 2, 1>, 0> t;
    std::vector<fl_matrix<double, 2, 1>> pts;
    std::mt19937 mt;
    std::uniform_real_distribution<> di(0, 1.0);
    for (auto i = 0ul; i < cnt; ++i) {
        pts.push_back(
            {
               di(mt) * 10,
               di(mt) * 10
            });
    }
    auto r = t(pts.begin(), pts.end(), t.return_as_idx);
    //std::ofstream ofs_p("p.txt");
    //for (auto& p : pts) {
    //    ofs_p << p(0) << ' ' << p(1) << '\n';
    //}

    std::ofstream ofs("tri.dat");
    ofs <<
R"(
set terminal svg size 400,300 enhanced fname 'arial'  fsize 10 butt solid
set output 'out.svg'

# Key means label...
set key outside bottom right
set xrange[-0:10]
set yrange[-0:10]

)";
    ofs << R"(plot "-" w l lw 0.5)" "\n\n";
    for (auto& s : r) {
        std::array<fl_matrix<double, 2, 1>, 2 + 1> buf;
        for (auto i = 0ul; i < s.size(); ++i) {
            for (auto j = 0ul; j < s.size(); ++j) {
                if (i == j) {
                    continue;
                }
                auto p = pts[s[j]];
                ofs << p(0) << ' ' << p(1) << "\n";
            }
            ofs << "\n";
        }
    }
    ofs << "e\n";
}
