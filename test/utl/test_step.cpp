#include "../test.hpp"
#include "ouchilib/utl/step.hpp"

constexpr int constexpr_step_test(int i)
{
    int j = 0;
    for (auto k : ouchi::step{ i }) j += k;
    return j;
}

static_assert(constexpr_step_test(10) == 45);

