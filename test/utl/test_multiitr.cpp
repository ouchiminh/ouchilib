#include "../test.hpp"
#include "ouchilib/utl/multiitr.hpp"
#include <vector>
#include <forward_list>

DEFINE_TEST(test_multiitr_instantiate)
{
    std::vector<int> v1{ 1, 2, 3 }, vv{ 1, 2, 3 };
    int v2[] = { 3, 2, 1 };
    size_t c = 0;

    for (auto [i1, i2] : ouchi::multiitr{ v1, v2 }) {
        CHECK_EQUAL(i1 + i2, 4);
        ++c;
    }
    CHECK_EQUAL(c, 3);
    int v3[] = { 1 };
    try {
        for (auto [i1, i2] : ouchi::multiitr{ std::as_const(v1), std::as_const(v3) });
        CHECK_TRUE(false);
    } catch (std::runtime_error&) {}
    for (auto [i1, i2] : ouchi::multiitr{ v1, v2 });
    auto m = ouchi::multiitr{ v1, vv };
    auto b = ++m.begin();
    --b;
}

namespace ouchi {

template<class T>
class oforward_list : public std::forward_list<T> {
public:
    auto size() const
    {
        return std::distance(this->begin(), this->end());
    }
    oforward_list(std::initializer_list<T> il)
        : std::forward_list<T>(std::move(il))
    {}
};

}

DEFINE_TEST(test_multiitr_ce)
{
    ouchi::oforward_list<int> l1{ 1, 2, 3 }, l2{ 1,2,3 };

    for (auto [i1, i2] : ouchi::multiitr{ l1, l2 });

    ouchi::multiitr m{ l1, l2 };
    auto b = m.begin();
    --b;
}
