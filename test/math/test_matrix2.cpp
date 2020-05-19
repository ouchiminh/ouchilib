#include "../test.hpp"
#include "ouchilib/math/matrix.hpp"

DEFINE_TEST(test_mat2_static)
{
    using namespace ouchi::math::matrix_size_specifier;

    static_assert(detail::is_n_by_n_or_larger_v<fixed_length<1, 1>> == detail::condvalue::yes);
    static_assert(detail::is_n_by_n_or_larger_v<fixed_length<0, 0>> == detail::condvalue::no);
    static_assert(detail::is_n_by_n_or_larger_v<fixed_length<2, 2>> == detail::condvalue::yes);
    static_assert(detail::is_n_by_n_or_larger_v<variable_length> == detail::condvalue::maybe);

    static_assert(detail::add_possibility_v<variable_length, variable_length> == detail::condvalue::maybe);
    static_assert(detail::add_possibility_v<fixed_length<1, 2>, variable_length> == detail::condvalue::maybe);
    static_assert(detail::add_possibility_v<fixed_length<1, 2>, fixed_length<1, 2>> == detail::condvalue::yes);
    static_assert(detail::add_possibility_v<fixed_length<2, 2>, fixed_length<1, 2>> == detail::condvalue::no);

    static_assert(std::is_same_v<detail::add_possibility_t<fixed_length<2, 2>, fixed_length<2, 2>>, fixed_length<2, 2>>);

    static_assert(detail::mul_possibility_v<variable_length, variable_length> == detail::condvalue::maybe);
    static_assert(detail::mul_possibility_v<fixed_length<2, 2>, variable_length> == detail::condvalue::maybe);
    static_assert(detail::mul_possibility_v<fixed_length<2, 2>, fixed_length<2, 2>> == detail::condvalue::yes);
    static_assert(detail::mul_possibility_v<fixed_length<5, 2>, fixed_length<2, 4>> == detail::condvalue::yes);
    static_assert(detail::mul_possibility_v<fixed_length<4, 4>, fixed_length<4, 1>> == detail::condvalue::yes);
    static_assert(detail::mul_possibility_v<fixed_length<3, 3>, fixed_length<2, 4>> == detail::condvalue::no);

    static_assert(std::is_same_v<detail::mul_possibility_t<fixed_length<4, 4>, fixed_length<4, 1>>, fixed_length<4, 1>>);
    
    static_assert(is_fixed_length_v<fixed_length<1, 1>>);
    static_assert(!is_fixed_length_v<variable_length>);
}

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

DEFINE_TEST(test_matrix2_addition)
{
    using namespace ouchi::math;
    vl_matrix<int> vm1({ 1,1,1,1 }, 2, 2);
    vl_matrix<int> vm2({ 1,2,3,4 }, 2, 2);
    constexpr fl_matrix<int, 2, 2> fm1{ 1,1,1,1 };
    constexpr fl_matrix<int, 2, 2> fm2{ 1,2,3,4 };

    auto r1 = vm1 + vm2;
    for (auto i = 0u; i < r1.total_size(); ++i) {
        CHECK_EQUAL(r1(i), (int)i+2);
    }
    auto r2 = vm1 + fm2;
    for (auto i = 0u; i < r2.total_size(); ++i) {
        CHECK_EQUAL(r2(i), (int)i+2);
    }
    constexpr auto r3 = fm1 + fm2;
    for (auto i = 0u; i < r3.total_size(); ++i) {
        CHECK_EQUAL(r3(i), (int)i+2);
    }
    fl_matrix<int, 1, 2> err;
    CHECK_THROW(vm1 + err);
    CHECK_THROW(err + vm1);
    // fm1 + err;
}

DEFINE_TEST(test_matrix2_mul)
{
    using namespace ouchi::math;
    constexpr fl_matrix<int, 2, 2> fm1{ 2,0,0,2 };
    constexpr fl_matrix<int, 2, 2> fm2{ 1,2,3,4 };
    vl_matrix<int> vm1({ 2,0,0,2 }, 2, 2);
    vl_matrix<int> vm2({ 1,2,3,4 }, 2, 2);

    constexpr auto r1 = fm2 * fm1;
    auto r2 = fm2 * vm1;
    auto r3 = vm2 * fm1;
    auto r4 = vm2 * vm1;

    for (auto i = 0u; i < 4; ++i) {
        CHECK_EQUAL(r1(i), (int)(i + 1) * 2);
        CHECK_EQUAL(r2(i), (int)(i + 1) * 2);
        CHECK_EQUAL(r3(i), (int)(i + 1) * 2);
        CHECK_EQUAL(r4(i), (int)(i + 1) * 2);
    }

    constexpr fl_matrix<int, 1, 2> err{};
    CHECK_THROW(vm1 * err);
    (void)(err * fm1);
    //fm1 * err;
    constexpr fl_matrix<int, 4, 4> fm4{};
    constexpr fl_matrix<int, 4, 1> fm5{};
    (void)(fm4 * fm5);
}

DEFINE_TEST(test_matrix_sub)
{
    using namespace ouchi::math;
    constexpr fl_matrix<int, 2, 2> fm1{ 2,0,0,2 };
    constexpr fl_matrix<int, 2, 2> fm2{ 3, 1, 1, 3 };
    vl_matrix<int> vm2({ 3,1,1,3 }, 2, 2);
    constexpr auto r1 = fm2 - fm1;
    auto r2 = vm2 - fm1;
    for (auto i = 0u; i < r1.total_size(); ++i) {
        CHECK_EQUAL(r1(i), 1);
        CHECK_EQUAL(r2(i), 1);
    }

}

DEFINE_TEST(test_matrix_resize)
{
    using namespace ouchi::math;
    vl_matrix<int> vm;
    vm.resize(2, 2);
    for (auto i = 0u; i < 4; ++i)
        CHECK_EQUAL(vm(i), 0);
}

DEFINE_TEST(test_det)
{
    using namespace ouchi::math;
    fl_matrix<double, 2, 2> m1{ 0, 0, 0, 0 };
    CHECK_EQUAL(det(m1), 0);
    vl_matrix<double> m2(
        {
            3,1,1,2,
            5,1,3,4,
            2,0,1,0,
            1,3,2,1
        }, 4, 4
    );
    auto d = det(m2);
    auto d2 = fast_det(m2);
    CHECK_EQUAL(d, -22);
    CHECK_TRUE(std::abs(d - d2) < 1e-8);
}

DEFINE_TEST(test_minor)
{
    using namespace ouchi::math;
    constexpr fl_matrix<double, 2, 2> m1{
        1, 2,
        3, 4
    };
    constexpr auto co = m1.minor(1, 1);
    CHECK_EQUAL(co(0, 0), 1);
    CHECK_EQUAL(co.total_size(), 1);

    vl_matrix<double> m2(
        {
            1, 2,
            3, 4
        }, 2, 2
    );
    auto co2 = m2.minor(1, 0);

    CHECK_EQUAL(co2(0, 0), 2);
    CHECK_EQUAL(co2.total_size(), 1);
}

DEFINE_TEST(test_cofactor_matrix)
{
    using namespace ouchi::math;
    constexpr fl_matrix<double, 3, 3> m1{
        7,2,3,
        4,1,9,
        1,3,5
    };
    constexpr fl_matrix<double, 3, 3> mc{
        -22, -1, 15,
        -11, 32,-51,
        11, -19, -1
    };
    auto c = m1.cofactor_matrix();

    CHECK_EQUAL(c, mc);
}

DEFINE_TEST(test_scalar_production)
{
    using namespace ouchi::math;
    constexpr fl_matrix<double, 2, 2> m1{
        0, 1,
        2, 3
    };
    vl_matrix<double> m2(
        {
            0, 1,
            2, 3
        }, 2, 2
    );
    auto r1 = m1 * 2;
    auto r2 = m2 * 2;
    auto r3 = 2 * m1;
    for (auto i = 0ul; i < m1.total_size(); ++i) {
        CHECK_EQUAL(r1(i), i * 2);
        CHECK_EQUAL(r2(i), i * 2);
        CHECK_EQUAL(r3(i), i * 2);
    }
}

DEFINE_TEST(test_transpose)
{
    using namespace ouchi::math;
    constexpr fl_matrix<int, 2, 1> m1{
        1,
        2
    };
    vl_matrix<int> m2{
        {
            1,
            2
        }, 2, 1
    };
    auto t1 = m1.transpose();
    auto t2 = m2.transpose();
    for (auto i = 0ul; i < 1; ++i) {
        for (auto j = 0ul; j < 2; ++j) {
            CHECK_EQUAL(t1(i, j), m1(j, i));
            CHECK_EQUAL(t2(i, j), m2(j, i));
        }
    }
}

DEFINE_TEST(test_cofactor)
{
    using namespace ouchi::math;
    fl_matrix<int, 4, 4> m{
        3,4,1,2,
        7,5,6,7,
        8,6,7,5,
        1,3,2,2
    };
    CHECK_EQUAL(m.cofactor(1, 2), 17);
    vl_matrix<int> mv{
        {3,4,1,2,
         7,5,6,7,
         8,6,7,5,
         1,3,2,2},
        4, 4
    };
}

DEFINE_TEST(test_matrix_lu)
{
    using namespace ouchi::math;
    fl_matrix<double, 2, 2> m{
        2, 16,
        4, 8
    };
    vl_matrix<double> mv{
        {
        4, 8,
        2, 16
        }, 2, 2
    };
    auto lu = m.lu();
    auto luv = m.lu();
    fl_matrix<double, 2, 2>ans{
        4.0, 8.0,
        0.5, 12.0
    };
    fl_matrix<double, 2, 2>P{
        0,1,
        1,0
    };
    CHECK_EQUAL(ans, lu.second);
    CHECK_EQUAL(ans, luv.second);
    CHECK_EQUAL(luv.first, P);
}

DEFINE_TEST(test_matrix_inv)
{
    using namespace ouchi::math;
    fl_matrix<double, 2, 2> m{
        2, 0,
        4, 2
    };
    vl_matrix<double> mv{
        {
        2, 0,
        4, 2
        }, 2, 2
    };
    fl_matrix<double, 2, 2> ans{
        .5, 0,
        -1,.5
    };
    auto inv = m.inv().unwrap();
    auto vinv = mv.inv().unwrap();
    CHECK_EQUAL(inv, ans);
    CHECK_EQUAL(vinv, ans);
    fl_matrix<double, 2, 2> erm{
        0, 0,
        0, 0
    };
    auto er = erm.inv();
    CHECK_TRUE(!er);
}

