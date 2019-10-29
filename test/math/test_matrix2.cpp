#include "../test.hpp"
#include "ouchilib/math/matrix2.hpp"

DEFINE_TEST(test_computability)
{
    using namespace ouchi::math::matrix_size_specifier;
    static_assert(detail::add_possibility_v<variable_length, variable_length> == detail::computability::maybe);
    static_assert(detail::add_possibility_v<fixed_length<1, 2>, variable_length> == detail::computability::maybe);
    static_assert(detail::add_possibility_v<fixed_length<1, 2>, fixed_length<1, 2>> == detail::computability::possible);
    static_assert(detail::add_possibility_v<fixed_length<2, 2>, fixed_length<1, 2>> == detail::computability::impossible);

    static_assert(detail::mul_possibility_v<variable_length, variable_length> == detail::computability::maybe);
    static_assert(detail::mul_possibility_v<fixed_length<2, 2>, variable_length> == detail::computability::maybe);
    static_assert(detail::mul_possibility_v<fixed_length<2, 2>, fixed_length<2, 2>> == detail::computability::possible);
    static_assert(detail::mul_possibility_v<fixed_length<5, 2>, fixed_length<2, 4>> == detail::computability::possible);
    static_assert(detail::mul_possibility_v<fixed_length<3, 3>, fixed_length<2, 4>> == detail::computability::impossible);
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
    // fm1 + err;
}

template<class T, std::enable_if_t<std::is_arithmetic_v<T>, std::nullptr_t> = nullptr>
struct A {
    template<class U>
    friend auto operator+(A, A<U>) { return A{}; }
};

DEFINE_TEST(test_msvc)
{
    A<int>{}+A<double>{};
}
